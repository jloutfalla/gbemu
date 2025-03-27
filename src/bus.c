#include "bus.h"

#include "emu.h"
#include "utils.h"

/*
 * 0000 - 3FFF | 16 KiB ROM bank 00             | From cartridge, usually a fixed bank
 * 4000 - 7FFF | 16 KiB ROM Bank 01–NN          | From cartridge, switchable bank via mapper (if any)
 * 8000 - 9FFF | 8 KiB Video RAM (VRAM)         | In CGB mode, switchable bank 0/1
 * A000 - BFFF | 8 KiB External RAM             | From cartridge, switchable bank if any
 * C000 - CFFF | 4 KiB Work RAM (WRAM)          |
 * D000 - DFFF | 4 KiB Work RAM (WRAM)          | In CGB mode, switchable bank 1–7
 * E000 - FDFF | Echo RAM (mirror of C000–DDFF) | Nintendo says use of this area is prohibited.
 * FE00 - FE9F | Object attribute memory (OAM)  |
 * FEA0 - FEFF | Not Usable                     | Nintendo says use of this area is prohibited.
 * FF00 - FF7F | I/O Registers                  |
 * FF80 - FFFE | High RAM (HRAM)                |
 * FFFF - FFFF | Interrupt Enable register (IE) |
 */

#define RESTRICTED_START    (0xFEA0)
#define RESTRICTED_END      (0xFEFF)

u8 bus_read(struct emulator *e, u16 address)
{
    if (address <= BANK1_END)
        return cart_read(&e->cart, address);

    if (RESTRICTED_START <= address && address <= RESTRICTED_END)
        return 0;

    assert(0 && "Not implemented");
    return 0;
}

void bus_write(struct emulator *e, u16 address, u8 value)
{
    if (address < 0x8000)
        cart_write(&e->cart, address, value);

    NOT_IMPL;
}
