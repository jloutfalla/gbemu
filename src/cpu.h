#ifndef _CPU_H_
#define _CPU_H_

#include "types.h"

struct LR35902
{
    union {
        u16 AF;
        struct
        {
            u8 A;
            union {
                u8 F;
                struct
                {
                    u8 z : 1;
                    u8 n : 1;
                    u8 h : 1;
                    u8 c : 1;
                    u8 _ : 4;
                };
            };
        };
    };

    union {
        u16 BC;
        struct
        {
            u8 B;
            u8 C;
        };
    };

    union {
        u16 DE;
        struct
        {
            u8 D;
            u8 E;
        };
    };

    union {
        u16 HL;
        struct
        {
            u8 H;
            u8 L;
        };
    };

    u16 SP;
    u16 PC;
};

#endif /* !_CPU_H_ */
