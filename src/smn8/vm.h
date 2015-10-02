#ifndef SMN8_VM_H
#define SMN8_VM_H

#include "smn8/rom.h"

#define SMN8_REGS_MAX	0xF + 0x1
#define SMN8_KEYS_MAX	0xF + 0x1
#define SMN8_STACK_MAX	0xF + 0x1

#define SMN8_VGA_WIDTH 64
#define SMN8_VGA_WIDTH_MASK 63
#define SMN8_VGA_WIDTH_SHL 6

#define SMN8_VGA_HEIGHT 32
#define SMN8_VGA_HEIGHT_MASK 31

#define SMN8_VGA_SIZE SMN8_VGA_WIDTH * SMN8_VGA_HEIGHT

#define SMN8_TIMER_HZ_IN_MS 0xF + 0x1
#define SMN8_VM_CYCLES 0x9

typedef struct
{
	unsigned char regs[SMN8_REGS_MAX];
	unsigned char keys[SMN8_KEYS_MAX + 0x1];
	unsigned char ram[SMN8_RAM_SIZE];
	unsigned char vga[SMN8_VGA_SIZE];
	unsigned short stack[SMN8_STACK_MAX];
	unsigned short i;
	unsigned short pc;
	unsigned char dt;
	unsigned char st;
	unsigned char sp;
} smn8_vm;

typedef enum
{
	SMN8_VM_KEY_0 = 0,
	SMN8_VM_KEY_1,
	SMN8_VM_KEY_2,
	SMN8_VM_KEY_3,
	SMN8_VM_KEY_4,
	SMN8_VM_KEY_5,
	SMN8_VM_KEY_6,
	SMN8_VM_KEY_7,
	SMN8_VM_KEY_8,
	SMN8_VM_KEY_9,
	SMN8_VM_KEY_A,
	SMN8_VM_KEY_B,
	SMN8_VM_KEY_C,
	SMN8_VM_KEY_D,
	SMN8_VM_KEY_E,
	SMN8_VM_KEY_F,
	SMN8_VM_KEY_MAX
} smn8_vm_key;

typedef enum
{
	SMN8_VM_KEY_UP = 0,
	SMN8_VM_KEY_DOWN
} smn8_vm_key_state;

#define smn8_vm_set_key(vm, key, state) vm->keys[key] = state
#define smn8_vm_get_pixel(vm, x, y) vm->vga[x + (y << SMN8_VGA_WIDTH_SHL)]
#define smn8_vm_get_pixel_offset(vm, y) (y << SMN8_VGA_WIDTH_SHL)
#define smn8_vm_get_pixel_with_offset(vm, x, offset) vm->vga[x + offset]
#define smn8_vm_get_sound_timer(vm) vm->st
#define smn8_vm_get_register(vm, r) vm->regs[r]
#define smn8_vm_get_pc(vm) vm->pc
#define smn8_vm_get_ram(vm) vm->ram
#define smn8_vm_get_vga(vm) vm->vga
#define smn8_vm_vga_cpy(vm, vga) smn8_memcpy(vga, vm->vga, sizeof(vm->vga))

int smn8_vm_init(smn8_vm *vm, const smn8_rom *rom);
void smn8_vm_tick_cycle(smn8_vm *vm);
void smn8_vm_tick_timer(smn8_vm *vm);
void smn8_vm_clear(smn8_vm *vm);

#endif
