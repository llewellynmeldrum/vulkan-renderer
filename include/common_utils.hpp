#pragma once 
#define SIZE_BYTES(x)   (sizeof(x))
#define SIZE_BITS(x)    ((sizeof(x)) * (8))

#if defined(__cplusplus)
#include <climits>
    static_assert(CHAR_BIT == 8);
// We have to assume this so that the file can be included in SLANG
#endif



