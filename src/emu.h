#ifndef _EMU_H_
#define _EMU_H_

#include "cpu.h"
#include "cart.h"

struct emulator
{
    struct LR35902 cpu;
    struct cartridge cart;
};

bool emu_run(struct emulator *emu, const char *filename);
void emu_clean(struct emulator *emu);

#endif /* !_EMU_H_ */
