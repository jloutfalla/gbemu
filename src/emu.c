#include "emu.h"

#include <string.h>
#include "utils.h"

bool emu_run(struct emulator *emu, const char *filename)
{
    bool result = true;

    if (cart_load(&emu->cart, filename) == false)
        DEFER(false);

defer:
    emu_clean(emu);
    return result;
}

void emu_clean(struct emulator *emu)
{
    cart_unload(&emu->cart);
    memset(emu, 0, sizeof(*emu));
}
