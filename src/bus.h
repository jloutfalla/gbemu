#ifndef _BUS_H_
#define _BUS_H_

#include "types.h"

struct emulator;

#define VRAM_START   (0x8000)
#define VRAM_END     (0x9FFF)

#define EXTRAM_START (0xA000)
#define EXTRAM_END   (0xBFFF)

#define WRAM_START   (0xC000)
#define WRAM_END     (0xDFFF)

#define ERAM_START   (0xE000)
#define ERAM_END     (0xFDFF)

#define OAM_START    (0xFE00)
#define OAM_END      (0xFE9F)

#define IO_START     (0xFF00)
#define IO_END       (0xFF7F)

#define HRAM_START   (0xFF80)
#define HRAM_END     (0xFFFE)

#define IE_REG       (0xFFFF)

u8 bus_read(struct emulator *e, u16 address);
void bus_write(struct emulator *e, u16 address, u8 value);

#endif /* !_BUS_H_ */
