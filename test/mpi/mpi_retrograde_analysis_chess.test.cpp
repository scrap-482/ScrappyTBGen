#include <iostream>

#ifdef MULTI_NODE
#include "../../src/retrograde_analysis/retrograde_analysis.hpp"
#include "../../src/retrograde_analysis/state_transition.hpp"

#include "../../src/rules/chess/interface.h"

int main()
{
  auto fwdMoveGenerator = ChessGenerateForwardMoves();
  auto revMoveGenerator = ChessGenerateReverseMoves();
  auto winCondEvaluator = ChessCheckmateEvaluator();
  auto boardPrinter = ChessBoardPrinter();

  constexpr ::std::size_t N      = 3;
  constexpr ::std::size_t ROW_SZ = 8;
  constexpr ::std::size_t COL_SZ = 8;
  
  // TODO: This is hardcoded for now. playout for correctness
  std::vector<piece_label_t> noRoyaltyPieceset = { 'q' };
  std::vector<piece_label_t> royaltyPieceset = { 'k', 'K' };

  std::vector<piece_label_t> fullPieceset = { 'k', 'K', 'q' };
  
  MPI_Init(NULL, NULL);

  // calculate MPI global processes
  int globalSz = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &globalSz);
  int localRank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &localRank);

  KStateSpacePartition<64, BoardState<64, ChessNPD>> partitioner(fullPieceset[0], globalSz); 
  
  std::unordered_set<BoardState<64, ChessNPD>, BoardStateHasher<64, ChessNPD>> localCheckmates;

  localCheckmates = MPI_generateConfigCheckmates<64>(localRank, partitioner, 
      std::move(localCheckmates), fullPieceset, winCondEvaluator); 

  std::cout << "node: " << localRank << " checkmates found: " << localCheckmates.size() << std::endl;
    
  MPI_Finalize();
  return 0;
}

#else
int main()
{
  std::cerr << "ERROR: codebase not built with -DMULTI_NODE option." << std::endl;
}
#endif
