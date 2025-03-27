#ifndef _UTILS_H_
#define _UTILS_H_

#include <assert.h>

#include "types.h"

#define shift(argc, argv) (argc > 0 ? ((argc)--, *(argv)++) : NULL)

#define NOT_IMPL assert(0 && "Not implemented")

#define DEFER(value)      \
    do                    \
    {                     \
        result = (value); \
        goto defer;       \
    }                     \
    while (0)


#endif /* !_UTILS_H_ */
