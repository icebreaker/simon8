#include "smn8/vm.h"
#include "smn8/font.h"

#define smn8_vm_abort() \
	{ \
		SMN8_UNSUPPORTED_OPS(op1, op2); \
		vm->pc -= 2; \
	} while(0)

int smn8_vm_init(smn8_vm *vm, const smn8_rom *rom)
{
	smn8_memset(vm, 0x00, sizeof(*vm));

	smn8_memcpy(vm->ram + SMN8_FONT_ADDR, smn8_font, sizeof(smn8_font));
	smn8_memcpy(vm->ram + SMN8_ROM_ADDR, rom->bytes, rom->size);

	vm->pc = SMN8_ROM_ADDR;

	smn8_randomize();

	return 1;
}

void smn8_vm_tick_cycle(smn8_vm *vm)
{
	unsigned char op1 = vm->ram[vm->pc++];
	unsigned char op2 = vm->ram[vm->pc++];

	switch(((op1 >> 4) & 0xF))
	{
		case 0x0:
			{
				switch(op2)
				{
					case 0xE0:
						smn8_memset(vm->vga, 0x00, sizeof(vm->vga));
						break;

					case 0xEE:
						vm->pc = vm->stack[--vm->sp];
						break;

					default:
						smn8_vm_abort();
						break;
				}
			}
			break;

		case 0x1:
			vm->pc = ((op1 & 0xF) << 8) | op2;
			break;

		case 0x2:
			{
				vm->stack[vm->sp++] = vm->pc;
				vm->pc = ((op1 & 0xF) << 8) | op2;
			}
			break;

		case 0x3:
			vm->pc += vm->regs[op1 & 0xF] == op2 ? 2 : 0;
			break;

		case 0x4:
			vm->pc += vm->regs[op1 & 0xF] != op2 ? 2 : 0;
			break;

		case 0x5:
			vm->pc += vm->regs[op1 & 0xF] == vm->regs[((op2 >> 4) & 0xF)] ? 2 : 0;
			break;

		case 0x6:
			vm->regs[op1 & 0xF] = op2;
			break;

		case 0x7:
			vm->regs[op1 & 0xF] += op2;
			break;

		case 0x8:
			{
				switch(op2 & 0xF)
				{
					case 0x0:
						vm->regs[op1 & 0xF] = vm->regs[((op2 >> 4) & 0xF)];
						break;

					case 0x1:
						vm->regs[op1 & 0xF] |= vm->regs[((op2 >> 4) & 0xF)];
						break;

					case 0x2:
						vm->regs[op1 & 0xF] &= vm->regs[((op2 >> 4) & 0xF)];
						break;

					case 0x3:
						vm->regs[op1 & 0xF] ^= vm->regs[((op2 >> 4) & 0xF)];
						break;

					case 0x4:
						{
							unsigned short x = vm->regs[op1 & 0xF] + vm->regs[((op2 >> 4) & 0xF)];
							vm->regs[0xF] = (x > 0xFF) ? 1 : 0;
							vm->regs[op1 & 0xF] = x & 0xFF;
						}
						break;

					case 0x5:
						{
							short x = vm->regs[op1 & 0xF];
							short y = vm->regs[((op2 >> 4) & 0xF)];
							vm->regs[0xF] = (x > y) ? 1 : 0;
							vm->regs[op1 & 0xF] = x - y;
						}
						break;

					case 0x6:
						{
							vm->regs[0xF] = ((vm->regs[op1 & 0xF]) & 0x1);
							vm->regs[op1 & 0xF] >>= 1;
						}
						break;

					case 0x7:
						{
							short x = vm->regs[op1 & 0xF];
							short y = vm->regs[((op2 >> 4) & 0xF)];
							vm->regs[0xF] = (y > x) ? 1 : 0;
							vm->regs[op1 & 0xF] = y - x;
						}
						break;

					case 0xE:
						{
							vm->regs[0xF] = ((vm->regs[op1 & 0xF] >> 7) & 0x1);
							vm->regs[op1 & 0xF] <<= 1;
						}
						break;

					default:
						smn8_vm_abort();
						break;
				}
			}
			break;

		case 0x9:
			vm->pc += vm->regs[op1 & 0xF] != vm->regs[((op2 >> 4) & 0xF)] ? 2 : 0;
			break;

		case 0xA:
			vm->i = ((op1 & 0xF) << 8) | op2;
			break;

		case 0xB:
			vm->pc = (((op1 & 0xF) << 8) | op2) + vm->regs[0x0];
			break;

		case 0xC:
			vm->regs[op1 & 0xF] = smn8_random() & op2;
			break;

		case 0xD:
			{
				unsigned char x = vm->regs[op1 & 0xF];
				unsigned char y = vm->regs[(op2 >> 4) & 0xF];
				unsigned char n = op2 & 0xF;
				unsigned char vf = 0;
				char xx, yy;

				for(yy = 0; yy < n; yy++, y++)
				{
					unsigned char pixel = vm->ram[vm->i + yy];
					unsigned short oy = ((y & SMN8_VGA_HEIGHT_MASK) << SMN8_VGA_WIDTH_SHL);

					for(xx = 0; xx < 8; xx++)
					{
						unsigned char color = (pixel >> (7 - xx)) & 1;
						unsigned short index = ((x + xx) & SMN8_VGA_WIDTH_MASK) + oy;

						unsigned char old_color = vm->vga[index];
						unsigned char new_color = old_color ^ color;

						vf |= (old_color == 1 && new_color == 0) ? 1 : 0;

						vm->vga[index] = new_color;
					}
				}

				vm->regs[0xF] = vf;
			}
			break;

		case 0xE:
			{
				switch(op2)
				{
					case 0x9E:
						vm->pc += vm->keys[vm->regs[op1 & 0xF]] == SMN8_VM_KEY_DOWN ? 2 : 0; 
						break;

					case 0xA1:
						vm->pc += vm->keys[vm->regs[op1 & 0xF]] == SMN8_VM_KEY_UP ? 2 : 0; 
						break;

					default:
						smn8_vm_abort();
						break;
				}
			}
			break;

		case 0xF:
			{
				switch(op2)
				{
					case 0x07:
						vm->regs[op1 & 0xF] = vm->dt;
						break;

					case 0x0A:
						{
							unsigned char i;
							char key = -1;

							for(i = 0; i < SMN8_VM_KEY_MAX; i++)
							{
								if(vm->keys[i] == SMN8_VM_KEY_DOWN)
								{
									key = i;
									break;
								}
							}

							key == -1 ? (vm->pc -= 2) : (vm->regs[op1 & 0xF] = (unsigned char) key);
						}
						break;

					case 0x15:
						vm->dt = vm->regs[op1 & 0xF];
						break;

					case 0x18:
						vm->st = vm->regs[op1 & 0xF];
						break;

					case 0x1E:
						vm->i += vm->regs[op1 & 0xF];
						break;

					case 0x29:
						vm->i = vm->regs[op1 & 0xF] * SMN8_FONT_OFFSET;
						break;

					case 0x33:
						{
							short x = vm->regs[op1 & 0xF];
							vm->ram[vm->i + 0] = (x / 100);
							vm->ram[vm->i + 1] = (x % 100) / 10;
							vm->ram[vm->i + 2] = (x % 10);
						}
						break;

					case 0x55:
						{
							unsigned char x = op1 & 0xF;
							unsigned char i;

							for(i = 0; i <= x; i++)
								vm->ram[vm->i + i] = vm->regs[i];
						}
						break;

					case 0x65:
						{
							unsigned char x = op1 & 0xF;
							unsigned char i;

							for(i = 0; i <= x; i++)
								vm->regs[i] = vm->ram[vm->i + i];
						}
						break;

					default:
						smn8_vm_abort();
						break;
				}
			}
			break;

		default:
			smn8_vm_abort();
			break;
	}
}

void smn8_vm_tick_timer(smn8_vm *vm)
{
	if(vm->dt > 0)
		vm->dt--;

	if(vm->st > 0)
		vm->st--;
}

void smn8_vm_clear(smn8_vm *vm)
{
	SMN8_UNUSED(vm);
}
