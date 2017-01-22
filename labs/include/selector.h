#ifndef SELECTOR_H
#define SELECTOR_H

#pragma pack(push, 1)
typedef struct {
	uint16 RPL: 2;
	uint16 TI: 1;
	uint16 index: 13;
} s_selector;

#pragma pack(pop)

#endif
