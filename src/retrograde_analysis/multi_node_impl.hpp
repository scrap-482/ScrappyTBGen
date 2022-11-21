#ifndef MULTI_NODE_IMPL_HPP_

#include <tuple>
#include <bitset>
#include <cmath>
#include <cassert>
#include <iostream>
#include <cstddef>
#include <algorithm>
#include <memory>
#include <chrono>

#include <mpi.h>

#include "state.hpp"
#include "checkmate_generation.hpp"

// Global MPI type definitions. Must be initialized in main with initialize_comm_structs
MPI_Datatype MPI_NodeCommData;
MPI_Datatype MPI_NonPlacementDataType;
MPI_Datatype MPI_BoardState;

// Data sent between MPI nodes
template <::std::size_t FlattenedSz, typename NonPlacementDataType> 
struct NodeCommData 
{
  bool winLabel;
  short G;
  BoardState<FlattenedSz, NonPlacementDataType> b; 
};

struct NodeEstimateData
{
  short T;
  short M;
  short C;
};

// Works under the assumption that NonPlacement DataType is already
// serialized in the MPI_NonPlacementDataType global variable
template <::std::size_t FlattenedSz, typename NonPlacementDataType> 
void initialize_comm_structs(void)
{
  { // serialize Board State
    // C++ preprocessor does not understand template syntax so this is necessary
    typedef BoardState<FlattenedSz, NonPlacementDataType> board_state_t;

    int count = 3;
    int blocklengths[] = { 1, FlattenedSz, 1 };

    MPI_Aint displacements[] = 
    { 
      offsetof(board_state_t, m_player), 
      offsetof(board_state_t, m_board),
      offsetof(board_state_t, nonPlacementData)
    };
    
    MPI_Datatype types[] = { MPI_C_BOOL, MPI_CHAR, MPI_NonPlacementDataType };
    
    MPI_Datatype tmp;
    MPI_Aint lowerBound;
    MPI_Aint extent;

    MPI_Type_create_struct(count, blocklengths, displacements, types,
      &tmp);

    MPI_Type_get_extent(tmp, &lowerBound, &extent);
    MPI_Type_create_resized(tmp, lowerBound, extent, &MPI_BoardState);
    MPI_Type_commit(&MPI_BoardState);
  }
  std::cout << "here2" << std::endl;
  { // Serialize NodeCommData 
    // C++ preprocessor does not understand template syntax so this is necessary
    typedef NodeCommData<FlattenedSz, NonPlacementDataType> node_comm_data_t;

    int count = 3;
    int blocklengths[] = { 1, 1, 1 };

    MPI_Aint displacements[] = 
    { 
      offsetof(node_comm_data_t, winLabel),
      offsetof(node_comm_data_t, G), 
      offsetof(node_comm_data_t, b)
    };

    MPI_Datatype types[] = { MPI_C_BOOL, MPI_SHORT, MPI_BoardState };
    
    MPI_Datatype tmp;
    MPI_Aint lowerBound;
    MPI_Aint extent;

    MPI_Type_create_struct(count, blocklengths, displacements, types,
      &tmp);
    MPI_Type_get_extent(tmp, &lowerBound, &extent);
    MPI_Type_create_resized(tmp, lowerBound, extent, &MPI_NodeCommData);
    MPI_Type_commit(&MPI_NodeCommData);
  }
}

// TODO: better scheme as future work.
// contingent on the location of a single piece on the board. each 
// process is assigned all positions dependent on position of one piece
template <::std::size_t FlattenedSz, typename BoardType>
class KStateSpacePartition
{
  piece_label_t m_toTrack;
  int m_segLength;

public:
  KStateSpacePartition(const piece_label_t& toTrack, int K)
    : m_toTrack(toTrack),
      m_segLength(FlattenedSz / K)
  {
    // with this partitioning scheme, cannot have more nodes than max board size.
    assert(FlattenedSz > K);
  }
  
  // Contingent on tracked piece location 
  int operator()(const BoardType& b) const
  {
    int idx = 0;
    
    for (const auto& c : b.m_board)
    {
      if (c == m_toTrack)
        break;

      ++idx;
    }
    return idx / m_segLength;
  }

  auto getRange(int k) const
  {
    return ::std::make_tuple(k * m_segLength, 
        ::std::min((k+1) * m_segLength, static_cast<int>(FlattenedSz)));
  }
  
  inline bool checkInRange(const auto& startBoard, 
    const auto& currentBoard) const
  {
    return (currentBoard[0] - startBoard[0]) < m_segLength; 
  }
};


template<::std::size_t FlattenedSz, typename NonPlacementDataType, typename EvalFn,
  typename IsValidBoardFn=null_type>
