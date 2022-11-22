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

int main(){
#if defined rowSz && defined colSz
constexpr std::size_t flattenedSz = rowSz * colSz;
forwardMoveGenerator<flattenedSz, nonPlacementDatatype> forward;
reverseMoveGenerator<flattenedSz, nonPlacementDatatype> reverse;

hzSymEvaluator hzSymmetryCheck;
vtSymEvaluator vtSymmetryCheck;

//invoke retrograde analysis and checkmate generator with paramaters passed
#ifndef MULTI_NODE
auto checkmates = generateAllCheckmates<flattenedSz, nonPlacementDatatype, N, rowSz, colSz, winCondEvaluator, 
  hzSymEvaluator, vtSymEvaluator, isValidBoardFn>(noRoyaltyPieceset, royaltyPieceset, winCondEvaluator eval, 
  hzSymEvaluator hzSymFn={}, vtSymEvaluator vSymFn={}, isValidBoardFn isValidBoardFn={}); 

retrograde_analysis<MachineType::SINGLE_NODE, flattenedSz, nonPlacementDatatype, N, 
  rowSz, colSz, decltype(forward), decltype(reverse), hzSymEvaluator, vtSymEvaluator, 
  isValidBoardFn>(Args&&... args);
#else
retrograde_analysis<MachineType::MULTI_NODE, flattenedSz, nonPlacementDatatype, N, 
  rowSz, colSz, decltype(forward), decltype(reverse), hzSymEvaluator, vtSymEvaluator, 
  isValidBoardFn>(Args&&... args);
#endif
#else
std::cerr << "ERROR: Invalid configuration file" << std::endl;
assert(false);
#endif
}
