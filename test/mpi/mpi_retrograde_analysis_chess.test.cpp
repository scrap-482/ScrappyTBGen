#include <iostream>

#ifdef MULTI_NODE
#include "../../src/retrograde_analysis/retrograde_analysis.hpp"
#include "../../src/retrograde_analysis/state_transition.hpp"

#include "../../src/rules/chess/interface.h"

// still need to fully figure out what to do here
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

int main()
{
  auto fwdMoveGenerator = ChessGenerateForwardMoves();
  auto revMoveGenerator = ChessGenerateReverseMoves();
  auto winCondEvaluator = ChessCheckmateEvaluator();
  auto boardPrinter = ChessBoardPrinter();

  constexpr ::std::size_t N      = 3;
  constexpr ::std::size_t ROW_SZ = 8;
  constexpr ::std::size_t COL_SZ = 8;
  
  std::vector<piece_label_t> noRoyaltyPieceset = { 'q' };
  std::vector<piece_label_t> royaltyPieceset = { 'k', 'K' };

  std::vector<piece_label_t> fullPieceset = { 'k', 'K', 'q' };
  
  MPI_Init(NULL, NULL);
  initialize_chess_non_placement();
  initialize_comm_structs<64, ChessNPD>();

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
  
  auto [wins, losses /*, dtm*/] = retrogradeAnalysisClusterImpl<64, ChessNPD, N, ROW_SZ, COL_SZ, 
    decltype(fwdMoveGenerator), decltype(revMoveGenerator)>(partitioner, localRank, globalSz, 
    std::move(localCheckmates), fwdMoveGenerator, revMoveGenerator);

  MPI_Finalize();
  return 0;
}

#else
int main()
{
  std::cerr << "ERROR: codebase not built with -DMULTI_NODE option." << std::endl;
}
#endif