auto inline MPI_generateConfigCheckmates(int k, const KStateSpacePartition<FlattenedSz, BoardState<FlattenedSz, NonPlacementDataType>>& partitioner,
    ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>>&& losses,
    const ::std::vector<piece_label_t>& pieceSet,
    EvalFn checkmateEval,
    IsValidBoardFn boardValidityEval = {})
{
  ::std::array<::std::size_t, FlattenedSz> indexPermutations;
  auto [startFirstIdx, endFirstIdx] = partitioner.getRange(k);
  
  indexPermutations[0] = startFirstIdx;
  int j = 0;
  for (::std::size_t i = 1; i < indexPermutations.size(); ++i)
  {
    if (j == startFirstIdx)
      ++j;
    indexPermutations[i] = j;
    ++j;
  }

  auto startBoard = indexPermutations;

  ::std::size_t kPermute = pieceSet.size();
  bool hasNext = false;
   
  // generates [3, ..., kPermute] sets of new checkmate positions.
  for (::std::size_t kPermute = 3; kPermute != pieceSet.size() + 1; ++kPermute)
  {
    do 
    {
      BoardState<FlattenedSz, NonPlacementDataType> currentBoard;
      for (::std::size_t i = 0; i != kPermute; ++i)
        currentBoard.m_board[indexPermutations[i]] = pieceSet[i]; // scatter pieces

      if constexpr (!::std::is_same<null_type, IsValidBoardFn>::value)
        if (!boardValidityEval(currentBoard))
          continue;
      
      // checking if black loses (white wins) 
      if (checkmateEval(currentBoard))
        losses.insert(currentBoard);
      
      currentBoard.m_player = true;
      
      // checking if white loses (black wins)
      if (checkmateEval(currentBoard))
      {
        losses.insert(currentBoard);
      }

      ::std::reverse(indexPermutations.begin() + kPermute, indexPermutations.end());
      hasNext = ::std::next_permutation(indexPermutations.begin(), indexPermutations.end());
    } while (hasNext && partitioner.checkInRange(startBoard, indexPermutations));
  }
  return ::std::move(losses);
}

template <typename WinFrontier, typename LoseFrontier, typename Partitioner, typename EndGameSet,
  typename PredecessorGen, typename BoardMap>
inline auto do_majorIteration(int id, short v, int numProcs, const Partitioner& p, 
    BoardMap&& boardMap, LoseFrontier& loseFrontier, WinFrontier&& winFrontier,
    const EndGameSet& losses, EndGameSet&& wins, PredecessorGen predFn)
{
  // TODO: See if finer grain parallelism can be employed with OpenMP (otherwise just launch more
  // MPI processes)
  bool b_winFound = false;
  ::std::vector<MPI_Request*> sendRequests;
  for (::std::size_t i = 0; i < loseFrontier.size(); ++i)
  {
    if (wins.find(loseFrontier[i].b) == wins.end())
    {
      if (!b_winFound)
        b_winFound = true;

      auto preds = predFn(loseFrontier[i].b);
      wins.insert(loseFrontier[i].b);

      if (boardMap.find(loseFrontier[i].b) == boardMap.end())
      {
        boardMap[loseFrontier[i].b] = { v, {}, {} }; // last two fields are only relevant to potentially lost states.  
      }
      else
      {
        boardMap[loseFrontier[i].b].T = v; 
      }

      for (auto&& pred : preds)
      {
        auto targetId = p(pred);
        if (targetId == id)
          winFrontier.push_back({true, v, std::move(pred)}); // tell the predecessor that the current state wins in v moves.
        else
        {
          // perform MPI send from current to targetId
          MPI_Request* r = new MPI_Request(); // receiver responsible for knowing 
          sendRequests.push_back(r);
          loseFrontier[i].G = v;
          MPI_Isend(&loseFrontier[i], 1, MPI_NodeCommData, targetId, 0, // need to send a 1 if it is the last msg
            MPI_COMM_WORLD, r);
        }
      }
    }
  }
  // currently use null board. can be extended in the future to pack the last message
  for (int i = 0; i < numProcs; ++i)
  {
    if (i != id)
    {
      MPI_Request* r = new MPI_Request();
      sendRequests.push_back(r);
      MPI_Isend(&loseFrontier[0], 1, MPI_NodeCommData, i, 1, // need to send a 1 if it is the last msg
        MPI_COMM_WORLD, r);
    }
  }
  return ::std::make_tuple(b_winFound, sendRequests,
    ::std::move(boardMap), ::std::move(winFrontier), ::std::move(wins));
}

template <typename WinFrontier, typename LoseFrontier, typename Partitioner, typename EndGameSet, 
  typename PredecessorGen, typename SuccessorGen, typename BoardMap>
