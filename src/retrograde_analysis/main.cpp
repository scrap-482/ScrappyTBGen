#include "retrograde_analysis.hpp"
#include <cassert>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <cctype>
#include <chrono>

#include "probe.hpp"

// preprocessor checks and defs for optional fields
#ifndef HZ_SYM_EVALUATOR
#  define HZ_SYM_EVALUATOR false_fn
#endif
#ifndef VT_SYM_EVALUATOR
#  define VT_SYM_EVALUATOR false_fn
#endif
#ifndef IS_VALID_BOARD_FN
#  define IS_VALID_BOARD_FN null_type
#endif

// reads in algebraic notation for stdin and returns corresonding board
// for giving pieceset
template <typename BoardType>
auto readBoardInput(const std::vector<piece_label_t>& pieceset)
{
  BoardType b;
  std::cout << "Which side to move (w or b)? ";
  
  char move;
  std::cin >> move;
  std::cout << std::endl;

  if (move == 'b')
    b.m_player = false;
  else
    b.m_player = true;

  for (const auto c : pieceset)
  {
    std::cout << "Enter the position for " << c << " in algebraic notation: ";

    int row;
    char ccol;

    std::cin >> ccol;
    std::cin >> row;
    std::cout << std::endl;
    
    ccol = std::tolower(ccol);
    int col = ccol - 'a';
    
    int pos = COL_SZ * (row-1) + col;

    b.m_board[pos] = c;
  }
  
  return b;
}

auto readClArgs(int argc, char* argv[], 
  const std::vector<piece_label_t>& royaltyPieceset)
{
  if (argc == 1)
  {
    std::cerr << "ERROR: Please enter a pieceset configuration" << std::endl;
    assert(false);
  }
  std::string fullset = argv[1];

  std::vector<piece_label_t> userset;
  for(int i = 0; i < fullset.length(); i++){
    userset.push_back(fullset.at(i));
  }
  //sort royalty pieces first (assuming we have royalty_set)
  std::sort(userset.begin(), userset.end(), 
    [&royaltyPieceset = std::as_const(royaltyPieceset)]
    (const char fst, const char snd)
    {
      // fst is royalty, let us put it first
      if (std::find(royaltyPieceset.begin(), royaltyPieceset.end(), fst) 
        != royaltyPieceset.end())
        return true;
      // snd is royalty (and fst is not), so we put snd before fst
      else if (std::find(royaltyPieceset.begin(), royaltyPieceset.end(),
        snd) != royaltyPieceset.end())
        return false;
      return true; // neither piece is royalty, so we don't care
    });

  return userset;
}

