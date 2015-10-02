#ifndef SMN8_ROM_H
#define SMN8_ROM_H

#include "smn8/sys.h"

#define SMN8_ROM_ADDR SMN8_RAM_RESERVED_SIZE
#define SMN8_ROM_SIZE SMN8_RAM_SIZE - SMN8_ROM_ADDR

typedef struct
{
	unsigned short size;
	unsigned char bytes[SMN8_ROM_SIZE];
} smn8_rom;

int smn8_rom_load(smn8_rom *rom, FILE *fp);
void smn8_rom_disasm(const smn8_rom *rom, FILE *fp);
void smn8_rom_disasm_blob(const unsigned char *bytes, 
						  const unsigned short start,
						  const unsigned short end,
						  const unsigned short offset,
						  FILE *fp);

#endif
