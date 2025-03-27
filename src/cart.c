#include "cart.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

static const char* cart_type_str(const struct cartridge *cart)
{
    switch (cart->header.cart_type)
    {
        case 0x00: return "ROM ONLY";
        case 0x01: return "MBC1";
        case 0x02: return "MBC1+RAM";
        case 0x03: return "MBC1+RAM+BATTERY";
        case 0x05: return "MBC2";
        case 0x06: return "MBC2+BATTERY";
        case 0x08: return "ROM+RAM";
        case 0x09: return "ROM+RAM+BATTERY";
        case 0x0B: return "MMM01";
        case 0x0C: return "MMM01+RAM";
        case 0x0D: return "MMM01+RAM+BATTERY";
        case 0x0F: return "MBC3+TIMER+BATTERY";
        case 0x10: return "MBC3+TIMER+RAM+BATTERY";
        case 0x11: return "MBC3";
        case 0x12: return "MBC3+RAM";
        case 0x13: return "MBC3+RAM+BATTERY";
        case 0x19: return "MBC5";
        case 0x1A: return "MBC5+RAM";
        case 0x1B: return "MBC5+RAM+BATTERY";
        case 0x1C: return "MBC5+RUMBLE";
        case 0x1D: return "MBC5+RUMBLE+RAM";
        case 0x1E: return "MBC5+RUMBLE+RAM+BATTERY";
        case 0x20: return "MBC6";
        case 0x22: return "MBC7+SENSOR+RUMBLE+RAM+BATTERY";
        case 0xFC: return "POCKET CAMERA";
        case 0xFD: return "BANDAI TAMA5";
        case 0xFE: return "HuC3";
        case 0xFF: return "HuC1+RAM+BATTERY";

        default: return "UNKNOWN";
    }
}

static u8 cart_nb_ram_banks(const struct cartridge *cart)
{
    switch (cart->header.ram_size)
    {
        case 0x02: return 1;
        case 0x03: return 4;
        case 0x04: return 16;
        case 0x05: return 8;

        default: return 0;
    }
}

static const char* cart_new_licensee_str(const struct cartridge *cart)
{
    const char *code = cart->header.new_licensee_code;

#define SAME(c,t) if (memcmp(c, t, 2) == 0)
    SAME(code, "00") return "None";
    SAME(code, "01") return "Nintendo R&D1";
    SAME(code, "08") return "Capcom";
    SAME(code, "13") return "EA (Electronic Arts)";
    SAME(code, "18") return "Hudson Soft";
    SAME(code, "19") return "B-AI";
    SAME(code, "20") return "KSS";
    SAME(code, "22") return "Planning Office WADA";
    SAME(code, "24") return "PCM Complete";
    SAME(code, "25") return "San-X";
    SAME(code, "28") return "Kemco";
    SAME(code, "29") return "SETA Corporation";
    SAME(code, "30") return "Viacom";
    SAME(code, "31") return "Nintendo";
    SAME(code, "32") return "Bandai";
    SAME(code, "33") return "Ocean Software/Acclaim Entertainment";
    SAME(code, "34") return "Konami";
    SAME(code, "35") return "HectorSoft";
    SAME(code, "37") return "Taito";
    SAME(code, "38") return "Hudson Soft";
    SAME(code, "39") return "Banpresto";
    SAME(code, "41") return "Ubi Soft";
    SAME(code, "42") return "Atlus";
    SAME(code, "44") return "Malibu Interactive";
    SAME(code, "46") return "Angel";
    SAME(code, "47") return "Bullet-Proof Software";
    SAME(code, "49") return "Irem";
    SAME(code, "50") return "Absolute";
    SAME(code, "51") return "Acclaim Entertainment";
    SAME(code, "52") return "Activision";
    SAME(code, "53") return "Sammy USA Corporation";
    SAME(code, "54") return "Konami";
    SAME(code, "55") return "Hi Tech Expressions";
    SAME(code, "56") return "LJN";
    SAME(code, "57") return "Matchbox";
    SAME(code, "58") return "Mattel";
    SAME(code, "59") return "Milton Bradley Company";
    SAME(code, "60") return "Titus Interactive";
    SAME(code, "61") return "Virgin Games Ltd.";
    SAME(code, "64") return "Lucasfilm Games";
    SAME(code, "67") return "Ocean Software";
    SAME(code, "69") return "EA (Electronic Arts)";
    SAME(code, "70") return "Infogrames";
    SAME(code, "71") return "Interplay Entertainment";
    SAME(code, "72") return "Broderbund";
    SAME(code, "73") return "Sculptured Software";
    SAME(code, "75") return "The Sales Curve Limited";
    SAME(code, "78") return "THQ";
    SAME(code, "79") return "Accolade";
    SAME(code, "80") return "Misawa Entertainment";
    SAME(code, "83") return "lozc";
    SAME(code, "86") return "Tokuma Shoten";
    SAME(code, "87") return "Tsukuda Original";
    SAME(code, "91") return "Chunsoft Co.";
    SAME(code, "92") return "Video System";
    SAME(code, "93") return "Ocean Software/Acclaim Entertainment";
    SAME(code, "95") return "Varie";
    SAME(code, "96") return "Yonezawa/s’pal";
    SAME(code, "97") return "Kaneko";
    SAME(code, "99") return "Pack-In-Video";
    SAME(code, "9H") return "Bottom Up";
    SAME(code, "A4") return "Konami (Yu-Gi-Oh!)";
    SAME(code, "BL") return "MTO";
    SAME(code, "DK") return "Kodansha";
#undef SAME

    return "UNKNOWN";
}

