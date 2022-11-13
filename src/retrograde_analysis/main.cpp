#include "retrograde_analysis.hpp"
#include <cassert>
#ifdef PATH_TO_USER_HEADER
#  include PATH_TO_USER_HEADER
#endif
#ifndef HZ_SYM_CHECK
#  define HZ_SYM_CHECK false_fn
#endif
#ifndef VT_SYM_CHECK
#  define VT_SYM_CHECK false_fn
#endif
#ifndef IS_VALID_BOARD_FN
#  define IS_VALID_BOARD_FN null_type
#endif

int main(){
#if defined ROWSZ && defined COLSZ && defined FLATTENEDSZ
FORWARD_MOVE_GENERATOR<FLATTENEDSZ, NON_PLACEMENT_DATATYPE> forward;
REVERSE_MOVE_GENERATOR<FLATTENEDSZ, NON_PLACEMENT_DATATYPE> reverse;

HZ_SYM_CHECK hzSymmetryCheck;
VT_SYM_CHECK vtSymmetryCheck;

//invoke retrograde analysis and checkmate generator with paramaters passed
#ifndef MULTI_NODE_
auto checkmates = generateAllCheckmates<FLATTENEDSZ, NON_PLACEMENT_DATATYPE, N, ROWSZ, COLSZ, CheckmateEvalFn>(); 
retrograde_analysis<MachineType::SINGLE_NODE, FLATTENEDSZ, NON_PLACEMENT_DATATYPE, N, 
  ROWSZ, COLSZ, decltype(forward), decltype(reverse), HZ_SYM_CHECK, VT_SYM_CHECK, 
  IS_VALID_BOARD_FN>();
#else
retrograde_analysis<MachineType::MULTI_NODE, FLATTENEDSZ, NON_PLACEMENT_DATATYPE, N, 
  ROWSZ, COLSZ, decltype(forward), decltype(reverse), HZ_SYM_CHECK, VT_SYM_CHECK, 
  IS_VALID_BOARD_FN>();
#endif
#else
assert(false);
#endif
}