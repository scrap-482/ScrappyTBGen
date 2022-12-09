#ifndef _IGNORE_MACROS_H_
#define _IGNORE_MACROS_H_

// Define macros here so my IDE doesn't underline everything red, 
// even though these should never be called because these should be defined.
#ifndef ROW_SZ
    #error "ROW_SZ not specified!"
    #define ROW_SZ 8
#endif
#ifndef COL_SZ
    #error "COL_SZ not specified!"
    #define COL_SZ 8
#endif
#ifndef N_MAN
    #error "N_MAN not specified!"
    #define N_MAN 3
#endif
#ifndef NO_ROYALTY_PIECESET
    #error "NO_ROYALTY_PIECESET not specified!"
    #define NO_ROYALTY_PIECESET {'p', 'P', 'r', 'R', 'n', 'N', 'b', 'B', 'q', 'Q'};
#endif
#ifndef ROYALTY_PIECESET
    #error "ROYALTY_PIECESET not specified!"
    #define ROYALTY_PIECESET {'k', 'K'};
#endif

#endif