inline auto do_minorIteration(int id, int v, int numProcs, const Partitioner& p, 
    BoardMap&& boardMap, LoseFrontier&& loseFrontier, WinFrontier& winFrontier,
    EndGameSet&& losses, const EndGameSet& wins, PredecessorGen predFn, SuccessorGen succFn)
{
  // TODO: See if finer grain parallelism can be employed with OpenMP (otherwise just launch more
  // MPI processes)
  bool b_lossFound = false;
  ::std::vector<MPI_Request*> sendRequests;
  int g = v - 1;
  for (::std::size_t i = 0; i < winFrontier.size(); ++i)
  {
    if (losses.find(winFrontier[i].b) == losses.end() 
        && wins.find(winFrontier[i].b) == wins.end())
    {
      if (!b_lossFound)
        b_lossFound = true;
      
      if (boardMap.find(winFrontier[i].b) == boardMap.end())
      {
        auto succs = succFn(winFrontier[i].b);
        boardMap[winFrontier[i].b] = 
          { 0, winFrontier[i].G, static_cast<short>(succs.size() - 1) }; 
      }

      else // decrement remaining moves by 1 and determine max to loss
      {
        --(boardMap[winFrontier[i].b].C);
        boardMap[winFrontier[i].b].M = ::std::max(boardMap[winFrontier[i].b].M, winFrontier[i].G);
      }

      short remainingPaths = boardMap[winFrontier[i].b].C;
      if (remainingPaths == 0)
      {
        auto preds = predFn(winFrontier[i].b);
        losses.insert(winFrontier[i].b);
        boardMap[winFrontier[i].b].T = boardMap[winFrontier[i].b].M;

        for (auto&& pred : preds)
        {
          auto targetId = p(pred);
          if (targetId == id)
            loseFrontier.push_back({false, boardMap[winFrontier[i].b].T, ::std::move(pred)});
          else
          {
            MPI_Request* r = new MPI_Request();
            sendRequests.push_back(r);
            winFrontier[i].G = boardMap[winFrontier[i].b].T;
            MPI_Isend(&winFrontier[i], 1, MPI_NodeCommData, targetId, 0, 
              MPI_COMM_WORLD, r);
          }
        }
      }
    }
  }
  // communicate finished message
  for (int i = 0; i < numProcs; ++i)
  {
    if (i != id)
    {
      MPI_Request* r = new MPI_Request();
      sendRequests.push_back(r);
      MPI_Isend(&winFrontier[0], 1, MPI_NodeCommData, i, 1, // need to send a 1 if it is the last msg
        MPI_COMM_WORLD, r);
    }
  }
  return ::std::make_tuple(b_lossFound, sendRequests, ::std::move(boardMap), 
    ::std::move(loseFrontier), ::std::move(losses));
}

