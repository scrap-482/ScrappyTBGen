// Parallel checkmate identification timing
// Running on a 4-man configuration of kqvKR

#include "../../src/retrograde_analysis/state_transition.hpp"
#include "../../src/retrograde_analysis/checkmate_generation.hpp"
#include "../../src/retrograde_analysis/retrograde_analysis.hpp"

#include "../../src/rules/chess/interface.h"

#include <iostream>
#include <chrono>
#include <cstdlib>

#ifdef MULTI_NODE
void initialize_chess_non_placement(void)
{
  { // serialize Board State
    // C++ preprocessor does not understand template syntax so this is necessary
    int count = 1;
    int blocklengths[] = { 1 };

    MPI_Aint displacements[] = 
    { 
      offsetof(ChessNPD, enpassantRights), 
    };
    
    MPI_Datatype types[] = { MPI_INT };
    
    MPI_Datatype tmp;
    MPI_Aint lowerBound;
    MPI_Aint extent;

    MPI_Type_create_struct(count, blocklengths, displacements, types,
      &tmp);

    MPI_Type_get_extent(tmp, &lowerBound, &extent);
    MPI_Type_create_resized(tmp, lowerBound, extent, &MPI_NonPlacementDataType);
    MPI_Type_commit(&MPI_NonPlacementDataType);
  }
}
#endif

int main()
{
  constexpr ::std::size_t N      = 4;
  constexpr ::std::size_t ROW_SZ = 8;
  constexpr ::std::size_t COL_SZ = 8;
  
  auto winCondEvaluator = ChessCheckmateEvaluator();
  auto validityEvaluator = ChessValidBoardEvaluator();
 
  std::vector<piece_label_t> noRoyaltyPieceset = { 'q', 'R' };
  std::vector<piece_label_t> royaltyPieceset = { 'k', 'K' };

  std::vector<piece_label_t> fullPieceset = { 'k', 'K', 'q', 'R' };

  // OMP single node threading implementation assessment  
#ifndef MULTI_NODE 
  std::string str_num_threads{ std::getenv("OMP_NUM_THREADS") };
  
  // consider iterating n number of times and generating confidence interval
  auto t0 = std::chrono::high_resolution_clock::now();
  auto checkmates = generateParallelConfigCheckmates<64, ChessNPD, N, ROW_SZ, COL_SZ,
    decltype(winCondEvaluator), decltype(validityEvaluator)>(fullPieceset, winCondEvaluator, validityEvaluator);
  auto t1 = std::chrono::high_resolution_clock::now();
  auto runtime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
  std::cout << str_num_threads << "," << runtime << std::endl;
#else
  MPI_Init(NULL, NULL);
  
  initialize_chess_non_placement();
  initialize_comm_structs<64, ChessNPD>();

  int global_sz = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &global_sz); 

  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
  KStateSpacePartition<64, BoardState<64, ChessNPD>> partitioner(fullPieceset[0], global_sz); 
  std::unordered_set<BoardState<64, ChessNPD>, BoardStateHasher<64, ChessNPD>> localCheckmates;
  
  auto t0 = std::chrono::high_resolution_clock::now();
  localCheckmates = generatePartitionCheckmates<64>(rank, partitioner, 
      std::move(localCheckmates), fullPieceset, winCondEvaluator); 
  
  // wait until everyone is done before logging the time 
  MPI_Barrier(MPI_COMM_WORLD);

  auto t1 = std::chrono::high_resolution_clock::now();
  auto runtime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

  // after sync, one node should output elapsed time 
  if (rank == 0)
    std::cout << global_sz << "," << runtime << std::endl;

  MPI_Finalize();
  
  #endif
  return 0;
}
