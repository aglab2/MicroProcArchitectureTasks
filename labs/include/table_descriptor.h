#ifndef GDT_DESCRIPTOR_H
#define GDT_DESCRIPTOR_H

#include "stddef.h"
#include "selector.h"

#pragma pack(push, 1)
typedef struct {
	uint16 limit;
	uint32 base;
	uint16 padding;
} GDT_descriptor;

typedef struct {
	uint16 limit;
	uint32 base;
	uint16 padding;
} IDT_descriptor;

typedef s_selector LDT_descriptor;
typedef s_selector TS_descriptor;

#pragma pack(pop)


#endif
