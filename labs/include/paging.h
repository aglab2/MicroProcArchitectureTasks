#ifndef PAGING_H_
#define PAGING_H_

#include "stddef.h"

#pragma pack(push, 1)
typedef union _PTE {
	struct {
       	uint32 P:1;
       	uint32 RW:1;
       	uint32 US:1;
       	uint32 PWT:1;
       	uint32 PCD:1;
       	uint32 A:1;
       	uint32 D:1;
       	uint32 PAT_PS:1;
       	uint32 G:1;
       	uint32 OS:3;
		
       	uint32 pfn:20;
    	};
    	uint32 raw;
} pagetable_entry;
#pragma pack(pop)

#endif