// look out for clashing of macros with actual function names
int main(int argc, char* argv[])
{
#if defined ROW_SZ && defined COL_SZ && defined N_MAN && defined FORWARD_MOVE_GENERATOR          \
  && defined REVERSE_MOVE_GENERATOR && defined WIN_COND_EVALUATOR && defined NO_ROYALTY_PIECESET \
  && defined ROYALTY_PIECESET
  constexpr std::size_t FLATTENED_SZ = ROW_SZ * COL_SZ;
  
  std::vector<piece_label_t> noRoyaltyPieceset = NO_ROYALTY_PIECESET;
  std::vector<piece_label_t> royaltyPieceset = ROYALTY_PIECESET;

  std::vector<piece_label_t> fullPieceset = readClArgs(argc, argv, royaltyPieceset);


  FORWARD_MOVE_GENERATOR forward;
  REVERSE_MOVE_GENERATOR reverse;

  HZ_SYM_EVALUATOR hzSymmetryCheck;
  VT_SYM_EVALUATOR vtSymmetryCheck;
  IS_VALID_BOARD_FN isValidBoardFn;
  WIN_COND_EVALUATOR winEval;

  // worry about this later - most users will have a custom process due to how these
  // are scheduled
#ifndef MULTI_NODE
  auto t0 = std::chrono::high_resolution_clock::now();
  auto checkmates = generateParallelConfigCheckmates<FLATTENED_SZ, NON_PLACEMENT_DATATYPE, N_MAN, ROW_SZ, COL_SZ,
         decltype(winEval), decltype(isValidBoardFn)>(fullPieceset, winEval, isValidBoardFn);
  auto t1 = std::chrono::high_resolution_clock::now();
  auto cmDuration = std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count();
  
  t0 = std::chrono::high_resolution_clock::now();
  auto [wins, losses, dtm] = retrogradeAnalysisBaseImpl<FLATTENED_SZ, NON_PLACEMENT_DATATYPE, N_MAN, ROW_SZ, 
      COL_SZ, decltype(forward), decltype(reverse)>(::std::move(checkmates),
      forward, reverse);
  t1 = std::chrono::high_resolution_clock::now();
  auto rgDuration = std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count();
  
  std::cout << "-----------------------------------------" << std::endl;
  std::cout << "Checkmate identification execution time: " << cmDuration << " ms" << std::endl;
  std::cout << "Retrograde analysis execution time: " << rgDuration << " ms" << std::endl;
  std::cout << "-----------------------------------------" << std::endl;
  std::cout << "Number of wins: " << wins.size() << " Number of losses: " << losses.size() << std::endl; 
  std::cout << "-----------------------------------------" << std::endl;
  // todo: adjust this to be generic
  auto boardPrinter = ChessBoardPrinter();

  // repeatedly ask the user for queries until they wish to quit
  bool loop = true; 
  do {
    auto boardToQuery = readBoardInput<decltype(checkmates)::value_type>(fullPieceset);

    bool iteration;
    if (wins.find(boardToQuery) != wins.end())
    {
      iteration = true;
      auto [depthToEnd, path] = probe(boardToQuery, dtm, forward, iteration, boardPrinter);
      std::cout << "Number of moves until the end: " << depthToEnd << std::endl;
    }
    else if (losses.find(boardToQuery) != losses.end())
    {
      iteration = false;
      auto [depthToEnd, path] = probe(boardToQuery, dtm, forward, iteration, boardPrinter);
      std::cout << "Number of moves until the end: " << depthToEnd << std::endl;
    }
    else
      std::cout << "Board is a stalemate" << std::endl;

    std::cout << "Would you like to query another state (y or n)?" << std::endl;
    char nextQuery;
    std::cin >> nextQuery;

    if (nextQuery == 'y')
      loop = true;
    else
      loop = false;
  } while (loop);
     
#else
  MPI_Init(NULL, NULL);
  
  //initialize_chess_non_placement();
  // user must initialize non placement type
  //initialize_comm_structs<64, ChessNPD>();

  int global_sz = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &global_sz); 

  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
  KStateSpacePartition<64, BoardState<64, ChessNPD>> partitioner(fullPieceset[0], global_sz); 
  std::unordered_set<BoardState<64, ChessNPD>, BoardStateHasher<64, ChessNPD>> localCheckmates;
  
  localCheckmates = generatePartitionCheckmates<64>(rank, partitioner, 
      std::move(localCheckmates), fullPieceset, winEval); 
  
  // wait until everyone is done before logging the time 
  auto t0 = std::chrono::high_resolution_clock::now();
  auto [wins, losses, dtm] = retrogradeAnalysisClusterImpl<64, NON_PLACEMENT_DATATYPE, N_MAN, ROW_SZ, COL_SZ, 
    decltype(forward), decltype(reverse)>(partitioner, rank, global_sz, 
    std::move(localCheckmates), forward, reverse);
  auto t1 = std::chrono::high_resolution_clock::now();
  auto runtime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
  
  std::cout << global_sz << " " << wins.size() << " " << losses.size() << std::endl;
  // after sync, one node should output elapsed time 
  if (rank == 0)
    std::cout << global_sz << "," << runtime << std::endl;

  MPI_Finalize();
#endif
#else
  std::cerr << "ERROR: Invalid configuration file" << std::endl;
  assert(false);
#endif
}
