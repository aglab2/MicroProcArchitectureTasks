#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include "stddef.h"

#pragma pack(push, 1)
typedef struct{
	uint16 limit0;
	uint16 base0;

	uint8 base16;

	unsigned char type: 4;
	unsigned char S: 1;
	unsigned char DPL: 2;
	unsigned char P: 1;

	unsigned char limit16: 4;
	unsigned char AVL: 1;
	unsigned char L: 1;
	unsigned char G: 1;
	unsigned char DB: 1;

	uint8 base24: 8;
} s_descriptor;

typedef struct{
	uint16 offset0;
	uint16 selector;

	unsigned char reserved1: 5;
	unsigned char zeros_reserved2: 3;

	unsigned char type: 3;
	unsigned char D: 1;
	unsigned char bit0: 1;
	unsigned char DPL: 2;
	unsigned char P: 1;

	uint16 offset16;
} g_descriptor;

#pragma pack(pop)

#endif
