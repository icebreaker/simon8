#include "smn8/rom.h"

int smn8_rom_load(smn8_rom *rom, FILE *fp)
{
	int c;

	if(fp == NULL)
		return 0;

	rom->size = 0;

	for(;;)
	{
		c = fgetc(fp);

		if(c == EOF)
			break;

		if(rom->size == SMN8_ROM_SIZE)
			return -1;

		rom->bytes[rom->size++] = (unsigned char) c;
	}

	return (rom->size > 0);
}

void smn8_rom_disasm(const smn8_rom *rom, FILE *fp)
{
	smn8_rom_disasm_blob(rom->bytes, 0, rom->size, SMN8_ROM_ADDR, fp);
}

void smn8_rom_disasm_blob(const unsigned char *bytes, 
						  const unsigned short start,
						  const unsigned short end,
						  const unsigned short offset,
						  FILE *fp)
{
	unsigned short pc;
	unsigned char op1, op2;

	for(pc = start; pc < end; /* ; */)
	{
		fprintf(fp, "0x%02X: ", offset + pc);

		op1 = bytes[pc++];
		op2 = bytes[pc++];

		switch(((op1 >> 4) & 0xF))
		{
			case 0x0:
			{
				switch(op2)
				{
					case 0xE0:
						fprintf(fp, "CLS");
						break;

					case 0xEE:
						fprintf(fp, "RET");
						break;

					default:
						fprintf(fp, "DW   #%02x%02x", op1, op2);
						break;
				}
			}
			break;

			case 0x1:
				fprintf(fp, "JP   0x%X%02X", op1 & 0xF, op2);
				break;

			case 0x2:
				fprintf(fp, "CALL 0x%X%02X", op1 & 0xF, op2);
				break;

			case 0x3:
				fprintf(fp, "SE   V%X, 0x%02X", op1 & 0xF, op2);
				break;

			case 0x4:
				fprintf(fp, "SNE  V%X, 0x%02X", op1 & 0xF, op2);
				break;

			case 0x5:
				fprintf(fp, "SE   V%X, V%X", op1 & 0xF, ((op2 >> 4) & 0xF));
				break;

			case 0x6:
				fprintf(fp, "LD   V%X, 0x%02X", op1 & 0xF, op2);
				break;

			case 0x7:
				fprintf(fp, "ADD  V%X, 0x%02X", op1 & 0xF, op2);
				break;

			case 0x8:
			{
				switch(op2 & 0xF)
				{
					case 0x0:
						fprintf(fp, "LD   V%X, V%X", op1 & 0xF, ((op2 >> 4) & 0xF));
						break;

					case 0x1:
						fprintf(fp, "OR   V%X, V%X", op1 & 0xF, ((op2 >> 4) & 0xF));
						break;

					case 0x2:
						fprintf(fp, "AND  V%X, V%X", op1 & 0xF, ((op2 >> 4) & 0xF));
						break;

					case 0x3:
						fprintf(fp, "XOR  V%X, V%X", op1 & 0xF, ((op2 >> 4) & 0xF));
						break;

					case 0x4:
						fprintf(fp, "ADD  V%X, V%X", op1 & 0xF, ((op2 >> 4) & 0xF));
						break;
						
					case 0x5:
						fprintf(fp, "SUB  V%X, V%X", op1 & 0xF, ((op2 >> 4) & 0xF));
						break;

					case 0x6:
						fprintf(fp, "SHR  V%X, 1", op1 & 0xF);
						break;

					case 0x7:
						fprintf(fp, "SUBN V%X, V%X", op1 & 0xF, ((op2 >> 4) & 0xF));
						break;

					case 0xE:
						fprintf(fp, "SHL  V%X, 1", op1 & 0xF);
						break;

					default:
						fprintf(fp, "DW   #%02x%02x", op1, op2);
						break;
				}
			}
			break;

			case 0x9:
				fprintf(fp, "SNE  V%X, V%X", op1 & 0xF, ((op2 >> 4) & 0xF));
				break;

			case 0xA:
				fprintf(fp, "LD    I, 0x%X%02X", op1 & 0xF, op2);
				break;

			case 0xB:
				fprintf(fp, "JP   V0, 0x%X%02X", op1 & 0xF, op2);
				break;
	
			case 0xC:
				fprintf(fp, "RND  V%X, 0x%02X", op1 & 0xF, op2);
				break;

			case 0xD:
				fprintf(fp, "DRW  V%X, V%X, 0x%02X", op1 & 0xF, ((op2 >> 4) & 0xF), op2 & 0xF);
				break;

			case 0xE:
			{
				switch(op2)
				{
					case 0x9E:
						fprintf(fp, "SKP  V%X", op1 & 0xF);
						break;

					case 0xA1:
						fprintf(fp, "SKNP V%X", op1 & 0xF);
						break;

					default:
						fprintf(fp, "DW   #%02x%02x", op1, op2);
						break;
				}
			}
			break;

			case 0xF:
			{
				switch(op2)
				{
					case 0x07:
						fprintf(fp, "LD   V%X, DT", op1 & 0xF);
						break;

					case 0x0A:
						fprintf(fp, "LD   V%X, K", op1 & 0xF);
						break;

					case 0x15:
						fprintf(fp, "LD   DT, V%X", op1 & 0xF);
						break;

					case 0x18:
						fprintf(fp, "LD   ST, V%X", op1 & 0xF);
						break;

					case 0x1E:
						fprintf(fp, "ADD   I, V%X", op1 & 0xF);
						break;

					case 0x29:
						fprintf(fp, "LD    F, V%X", op1 & 0xF);
						break;

					case 0x33:
						fprintf(fp, "LD    B, V%X", op1 & 0xF);
						break;

					case 0x55:
						fprintf(fp, "LD   [I], V%X", op1 & 0xF);
						break;

					case 0x65:
						fprintf(fp, "LD   V%X, [I]", op1 & 0xF);
						break;

					default:
						fprintf(fp, "DW   #%02x%02x", op1, op2);
						break;
				}
			}
			break;

			default:
				fprintf(fp, "DW   #%02x%02x", op1, op2);
				break;
		}

		fprintf(fp, "\n");
	}
}
