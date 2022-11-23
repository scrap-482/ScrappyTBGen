#include "retrograde_analysis.hpp"
#include <cassert>
#include <iostream>
#ifndef hzSymEvaluator
#  define hzSymEvaluator false_fn
#endif
#ifndef vtSymEvaluator
#  define vtSymEvaluator false_fn
#endif
#ifndef isValidBoardFn
#  define isValidBoardFn null_type
#endif

// look out for clashing of macros with actual function names.
int main(int argc, char* argv[]){
#if defined rowSz && defined colSz && defined N
constexpr std::size_t flattenedSz = rowSz * colSz;
forwardMoveGenerator<flattenedSz, nonPlacementDatatype> forward;
reverseMoveGenerator<flattenedSz, nonPlacementDatatype> reverse;

hzSymEvaluator hzSymmetryCheck;
vtSymEvaluator vtSymmetryCheck;

std::vector<piece_label_t> noRoyaltyPieceset = { 'Q' };
std::vector<piece_label_t> royaltyPieceset = { 'k', 'K' };

std::vector<piece_label_t> fullPieceset = { 'k', 'K', 'q' };

//invoke retrograde analysis and checkmate generator with paramaters passed
// worry about this later - most users will have a custom process due to how these
// are scheduled
#ifndef MULTI_NODE
auto checkmates = generateAllCheckmates<flattenedSz, nonPlacementDatatype, N, rowSz, colSz, winCondEvaluator, 
  hzSymEvaluator, vtSymEvaluator, isValidBoardFn>(noRoyaltyPieceset, royaltyPieceset, winCondEvaluator eval, 
  hzSymEvaluator hzSymFn={}, vtSymEvaluator vSymFn={}, isValidBoardFn isValidBoardFn={}); 

retrograde_analysis<MachineType::SINGLE_NODE, flattenedSz, nonPlacementDatatype, N, 
  rowSz, colSz, decltype(forward), decltype(reverse), hzSymEvaluator, vtSymEvaluator, 
  isValidBoardFn>(Args&&... args);
#else
// checkmates will look something like this (with everything adjusted)
auto checkmates = generateConfigCheckmates<flattenedSz, nonPlacementDatatype, N, rowSz, colSz,
      decltype(winCondEvaluator)/*, symmetries go here*/>(fullPieceset, winCondEvaluator/*and here*/);

// retrograde analysis will look something like this
auto [wins, losses, dtm] = retrogradeAnalysisBaseImpl<flattenedSz, nonPlacementDatatype, N, rowSz, 
    colSz, decltype(fwdMoveGenerator), decltype(revMoveGenerator)>(::std::move(checkmates),
    forward, reverse);

// std::cout << wins.size() << " " << losses.size() << std::endl;
#endif
#else
std::cerr << "ERROR: Invalid configuration file" << std::endl;
assert(false);
#endif
}
