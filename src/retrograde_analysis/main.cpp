#include "retrograde_analysis.hpp"
#include <cassert>
#ifdef PATH_TO_USER_HEADER
#  include PATH_TO_USER_HEADER
#endif

int main(){
#if defined ROWSZ && defined COLSZ && defined FLATTENEDSZ
FORWARD_MOVE_GENERATOR<FLATTENEDSZ, NON_PLACEMENT_DATATYPE> forward;
REVERSE_MOVE_GENERATOR<FLATTENEDSZ, NON_PLACEMENT_DATATYPE> reverse;
HZ_SYM_CHECK hzSymmetryCheck;
VT_SYM_CHECK vtSymmetryCheck;
//invoke retrograde analysis and checkmate generator with paramaters passed
generateAllCheckmates<std::vector<piece_label_t>, std::vector<piece_label_t>>(); 
retrograde_analysis<FLATTENEDSZ, ROWSZ, COLSZ>();
#else
assert(false);
#endif
}