static const char* cart_licensee_str(const struct cartridge *cart)
{
    switch (cart->header.old_licensee_code)
    {
        case 0x00: return "None";
        case 0x01: return "Nintendo";
        case 0x08: return "Capcom";
        case 0x09: return "HOT-B";
        case 0x0A: return "Jaleco";
        case 0x0B: return "Coconuts Japan";
        case 0x0C: return "Elite Systems";
        case 0x13: return "EA (Electronic Arts)";
        case 0x18: return "Hudson Soft";
        case 0x19: return "ITC Entertainment";
        case 0x1A: return "Yanoman";
        case 0x1D: return "Japan Clary";
        case 0x1F: return "Virgin Games Ltd.";
        case 0x24: return "PCM Complete";
        case 0x25: return "San-X";
        case 0x28: return "Kemco";
        case 0x29: return "SETA Corporation";
        case 0x30: return "Infogrames";
        case 0x31: return "Nintendo";
        case 0x32: return "Bandai";
        case 0x33: return cart_new_licensee_str(cart);
        case 0x34: return "Konami";
        case 0x35: return "HectorSoft";
        case 0x38: return "Capcom";
        case 0x39: return "Banpresto";
        case 0x3C: return "Entertainment Interactive (stub)";
        case 0x3E: return "Gremlin";
        case 0x41: return "Ubi Soft";
        case 0x42: return "Atlus";
        case 0x44: return "Malibu Interactive";
        case 0x46: return "Angel";
        case 0x47: return "Spectrum HoloByte";
        case 0x49: return "Irem";
        case 0x4A: return "Virgin Games Ltd.";
        case 0x4D: return "Malibu Interactive";
        case 0x4F: return "U.S. Gold";
        case 0x50: return "Absolute";
        case 0x51: return "Acclaim Entertainment";
        case 0x52: return "Activision";
        case 0x53: return "Sammy USA Corporation";
        case 0x54: return "GameTek";
        case 0x55: return "Park Place";
        case 0x56: return "LJN";
        case 0x57: return "Matchbox";
        case 0x59: return "Milton Bradley Company";
        case 0x5A: return "Mindscape";
        case 0x5B: return "Romstar";
        case 0x5C: return "Naxat Soft";
        case 0x5D: return "Tradewest";
        case 0x60: return "Titus Interactive";
        case 0x61: return "Virgin Games Ltd.";
        case 0x67: return "Ocean Software";
        case 0x69: return "EA (Electronic Arts)";
        case 0x6E: return "Elite Systems";
        case 0x6F: return "Electro Brain";
        case 0x70: return "Infogrames";
        case 0x71: return "Interplay Entertainment";
        case 0x72: return "Broderbund";
        case 0x73: return "Sculptured Software";
        case 0x75: return "The Sales Curve Limited";
        case 0x78: return "THQ";
        case 0x79: return "Accolade";
        case 0x7A: return "Triffix Entertainment";
        case 0x7C: return "MicroProse";
        case 0x7F: return "Kemco";
        case 0x80: return "Misawa Entertainment";
        case 0x83: return "LOZC G.";
        case 0x86: return "Tokuma Shoten";
        case 0x8B: return "Bullet-Proof Software";
        case 0x8C: return "Vic Tokai Corp.";
        case 0x8E: return "Ape Inc.";
        case 0x8F: return "I’Max";
        case 0x91: return "Chunsoft Co.";
        case 0x92: return "Video System";
        case 0x93: return "Tsubaraya Productions";
        case 0x95: return "Varie";
        case 0x96: return "Yonezawa/S’Pal";
        case 0x97: return "Kemco";
        case 0x99: return "Arc";
        case 0x9A: return "Nihon Bussan";
        case 0x9B: return "Tecmo";
        case 0x9C: return "Imagineer";
        case 0x9D: return "Banpresto";
        case 0x9F: return "Nova";
        case 0xA1: return "Hori Electric";
        case 0xA2: return "Bandai";
        case 0xA4: return "Konami";
        case 0xA6: return "Kawada";
        case 0xA7: return "Takara";
        case 0xA9: return "Technos Japan";
        case 0xAA: return "Broderbund";
        case 0xAC: return "Toei Animation";
        case 0xAD: return "Toho";
        case 0xAF: return "Namco";
        case 0xB0: return "Acclaim Entertainment";
        case 0xB1: return "ASCII Corporation or Nexsoft";
        case 0xB2: return "Bandai";
        case 0xB4: return "Square Enix";
        case 0xB6: return "HAL Laboratory";
        case 0xB7: return "SNK";
        case 0xB9: return "Pony Canyon";
        case 0xBA: return "Culture Brain";
        case 0xBB: return "Sunsoft";
        case 0xBD: return "Sony Imagesoft";
        case 0xBF: return "Sammy Corporation";
        case 0xC0: return "Taito";
        case 0xC2: return "Kemco";
        case 0xC3: return "Square";
        case 0xC4: return "Tokuma Shoten";
        case 0xC5: return "Data East";
        case 0xC6: return "Tonkin House";
        case 0xC8: return "Koei";
        case 0xC9: return "UFL";
        case 0xCA: return "Ultra Games";
        case 0xCB: return "VAP, Inc.";
        case 0xCC: return "Use Corporation";
        case 0xCD: return "Meldac";
        case 0xCE: return "Pony Canyon";
        case 0xCF: return "Angel";
        case 0xD0: return "Taito";
        case 0xD1: return "SOFEL (Software Engineering Lab)";
        case 0xD2: return "Quest";
        case 0xD3: return "Sigma Enterprises";
        case 0xD4: return "ASK Kodansha Co.";
        case 0xD6: return "Naxat Soft";
        case 0xD7: return "Copya System";
        case 0xD9: return "Banpresto";
        case 0xDA: return "Tomy";
        case 0xDB: return "LJN";
        case 0xDD: return "Nippon Computer Systems";
        case 0xDE: return "Human Ent.";
        case 0xDF: return "Altron";
        case 0xE0: return "Jaleco";
        case 0xE1: return "Towa Chiki";
        case 0xE2: return "Yutaka";
        case 0xE3: return "Varie";
        case 0xE5: return "Epoch";
        case 0xE7: return "Athena";
        case 0xE8: return "Asmik Ace Entertainment";
        case 0xE9: return "Natsume";
        case 0xEA: return "King Records";
        case 0xEB: return "Atlus";
        case 0xEC: return "Epic/Sony Records";
        case 0xEE: return "IGS";
        case 0xF0: return "A Wave";
        case 0xF3: return "Extreme Entertainment";
        case 0xFF: return "LJN";

        default: return "UNKNOWN";
    }
}

