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

#endif