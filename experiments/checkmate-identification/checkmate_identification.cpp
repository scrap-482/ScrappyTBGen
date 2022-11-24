// Parallel checkmate identification timing
// Running on a 4-man configuration of kqvKR

#include "../../src/retrograde_analysis/state_transition.hpp"
#include "../../src/retrograde_analysis/checkmate_generation.hpp"
#include "../../src/retrograde_analysis/retrograde_analysis.hpp"

#include "../../src/rules/chess/interface.h"

#include <iostream>
#include <chrono>
#include <cstdlib>

int main()
{
  // OMP single node threading implementation assessment  
#ifndef MULTI_NODE
  constexpr ::std::size_t N      = 4;
  constexpr ::std::size_t ROW_SZ = 8;
  constexpr ::std::size_t COL_SZ = 8;
  
  auto winCondEvaluator = ChessCheckmateEvaluator();
  auto validityEvaluator = ChessValidBoardEvaluator();

  std::string str_num_threads{ std::getenv("OMP_NUM_THREADS") };
  
  std::vector<piece_label_t> noRoyaltyPieceset = { 'q', 'R' };
  std::vector<piece_label_t> royaltyPieceset = { 'k', 'K', 'R' };

  std::vector<piece_label_t> fullPieceset = { 'k', 'K', 'q', 'R' };
  
  // consider iterating n number of times and generating confidence interval
  auto t0 = std::chrono::high_resolution_clock::now();
  auto checkmates = generateParallelConfigCheckmates<64, ChessNPD, N, ROW_SZ, COL_SZ,
    decltype(winCondEvaluator), decltype(validityEvaluator)>(fullPieceset, winCondEvaluator, validityEvaluator);
  auto t1 = std::chrono::high_resolution_clock::now();
  
  auto runtime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
  std::cout << str_num_threads << "," << runtime << std::endl;
#else
  // TODO: mpi
#endif
  return 0;
}
