#include "retrograde_analysis.hpp"
#include <cassert>
#ifdef PATH_TO_USER_HEADER
#  include PATH_TO_USER_HEADER
#endif


int main(){
#if defined ROWSZ && defined COLSZ && defined FLATTENEDSZ
//invoke retrograde analysis and checkmate generator with paramaters passed
retrograde_analysis(FLATTENEDSZ, ROWSZ, COLSZ);
#else
assert(false);
#endif
}