bool cart_load(struct cartridge *cart, const char *filename)
{
    FILE* file;
    long size;
    u8 checksum, *header_data;
    struct rom_header *h;
    bool result = true;

    h = &(cart->header);

    file = fopen(filename, "rb");
    if (file == NULL)
    {
        perror("Can't open ROM file");
        return false;
    }

    if (fseek(file, 0L, SEEK_END) != 0)
    {
        perror("fseek");
        DEFER(false);
    }

    size = ftell(file);
    if (size < 0)
    {
        perror("ftell");
        DEFER(false);
    }
    rewind(file);

    cart->size = size - sizeof(*h);
    cart->data = (u8 *)malloc(cart->size);
    if (cart->data == NULL)
    {
        perror("Failed to create ROM buffer");
        DEFER(false);
    }

    if (fread(h, sizeof(*h), 1, file) == 0)
    {
        perror("fread header");
        DEFER(false);
    }

    if (fread(cart->data, cart->size, 1, file) == 0)
    {
        perror("fread data");
        DEFER(false);
    }
    fclose(file);
    file = NULL;

    checksum = 0;
    header_data = (u8*)h;
    for (u16 address = 0x0134; address <= 0x014C; address++)
    {
        checksum = checksum - header_data[address] - 1;
    }
    if (checksum != cart->header.checksum)
    {
        fprintf(stderr, "Checksum verification failed!\n");
        DEFER(false);
    }

    printf("ROM loaded:\n");
    printf("\tTitle:         %.*s\n", (int)sizeof(h->title), h->title);
    printf("\tType:          0x%.2X (%s)\n", h->cart_type, cart_type_str(cart));
    printf("\tROM size:      %4u KB\n", (32 << h->rom_size));
    printf("\tRAM size:      %4u KB\n", cart_nb_ram_banks(cart) * 8);
    printf("\tLicensee code: 0x%.2X (%s)\n", h->old_licensee_code, cart_licensee_str(cart));
    printf("\tROM version:   %4d\n", h->game_version);

defer:
    if (file != NULL)
        fclose(file);
    return result;
}

void cart_unload(struct cartridge *cart)
{
    if (cart->data != NULL)
        free(cart->data);

    memset(cart, 0, sizeof(*cart));
}

u8 cart_read(struct cartridge *cart, u16 address)
{
    (void)cart;
    (void)address;
    NOT_IMPL;
    return 0;
}

void cart_write(struct cartridge *cart, u16 address, u8 value)
{
    (void)cart;
    (void)address;
    (void)value;
    NOT_IMPL;
}
