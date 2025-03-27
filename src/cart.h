#ifndef _CART_H_
#define _CART_H_

#include "types.h"

#define BANK0_START  (0x0000)
#define BANK0_END    (0x3FFF)

#define BANK1_START  (0x4000)
#define BANK1_END    (0x7FFF)

struct rom_header
{
    u8 _unused[0x100];
    u8 entry[4];
    u8 logo[48];

    char title[16];
    char new_licensee_code[2];
    u8 sgb_flag;
    u8 cart_type;
    u8 rom_size;
    u8 ram_size;
    u8 region_code;
    u8 old_licensee_code;
    u8 game_version;
    u8 checksum;
    u16 global_checksum;
};

struct cartridge
{
    u8 *data;
    size_t size;
    struct rom_header header;
};

bool cart_load(struct cartridge *cart, const char *filename);
void cart_unload(struct cartridge *cart);

u8 cart_read(struct cartridge *cart, u16 address);
void cart_write(struct cartridge *cart, u16 address, u8 value);

#endif /* !_CART_H_ */
