#ifndef MULTI_NODE_IMPL_HPP_ 
#define MULTI_NODE_IMPL_HPP_

#include <tuple>
#include <bitset>
#include <cmath>
#include <cassert>
#include <iostream>
#include <cstddef>

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
  bool isWin;
  // D(k) in algorithm - can remove from mpi send
  int depthToEnd;
  // M(k) in algorithm - can remove from mpi send
  int remainingValidMoves;
  int G;

  BoardState<FlattenedSz, NonPlacementDataType> b; 
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
  int operator()(const BoardType& b)
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

template <typename WinFrontier, typename LoseFrontier, typename Partitioner, typename WinSet,
  typename PredecessorGen, typename BoardMap>
inline auto do_majorIteration(int id, int v, const Partitioner& p, 
    BoardMap&& boardMap, const auto& LoseFrontier l, WinFrontier&& w,
    WinSet&& wins, PredecessorGen predFn)
{  
  // TODO: See if finer grain parallelism can be employed with OpenMP (otherwise just launch more
  // MPI processes)
  bool b_winFound = false;
  for (::std::size_t i = 0; i < l.size(); ++i)
  {
    if (wins.find(loseFrontier[i]) == wins.end())
    {
      if (!b_winFound)
        b_winFound = true;

      auto preds = predFn(loseFrontier[i]);
      wins.insert(loseFrontier[i]);

      for (auto&& pred : prds)
      {
        // TODO: construct NodeCommStruct type. Figure out best way to persist until the 
        // MPI send is consumed 
        auto targetId = p(pred);
        if (targetID == id)
          winFrontier.push_back(std::move(pred));
        else
        {
          // be careful about the address
          // perform MPI send from current to targetId
        }
      }
    }
  }
  return ::std::make_tuple(b_winFound,::std::move(w), ::std::move(wins));
}

template <typename WinFrontier, typename LoseFrontier, typename Partitioner, typename LoseSet, 
  typename PredecessorGen, typename SuccessorGen, typename BoardMap>
inline auto do_minorIteration(int id, int v, const Partitioner& p, 
    BoardMap&& boardMap, LoseFrontier&& l, const WinFrontier& w,
    LoseSet&& losses, PredecessorGen predFn, SuccessorGen succFn)
{
  // TODO: See if finer grain parallelism can be employed with OpenMP (otherwise just launch more
  // MPI processes)
  bool b_lossFound = false;
  for (::std::size_t i = 0; i < w.size(); ++i)
  {
    if (losses.find(winFrontier[i]) == losses.end() && wins.find(winFrontier[i]) == wins.end()) 
    {
      if (!b_lossFound)
        b_lossFound = true;
      auto succs = succFn(winFrontier[i]);
      bool allWins = true;
      for (const auto& succ : succs)
      {
        if (wins.find(succ) == wins.end())
        {
          allWins = false;
          break;
        }
      }
      if (allWins)
      {
        // localLosses.push_back(winFrontier[i]);
        auto preds = predFn(winFrontier[i]);
        for (auto&& pred : predecessors)
        {
          auto targetId = p(pred);
          if (targetId == id)
            loseFrontier.push_back(::std::move(pred));
          else
          {
            // be careful about the address
            // perform MPI send from current to targetId
          }
        }
      }
    }
  }
  return ::std::make_tuple(b_lossFound, ::std::move(l), ::std::move(losses));
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
  using board_set_t = ::std::unordered_set<NodeCommData<FlattenedSz, NonPlacementDataType>, 
    BoardStateHasher<FlattenedSz, NonPlacementDataType>>;
  using frontier_t = ::std::vector<NodeCommData<FlattenedSz, NonPlacementDataType>>;
  
  // Estimate data during search - more expensive than omp implementation 
  // BoardState b -> (M b, D b) 
  using board_map_t = 
    ::std::unordered_map<BoardState<FlattenedSz, NonPlacementDataType>, 
    ::std::tuple<short, short, short>, // really want to compress data. 
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
        // mpi isend to target
      }
      else
      {
        loseFrontier.push_back(pred);
      }
    }
  }

  for (int v = 1; v > 0; ++v)
  {
    bool b_mark{};

    // 1. Invoke major iteration
    ::std::tie(b_mark, estimateData, winFrontier, wins) = do_majorIteration(id, v, partitioner, 
        ::std::move(estimateData), loseFrontier, ::std::move(winFrontier), 
        ::std::move(wins), generatePredecessors);
    
    // TODO: Synchronization routine
    
    loseFrontier.clear(); // must wait until after at the moment
    if (!b_mark)
      break;

    // 2. Invoke minor iteration
    
    ::std::tie(b_mark, estimateData, loseFrontier, losses) = do_minorIteration(id, v, partitioner,
      ::std::move(estimateData), ::std::move(loseFrontier), winFrontier,
      ::std::move(losses), generatePredecessors, generateSuccessors);
    // TODO: Synchronization routine

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

  //auto results = retrogradeAnalysisClusterImpl(
  MPI_Finalize();
}

#endif
