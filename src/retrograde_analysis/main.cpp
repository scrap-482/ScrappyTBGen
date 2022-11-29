// TODO:
// MPI stuff
// Board printing
#include "retrograde_analysis.hpp"
#include <cassert>
#include <iostream>
#include <algorithm>
#include <stdio.h>

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

auto readClArgs(int argc, char* argv[], 
  const std::vector<piece_label_t>& royaltyPieceset)
{
  if (argc == 0)
  {
    std::cerr << "ERROR: Please enter a pieceset confiuration" << std::endl;
    assert(false);
  }
  //does not account for commas
  std::string fullset = argv[1]; //see if argc > 1???

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
  std::vector<piece_label_t> noRoyaltyPieceset = NO_ROYALTY_PIECESET;
  std::vector<piece_label_t> royaltyPieceset = ROYALTY_PIECESET;

  std::vector<piece_label_t> fullPieceset = readClArgs(argc, argv, royaltyPieceset);

#if defined ROW_SZ && defined COL_SZ && defined N_MAN && defined FORWARD_MOVE_GENERATOR \
  && defined REVERSE_MOVE_GENERATOR && defined WIN_COND_EVALUATOR
  constexpr std::size_t FLATTENED_SZ = ROW_SZ * COL_SZ;

  FORWARD_MOVE_GENERATOR forward;
  REVERSE_MOVE_GENERATOR reverse;

  HZ_SYM_EVALUATOR hzSymmetryCheck;
  VT_SYM_EVALUATOR vtSymmetryCheck;
  IS_VALID_BOARD_FN isValidBoardFn;
  WIN_COND_EVALUATOR winEval;

  // worry about this later - most users will have a custom process due to how these
  // are scheduled
#ifndef MULTI_NODE
  auto checkmates = generateParallelConfigCheckmates<FLATTENED_SZ, NON_PLACEMENT_DATATYPE, N_MAN, ROW_SZ, COL_SZ,
         decltype(winEval), decltype(isValidBoardFn)>(fullPieceset, winEval, isValidBoardFn);

  auto [wins, losses, dtm] = retrogradeAnalysisBaseImpl<FLATTENED_SZ, NON_PLACEMENT_DATATYPE, N_MAN, ROW_SZ, 
      COL_SZ, decltype(forward), decltype(reverse)>(::std::move(checkmates),
      forward, reverse);

  std::cout << "Number of wins: " << wins.size() << " Number of losses: " << losses.size() << std::endl; 
  // todo: adjust this to be generic
  auto boardPrinter = ChessBoardPrinter();

  //BoardState<64, ChessNPD> longest3ManDTM
  //{
  //  true,
  //  {
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','r','K','\0','\0','\0','\0',
  //    '\0','k','\0','\0','\0','\0','\0','\0',
  //  },
  //  {-1}
  //};
  BoardState<64, ChessNPD> longest3ManDTM
  {
    true,
    {
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0', 'K','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0', 'q','\0','\0','\0','\0','\0','\0',
      'k', '\0','\0','\0','\0','\0','\0','\0',
    },
    {-1}
  };

  
  bool iteration;
  if (wins.find(longest3ManDTM) != wins.end())
    iteration = true;
  else if (losses.find(longest3ManDTM) != losses.end())
    iteration = false;
  else
    assert(false);
    
  auto [depthToEnd, path] = probe(longest3ManDTM, dtm, forward, iteration, boardPrinter);
  std::cout << "Number of moves until the end: " << depthToEnd << std::endl;

#else
  auto checkmates = generateParallelConfigCheckmates<FLATTENED_SZ, NON_PLACEMENT_DATATYPE, N_MAN, ROW_SZ, COL_SZ,
         decltype(winEval)>(fullPieceset, winEval);

  auto [wins, losses, dtm] = retrogradeAnalysisBaseImpl<FLATTENED_SZ, NON_PLACEMENT_DATATYPE, N_MAN, ROW_SZ, 
      COL_SZ, decltype(forward), decltype(reverse)>(::std::move(checkmates), forward, reverse);

  std::cout << "Number of wins: " << wins.size() << " Number of losses: " << losses.size() << std::endl; 
#endif
#else
  std::cerr << "ERROR: Invalid configuration file" << std::endl;
  assert(false);
#endif
}
