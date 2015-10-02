#ifndef SMN8_SYS_H
#define SMN8_SYS_H

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

#define SMN8_RAM_ADDR			0x000
#define SMN8_RAM_SIZE			0xFFF + 0x001
#define SMN8_RAM_RESERVED_SIZE	0x200

#define SMN8_UNUSED(x) (void)(x)
#define SMN8_ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define SMN8_STUB(x) fprintf(stderr, "stub('%s')\n", #x)
#define SMN8_UNSUPPORTED_OPS(op1, op2) fprintf(stderr, "unsupported ops('%02x%02x')\n", op1, op2);

#define smn8_memcpy memcpy
#define smn8_memset memset
#define smn8_randomize() srand(time(NULL))
#define smn8_random() (rand() & 0xFF)

#endif
