#ifndef CLUSTER_SUPPORT_H_
#define CLUSTER_SUPPORT_H_

#include <tuple>
#include <bitset>
#include <cmath>
#include <cassert>
#include <iostream>
#include <cstddef>

#include <mpi.h>

#include "state.hpp"

MPI_Datatype MPI_NodeCommData;
MPI_Datatype MPI_NonPlacementDataType;
MPI_Datatype MPI_BoardState;

template <::std::size_t FlattenedSz, typename NonPlacementDataType> 
struct NodeCommData 
{
  bool isWin;
  // D(k) in algorithm
  int depthToEnd;
  // M(k) in algorithm
  int remainingValidMoves;
  int G;

  // TODO: does this need to be sent?
  BoardState<FlattenedSz, NonPlacementDataType> b; 
};

template <::std::size_t FlattenedSz, typename NonPlacementDataType> 
void initialize_comm_structs(void)
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
    
    // TODO: may need to change the MPI_CHAR pending the chu shogi representation changes
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
// uses modulo partitioning on the sum of the gathered indices
// Matt TODO: Generate an unordered map of indices s.t. they corresond 
// to a pseudorandom number. Take the sum (or some other function) of the pseudorandom
// numbers mod the number of nodes to achieve a more even distribution. Nodes 
// would have to generate a random number for themselves and then broadcast to all other nodes first.
template <::std::size_t FlattenedSz, int K, typename BoardType>
class KStateSpacePartition
{
public:
  KStateSpacePartition(void) = default;
  
  // Matt TODO:  Suspect that this results in a heavily skewed distribution. see above
  // returns the rank of the node reponsible for this board state
  int operator()(const BoardType& b)
  {
    int sum = 0;
    int idx = 0;

    // gather indices
    for (const auto& c : b.m_board)
    {
      if (c != '\0')
        sum += idx;
      ++idx;
    }
    return sum % K;
  }
};

// TODO: remove this 
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

#endif
