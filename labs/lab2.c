#include "stdio.h"
#include "stdlib.h"

#include "include/table_descriptor.h"
#include "include/entry_descriptor.h"
#include "include/paging.h"

#define CR0_PG (1<<31) //0x80000000 -- Enable Paging
#define CR4_PSE (1<<4) //0x00000010 -- 4Mb page

#define PAGE_SIZE 0x1000
#define PF_NUM 14
#define BAD_PDI 0x222
#define BAD_ADDR 0x88812345 //0x222 << 22 + offset

//P=1, RW=1 (write), US=0 (kernel), 0,0, 0,0, 1,0, 000
//0000 1000 0011
#define PTE_KERNEL 0x083

uint32 old_handler;
uint32 old_selector;
uint32 bad_pde_virtual_address;

char *inthere = "We are in there!$";

void __declspec(naked) pf_handler(void)
{
	__asm {
        	pushfd
        	push eax
		push edx

		mov ah, 09h
		mov edx, inthere
		int 21h

        	mov eax, cr2
        	cmp eax, BAD_ADDR
        	jz my

        	//call original
		pop edx
        	pop eax
        	popfd
        	push old_selector
        	push old_handler
        	retf
        	//original will perform iretd himself

	my:
        	mov eax, bad_pde_virtual_address
        	or [eax], 1
        	//mov eax, cr3
        	//mov cr3, eax
        	invlpg [eax]



        	//[esp+14h] flags
        	//[esp+10h] cs
        	//[esp+0Ch] eip
        	//[esp+08h] error code
        	//[esp+04h] flags again
        	//[esp+00h] eax
        	//add [esp+0Ch], 6 //skip instruction that generate #PF

		pop edx
        	pop eax
        	popfd
        	add esp, 4
        	iretd
    	}
}

int main(){
    	uint32 _cr0 = 0;
    	uint32 _cr4 = 0;
    	IDT_descriptor _idt;
    	uint16 _cs;
    	g_descriptor* p_pf_entry_in_idt;
    	g_descriptor my_pf_entry;
    	pagetable_entry* pd;
    	int pdi;
	uint32 *p;

	uint32 my_handler = (uint32) &pf_handler;

    	__asm {
        	sidt _idt
        	mov ax, cs
        	mov _cs, ax
    	}

    	p_pf_entry_in_idt = &(((g_descriptor *)_idt.base)[PF_NUM]);

	//printf("14.1: 0x%08X'%08X \n", p_pf_entry_in_idt->raw1, p_pf_entry_in_idt->raw0);

    	old_handler = (p_pf_entry_in_idt->offset16 << 16) | (p_pf_entry_in_idt->offset0);
    	old_selector = p_pf_entry_in_idt->selector;
    	p_pf_entry_in_idt->offset16 = (uint16)(my_handler >> 16);
    	p_pf_entry_in_idt->offset0 = (uint16)(my_handler & 0xFFFF);
	p_pf_entry_in_idt->selector = _cs;

	//printf("14.2: 0x%08X'%08X \n", p_pf_entry_in_idt->raw1, p_pf_entry_in_idt->raw0);

    	printf("IDT: 0x%08x\n", _idt.base);
    	printf("old_handler: 0x%04X:0x%08x\n", old_selector, old_handler);
    	printf("my_handler 0x%04X:0x%08x\n", _cs, my_handler);

   	{
        	uint32 p = (uint32)malloc(2*PAGE_SIZE);
        	p = (p & (~0xFFF)) + PAGE_SIZE;
        	pd = (pagetable_entry *) p;
    	}

    	for (pdi = 0; pdi < 1024; pdi++) {
        	pd[pdi].raw = PTE_KERNEL;
        	pd[pdi].pfn = (pdi*(1<<(22-12))) | (1 << 5); //4Mb
    	}

    	//TODO map PD into virtual space smwhr between 3-4Gb (pdi~3xx)
    	pd[BAD_PDI].P = 0;
    	bad_pde_virtual_address = (uint32)&pd[BAD_PDI]; //TODO calculate virtual address of mapped PD

    	__asm {
        	//prepare PD
        	mov eax, pd
        	mov cr3, eax
        	//prepare 4Mb support
        	mov eax, cr4
        	or eax, 0x10
        	mov cr4, eax
        	//paging on
        	mov eax, cr0
        	or eax, 0x80000000
        	mov cr0, eax
        	//invalidate translation caches (recommended)
        	mov eax, cr3
        	mov cr3, eax
    	}

    	printf("Paging should work! \n");
    	printf("Generating PF... \n");

    	p = (uint32*)(BAD_ADDR);
    	*p = 0; //generate #PF

    	printf("Paging should work again! \n");
}
