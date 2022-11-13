#ifndef MULTI_NODE_IMPL_HPP_

#include <tuple>
#include <bitset>
#include <cmath>
#include <cassert>
#include <iostream>
#include <cstddef>
#include <algorithm>
#include <memory>

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
  // D(k) in algorithm - can remove from mpi send
  // int depthToEnd;
  // M(k) in algorithm - can remove from mpi send
  // int remainingValidMoves;
  int G;

  BoardState<FlattenedSz, NonPlacementDataType> b; 
};

struct NodeEstimateData
{
  short T;
  short M;
  short C;
};

// TODO: serialize NonPlacementDataType
template <::std::size_t FlattenedSz, typename NonPlacementDataType> 
void initialize_comm_structs(void /* TODO: NonPlacementDataSerializer: void -> MPI_Datatype*/)
{
  {
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

  { 
    // C++ preprocessor does not understand template syntax so this is necessary
    typedef NodeCommData<FlattenedSz, NonPlacementDataType> node_comm_data_t;

    int count = 5;
    int blocklengths[] = { 1, 1, 1, 1, 1 };

    MPI_Aint displacements[] = 
    { 
      offsetof(node_comm_data_t, isWin),
      offsetof(node_comm_data_t, depthToEnd), 
      offsetof(node_comm_data_t, remainingValidMoves), 
      offsetof(node_comm_data_t, G), 
      offsetof(node_comm_data_t, b)
    };

    MPI_Datatype types[] = { MPI_C_BOOL, MPI_INT, MPI_INT, MPI_INT, MPI_BoardState };
    
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

#if 1
// TODO: better scheme in future.
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

  auto getRange(int k)
  {
    return ::std::make_tuple(k * m_segLength, ::std::max((k+1) * m_segLength, static_cast<int>(FlattenedSz)));
  }
  
  inline bool checkInRange(const ::std::vector<int>& startBoard, 
      const ::std::vector<int>& currentBoard)
  {
    return (currentBoard[0] - startBoard[0]) < m_segLength; 
  }
};

#else 
template <::std::size_t FlattenedSz>
class KStateSpacePartition
{
  // inclusive range
  ::std::bitset<FlattenedSz> m_lowerRange;
  ::std::bitset<FlattenedSz> m_upperRange;

  ::std::size_t m_id;
  ::std::size_t m_k;
  ::std::size_t m_partitionSzLog;
  
  // https://www.geeksforgeeks.org/arithmetic-operations-with-stdbitset-in-c/
  bool fullAdder(bool b1, bool b2, bool& carry)
  {
      bool sum = (b1 ^ b2) ^ carry;
      carry = (b1 && b2) || (b1 && carry) || (b2 && carry);
      return sum;
  }

  // https://www.geeksforgeeks.org/arithmetic-operations-with-stdbitset-in-c/
  // Function to add two bitsets
  std::bitset<FlattenedSz> bitsetAdd(std::bitset<FlattenedSz>& x, std::bitset<FlattenedSz>& y)
  {
      bool carry = false;
      // bitset to store the sum of the two bitsets
      std::bitset<FlattenedSz> ans;
      for (int i = 0; i < FlattenedSz + 1; i++) {
          ans[i] = fullAdder(x[i], y[i], carry);
      }
      return ans;
  }

  void initRange(void)
  {
    ::std::size_t base2Log = ::std::log2(m_k);
    assert(base2Log > 0);

    m_partitionSzLog = FlattenedSz - base2Log;
    ::std::bitset<FlattenedSz> bitAdd;
    bitAdd.set(m_partitionSzLog);

    m_lowerRange = 0b0;
    for (::std::size_t i = 0; i < m_id; ++i)
    {
      m_lowerRange = bitsetAdd(m_lowerRange, bitAdd); 
    }

    m_upperRange = bitsetAdd(m_lowerRange, bitAdd).flip();
  }

public:
  KStateSpacePartition(void) = default;
  KStateSpacePartition(const ::std::size_t& id, 
    const ::std::size_t& k)
    : m_id(id),
      m_k(k)
  {
    initRange();
  }

  auto getRange(void)
  { return ::std::make_tuple(m_lowerRange, m_upperRange); }
};
#endif 

template<::std::size_t FlattenedSz, typename NonPlacementDataType, typename EvalFn,
  typename IsValidBoardFn=null_type>
void inline MPI_generateConfigCheckmates(int k, KStateSpacePartition<FlattenedSz, BoardState<FlattenedSz, NonPlacementDataType>> partitioner, 
    ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>>& losses,
    const ::std::vector<piece_label_t>& pieceSet,
    EvalFn checkmateEval,
    IsValidBoardFn boardValidityEval = {})
{
  ::std::array<::std::size_t, FlattenedSz> indexPermutations;
  auto [startFirstIdx, endFirstIdx] = partitioner.getRange(k);
  
  indexPermutations[0] = startFirstIdx;
  int j = 0;
  for (::std::size_t i = 1; i < indexPermutations().size; ++i)
  {
    if (i == startFirstIdx)
      ++j;
    indexPermutations[i] = j;
    ++j;
  }

  auto startBoard = indexPermutations;

  ::std::size_t kPermute = pieceSet.size();
  
  // generates kPermute new checkmate positions.
  // TODO: consider how to store smaller checkmates. Do we keep in storage somewhere?
  do 
  {
    BoardState<FlattenedSz, NonPlacementDataType> currentBoard;
    for (::std::size_t i = 0; i != kPermute; ++i)
      currentBoard.m_board[indexPermutations[i]] = pieceSet[i]; // scatter pieces

    if constexpr (!::std::is_same<null_type, IsValidBoardFn>::value)
      if (!IsValidBoardFn(currentBoard))
        continue;
    
    // checking if black loses (white wins) 
    if (checkmateEval(currentBoard))
    {
      losses.insert(currentBoard);
    }
    
    currentBoard.m_player = true;
    
    // checking if white loses (black wins)
    if (checkmateEval(currentBoard))
    {
      losses.insert(currentBoard);
    }

    ::std::reverse(indexPermutations.begin() + kPermute, indexPermutations.end());
    ::std::next_permutation(indexPermutations.begin(), indexPermutations.end());
  } while (partitioner.checkInRange(startBoard, indexPermutations));
}

template <typename WinFrontier, typename LoseFrontier, typename Partitioner, typename EndGameSet,
  typename PredecessorGen, typename BoardMap>
inline auto do_majorIteration(int id, int v, int numProcs, const Partitioner& p, 
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
        boardMap[loseFrontier[i].b] = { static_cast<short>(v), {}, {} }; // last two fields are only relevant to potentially lost states.  
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
      
      if (boardMap.find(winFrontier[i].b) == boardMap.end()) // replace these static casts
      {
        auto succs = succFn(winFrontier[i].b);
        boardMap[winFrontier[i].b] = 
        { 0, static_cast<short>(winFrontier[i].G), static_cast<short>(succs.size() - 1) }; 
      }

      else // decrement remaining moves by 1 
      {
        --(boardMap[winFrontier[i].b].C);
        boardMap[winFrontier[i].b].M = ::std::max(boardMap[winFrontier[i].b].M, static_cast<short>(winFrontier[i].G));
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
      MPI_Isend(&loseFrontier[0], 1, MPI_NodeCommData, i, 1, // need to send a 1 if it is the last msg
        MPI_COMM_WORLD, r);
    }
  }
  return ::std::make_tuple(b_lossFound, sendRequests, ::std::move(boardMap), 
    ::std::move(loseFrontier), ::std::move(losses));
}

// TODO: Consider more efficient communication scheme with MPI groups
template<typename BoardMap, typename Frontier> 
auto do_syncAndFree(int numNodes,
    ::std::vector<MPI_Request*> sendRequests,
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
    MPI_Recv(&recvBuf, 1, MPI_NodeCommData, MPI_ANY_SOURCE,
        MPI_ANY_TAG, MPI_COMM_WORLD, &status);

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
auto retrogradeAnalysisClusterImpl(KStateSpacePartition<FlattenedSz, BoardState<FlattenedSz, NonPlacementDataType>> partitioner, int id,
    ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>> checkmates,
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
  
  // ultimately invoke additional iteration
  for (const auto& l : losses)
  {
    auto preds = generatePredecessors(l);

    for (const auto& pred : preds)
    {
      auto target_id = partitioner(pred);
      if (target_id != id) // different node processes this
      {
        // mpi isend to target. should the predecessor also be sent? in the mark?
        // matt thinks yes but this doubles the size of the total send. think alternatives
      }
      else
      {
        loseFrontier.push_back({{}, {}, pred });
      }
    }
  }
  
  ::std::vector<MPI_Request*> sendRequests;
  int numProcs = 16;
  for (int v = 1; v > 0; ++v)
  {
    bool b_mark{};

    // 1. Invoke major iteration
    ::std::tie(b_mark, sendRequests, estimateData, winFrontier, wins) = do_majorIteration(id, v, numProcs,
        partitioner, ::std::move(estimateData), loseFrontier, ::std::move(winFrontier), losses,
        ::std::move(wins), generatePredecessors);
    
    // TODO: Synchronization routine
    ::std::tie(estimateData, winFrontier) = do_syncAndFree(numProcs, sendRequests, 
        ::std::move(estimateData), ::std::move(winFrontier));
    
    sendRequests.clear();
    loseFrontier.clear(); // must wait until after synchronization

    if (!b_mark)
      break;

    // 2. Invoke minor iteration
    ::std::tie(b_mark, sendRequests, estimateData, loseFrontier, losses) = do_minorIteration(id, v, numProcs,
      partitioner, ::std::move(estimateData), ::std::move(loseFrontier), winFrontier,
      ::std::move(losses), wins, generatePredecessors, generateSuccessors);
  
    // TODO: Synchronization routine
    ::std::tie(estimateData, loseFrontier) = do_syncAndFree(numProcs, sendRequests, 
        ::std::move(estimateData), ::std::move(loseFrontier));
    
    sendRequests.clear();
    winFrontier.clear(); // must wait until after synchronization 

    if (!b_mark)
      break; 
  }
  return ::std::make_tuple(wins, losses);
}

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
