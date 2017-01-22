#include "stdio.h"
#include "stdlib.h"

#include "include/table_descriptor.h"
#include "include/entry_descriptor.h"
#include "include/paging.h"

#define DB_NUM 3

uint32 old_handler;
uint32 old_selector;

char *HURRAY = "We are debugging here!\n$";

void __declspec(naked) db_handler(void)
{
	__asm {
        	pushfd
        	push eax
		push edx

		mov ah, 09h
		mov edx, HURRAY
		int 21h

		pop edx
		pop eax
		popfd

		push old_selector
		push old_handler
		retf
    	}
}

int main(){
    	IDT_descriptor _idt;
    	uint16 _cs;
    	g_descriptor* db_entry;

	uint32 new_handler = (uint32) &db_handler;

    	__asm {
        	sidt _idt
        	mov ax, cs
        	mov _cs, ax
    	}

    	db_entry = &(((g_descriptor *)_idt.base)[DB_NUM]);

    	old_handler = (db_entry->offset16 << 16) | (db_entry->offset0);
    	old_selector = db_entry->selector;
    	db_entry->offset16 = (uint16)(new_handler >> 16);
    	db_entry->offset0 = (uint16)(new_handler & 0xFFFF);
	db_entry->selector = _cs;

    	printf("IDT: 0x%08x\n", _idt.base);
    	printf("old_handler: 0x%04X:0x%08x\n", old_selector, old_handler);
    	printf("my_handler 0x%04X:0x%08x\n", _cs, new_handler);

	__asm{
		int 3h
		byte(0xCC)
	}
}
