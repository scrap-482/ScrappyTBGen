/*
* Copyright 2022 SCRAP
*
* This file is part of Scrappy Tablebase Generator.
* 
* Scrappy Tablebase Generator is free software: you can redistribute it and/or modify it under the terms 
* of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, 
* or (at your option) any later version.
* 
* Scrappy Tablebase Generator is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Scrappy Tablebase Generator. If not, see <https://www.gnu.org/licenses/>.
*/


#include "../../src/retrograde_analysis/state_transition.hpp"
#include "../../src/retrograde_analysis/checkmate_generation.hpp"
#include "../../src/retrograde_analysis/retrograde_analysis.hpp"

#include "../../src/rules/chess/interface.h"

#include <cassert>
#include <iostream>
#include <chrono>

template<typename BoardType, typename MapType, typename SuccFn>
auto probe(const BoardType& b, const MapType& m, SuccFn succFn, bool isWinIteration)
{
  BoardType g = b;
  int depthToEnd = 0;
  auto v = m.at(g);
  auto print = ChessBoardPrinter();
  std::vector<BoardType> pathwayToEnd = {g};
  for (;;)
  {
    auto succs = succFn(g);
    if (succs.size() == 0)
      break;
    std::cout << print(g) << std::endl;
    for (const auto& succ : succs)
    {
      // exploring a draw state
      if (m.find(succ) == m.end())
        continue;
      auto lvlToEnd = m.at(succ);
      // levels are doubled up per move
      if (isWinIteration && lvlToEnd == v - 1) 
      {
        g = succ;
        pathwayToEnd.push_back(g);
        ++depthToEnd;
        v = lvlToEnd;
        break;
      }
      else if (!isWinIteration && lvlToEnd == v)
      {
        g = succ;
        pathwayToEnd.push_back(g);
        ++depthToEnd;
        v = lvlToEnd;
        break;
      }
    }
    if (v == 0)
      break;
    isWinIteration = !isWinIteration;
  }
  return std::make_tuple(depthToEnd, pathwayToEnd);
}
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
  auto fwdMoveGenerator = ChessGenerateForwardMoves();
  auto revMoveGenerator = ChessGenerateReverseMoves();
  auto winCondEvaluator = ChessCheckmateEvaluator();
  auto boardPrinter = ChessBoardPrinter();
  auto validityEvaluator = ChessValidBoardEvaluator();

  constexpr ::std::size_t N      = 4;
  constexpr ::std::size_t ROW_SZ = 8;
  constexpr ::std::size_t COL_SZ = 8;
  
  std::vector<piece_label_t> noRoyaltyPieceset = { 'q', 'R' };
  std::vector<piece_label_t> royaltyPieceset = { 'k', 'K' };
  std::vector<piece_label_t> fullPieceset = { 'K', 'k', 'q', 'R'};

  //std::vector<piece_label_t> noRoyaltyPieceset = { 'q' };
  //std::vector<piece_label_t> royaltyPieceset = { 'k', 'K' };        
  //std::vector<piece_label_t> fullPieceset = { 'K', 'k', 'q' };

#ifndef MULTI_NODE  
  std::string str_num_threads{ std::getenv("OMP_NUM_THREADS") };
  
  auto checkmates = generateParallelConfigCheckmates<64, ChessNPD, N, ROW_SZ, COL_SZ,
       decltype(winCondEvaluator), decltype(validityEvaluator)>(fullPieceset, winCondEvaluator, validityEvaluator);

  auto t0 = std::chrono::high_resolution_clock::now();
  auto [wins, losses, dtm] = retrogradeAnalysisBaseImpl<64, ChessNPD, N, ROW_SZ, 
    COL_SZ, decltype(fwdMoveGenerator), decltype(revMoveGenerator)>(::std::move(checkmates),
      fwdMoveGenerator, revMoveGenerator);
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
  
  localCheckmates = generatePartitionCheckmates<64>(rank, partitioner, 
      std::move(localCheckmates), fullPieceset, winCondEvaluator); 
  
  // wait until everyone is done before logging the time 
  auto t0 = std::chrono::high_resolution_clock::now();
  auto [wins, losses, dtm] = retrogradeAnalysisClusterImpl<64, ChessNPD, N, ROW_SZ, COL_SZ, 
    decltype(fwdMoveGenerator), decltype(revMoveGenerator)>(partitioner, rank, global_sz, 
    std::move(localCheckmates), fwdMoveGenerator, revMoveGenerator);
  auto t1 = std::chrono::high_resolution_clock::now();
  auto runtime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
  
  std::cout << global_sz << " " << wins.size() << " " << losses.size() << std:;endl;
  // after sync, one node should output elapsed time 
  if (rank == 0)
    std::cout << global_sz << "," << runtime << std::endl;

  MPI_Finalize();
#endif
  
  //// TODO: remove. this is experimentation not validation.
  //// Farthest state to end with optimal play for 3 man.
  //// This is essentially the last state visited in the retrograde
  //// analysis implementation
  //BoardState<64, ChessNPD> longest3ManDTM
  //{
  //  true,
  //  {
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0', 'K','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0', 'q','\0','\0','\0','\0','\0','\0',
  //    'k', '\0','\0','\0','\0','\0','\0','\0',
  //  },
  //  {-1}
  //};

  //auto [lengthToEnd, pathway] = probe(longest3ManDTM, dtm, fwdMoveGenerator, false);
  //std::cout << "Number of moves until checkmate with optimal play: " << lengthToEnd << std::endl;
  return 0;
}