// TODO: Consider more efficient communication scheme with MPI groups
template<typename BoardMap, typename BoardSet, typename Frontier> 
auto do_syncAndFree(int numNodes,
    ::std::vector<MPI_Request*> sendRequests,
    const BoardSet& wins,
    const BoardSet& losses,
    BoardMap&& boardMap, Frontier&& frontier)
{
  // initially, we only know that the current node is complete with computation
  int finishedNodes = 1;

  typename ::std::remove_reference<decltype(frontier)>::type::value_type recvBuf;
  // 1. Process all receives for the current node
  MPI_Status status;
  // TODO: could this result in a deadlock?
  do 
  {
    // doing async send -> sync receive at the moment 
    MPI_Recv(&recvBuf, 1, MPI_NodeCommData, MPI_ANY_SOURCE,
        MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    
    if (wins.find(recvBuf.b) == wins.end() 
        && losses.find(recvBuf.b) == losses.end())
      frontier.push_back(recvBuf);

    finishedNodes += status.MPI_TAG;
  } while(finishedNodes != numNodes);
  
  ::std::vector<MPI_Status> statuses(sendRequests.size());

  // 2. Wait for requests and free memory
  for (::std::size_t i = 0; i < sendRequests.size(); ++i)
  {
    MPI_Wait(sendRequests[i], &status);
    delete sendRequests[i];
  }

  return ::std::make_tuple(::std::move(boardMap), ::std::move(frontier));
}

// This function is the base implementation for the single-node implementation
template<::std::size_t FlattenedSz, typename NonPlacementDataType, ::std::size_t N, 
  ::std::size_t rowSz, ::std::size_t colSz,
  typename MoveGenerator, typename ReverseMoveGenerator, typename HorizontalSymFn=false_fn, 
  typename VerticalSymFn=false_fn, typename IsValidBoardFn=null_type, 
  typename ::std::enable_if<::std::is_base_of<GenerateForwardMoves<FlattenedSz, NonPlacementDataType>, 
    MoveGenerator>::value>::type* = nullptr,
  typename ::std::enable_if<::std::is_base_of<GenerateReverseMoves<FlattenedSz, NonPlacementDataType>, 
    ReverseMoveGenerator>::value>::type* = nullptr>
auto retrogradeAnalysisClusterImpl(const KStateSpacePartition<FlattenedSz, BoardState<FlattenedSz, NonPlacementDataType>>& partitioner, int id, 
    int numProcs, ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, 
      BoardStateHasher<FlattenedSz, NonPlacementDataType>>&& checkmates,
    MoveGenerator generateSuccessors,
    ReverseMoveGenerator generatePredecessors,
    HorizontalSymFn hzSymFn={}, VerticalSymFn vSymFn={}, 
    IsValidBoardFn isValidBoardFn={})
{
  using board_set_t = ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, 
    BoardStateHasher<FlattenedSz, NonPlacementDataType>>;
  using frontier_t = ::std::vector<NodeCommData<FlattenedSz, NonPlacementDataType>>;
  
  // Estimate data during search - more expensive than omp implementation 
  // BoardState b -> (M b, D b) 
  using board_map_t = 
    ::std::unordered_map<BoardState<FlattenedSz, NonPlacementDataType>, 
    NodeEstimateData, // really want to compress data. 
    BoardStateHasher<FlattenedSz, NonPlacementDataType>>;

  board_set_t wins;
  board_set_t losses = ::std::move(checkmates);
  
  frontier_t winFrontier;
  frontier_t loseFrontier;
  
  board_map_t estimateData;
  ::std::vector<MPI_Request*> sendRequests;
  
  // ultimately invoke additional iteration
  for (const auto& l : losses)
  {
    auto preds = generatePredecessors(l);

    for (const auto& pred : preds)
    {
      auto targetId = partitioner(pred);
      if (targetId != id) // different node processes this
      {
        MPI_Request* r = new MPI_Request();
        winFrontier.push_back({false, 0, pred });
        MPI_Isend(&winFrontier[winFrontier.size()-1], 1, MPI_NodeCommData, targetId, 0, 
              MPI_COMM_WORLD, r);
      }
      else
      {
        loseFrontier.push_back({false, 0, pred });
      }
    }
  }
  for (int i = 0; i < numProcs; ++i)
  {
    if (i != id)
    {
      MPI_Request* r = new MPI_Request();
      sendRequests.push_back(r);
      MPI_Isend(&loseFrontier[0], 1, MPI_NodeCommData, i, 1, // need to send a 1 if it is the last msg
        MPI_COMM_WORLD, r);
    }
  }

  bool b_mark{};
  ::std::tie(estimateData, loseFrontier) = do_syncAndFree(numProcs, sendRequests, 
      wins, losses, ::std::move(estimateData), ::std::move(loseFrontier)); 
  std::cout << "passed first do sync and free" << std::endl;

  winFrontier.clear();
  sendRequests.clear();

  for (short v = 1; v > 0; ++v)
  {
    // 1. Invoke major iteration
    ::std::tie(b_mark, sendRequests, estimateData, winFrontier, wins) = do_majorIteration(id, v, numProcs,
        partitioner, ::std::move(estimateData), loseFrontier, ::std::move(winFrontier), losses,
        ::std::move(wins), generatePredecessors);
    
    ::std::tie(estimateData, winFrontier) = do_syncAndFree(numProcs, sendRequests,
        wins, losses, ::std::move(estimateData), ::std::move(winFrontier));
    
    sendRequests.clear();
    loseFrontier.clear(); // must wait until after synchronization

    if (!b_mark)
      break;

    // 2. Invoke minor iteration
    ::std::tie(b_mark, sendRequests, estimateData, loseFrontier, losses) = do_minorIteration(id, v, numProcs,
      partitioner, ::std::move(estimateData), ::std::move(loseFrontier), winFrontier,
      ::std::move(losses), wins, generatePredecessors, generateSuccessors);
    
    auto t0 = std::chrono::high_resolution_clock::now();
    ::std::tie(estimateData, loseFrontier) = do_syncAndFree(numProcs, sendRequests, 
        wins, losses, ::std::move(estimateData), ::std::move(loseFrontier));
    auto t1 = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count();
    std::cout << "sync time: " << elapsed << std::endl;

    sendRequests.clear();
    winFrontier.clear();

    if (!b_mark)
      break; 
    std::cout << "finished v=" << v << " frontier " << winFrontier.size() << " " << loseFrontier.size() << std::endl;
  }
  return ::std::make_tuple(wins, losses);
}

// TODO: finish implementing this wrapper 
template<typename... Args>
auto retrogradeAnalysisClusterInvoker(Args&&... args)
{
  // 1. initialization
  MPI_Init(NULL, NULL);

  // number of active processes
  int globalSz = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &globalSz);
  
  KStateSpacePartition<64, null_type>
  retrogradeAnalysisClusterImpl(::std::forward<Args>(args)...);

  MPI_Finalize();
}

#endif
