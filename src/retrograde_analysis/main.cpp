#include "retrograde_analysis.hpp"
#include <cassert>
#include <iostream>
#ifndef HZ_SYM_EVALUATOR
#  define HZ_SYM_EVALUATOR false_fn
#endif
#ifndef VT_SYM_EVALUATOR
#  define VT_SYM_EVALUATOR false_fn
#endif
#ifndef IS_VALID_BOARD_FN
#  define IS_VALID_BOARD_FN null_type
#endif

auto readClArgs(int argc, char* argv[])
{
  // read the pieceset configuration as --pieceset or something similar..
  // Return
}

// look out for clashing of macros with actual function names.
int main(int argc, char* argv[]){
#if defined ROW_SZ && defined COL_SZ && defined N_MAN
constexpr std::size_t FLATTENED_SZ = ROW_SZ * COL_SZ;

FORWARD_MOVE_GENERATOR forward;
REVERSE_MOVE_GENERATOR reverse;

HZ_SYM_EVALUATOR hzSymmetryCheck;
VT_SYM_EVALUATOR vtSymmetryCheck;
IS_VALID_BOARD_FN isValidBoardFn;
WIN_COND_EVALUATOR winEval;


std::vector<piece_label_t> noRoyaltyPieceset = NO_ROYALTY_PIECESET;
std::vector<piece_label_t> royaltyPieceset = ROYALTY_PIECESET;

//invoke retrograde analysis and checkmate generator with paramaters passed
// worry about this later - most users will have a custom process due to how these
// are scheduled
#ifndef MULTI_NODE
auto checkmates = generateAllCheckmates<FLATTENED_SZ, NON_PLACEMENT_DATATYPE, N_MAN, ROW_SZ, COL_SZ, WIN_COND_EVALUATOR, 
  HZ_SYM_EVALUATOR, VT_SYM_EVALUATOR, IS_VALID_BOARD_FN>(noRoyaltyPieceset, royaltyPieceset, winEval, 
  hzSymmetryCheck, vtSymmetryCheck, isValidBoardFn); 

#else
// checkmates will look something like this (with everything adjusted)
auto checkmates = generateConfigCheckmates<FLATTENED_SZ, NON_PLACEMENT_DATATYPE, N_MAN, ROW_SZ, COL_SZ,
      decltype(winCondEvaluator)/*, symmetries go here*/>(fullPieceset, winCondEvaluator/*and here*/);

// retrograde analysis will look something like this
auto [wins, losses, dtm] = retrogradeAnalysisBaseImpl<FLATTENED_SZ, NON_PLACEMENT_DATATYPE, N_MAN, ROW_SZ, 
    COL_SZ, decltype(forward), decltype(reverse)>(::std::move(checkmates),
    forward, reverse);

std::cout << wins.size() << " " << losses.size() << std::endl;
#endif
#else
std::cerr << "ERROR: Invalid configuration file" << std::endl;
assert(false);
#endif
}
