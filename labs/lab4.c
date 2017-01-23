#include "stdio.h"

typedef unsigned int uint32;
typedef unsigned short int uint16;

#define IA32_APIC_BASE      0x1B     
#define APIC_EOI_REGISTER   0xB0     //EOI register
#define APIC_SPUR_REGISTER  0xF0     //Spurious interrupt
#define APIC_ICR_LO_REGISTER   0x300 //Interrupt command register0
#define APIC_ICR_HI_REGISTER   0x310 //Interrupt command register32
#define APIC_LVT_TIMER	   0x320 //LVT Timer

#define APIC_TMRINITCNT 	   0x380 //default timer time
#define APIC_TMRDIV		   0x3E0 //timer div

#pragma pack(push, 1)
typedef struct _DTR {
	uint16 limit;
	uint32 base;
	uint16 padding;
} DTR;

typedef union _IDTENTRY {
	struct {
        	uint16 offset_lo;
        	uint16 selector;
        	uint16 smth;
        	uint16 offset_hi;
    	};
    	struct {
        	uint32 raw0;
        	uint32 raw1;
    	};
} IDTENTRY, *PIDTENTRY;

typedef union _ICR_LO {
    	struct {
        	uint32 vector:          8;

        	uint32 delivery:        3;     //000 = Fixed
        	uint32 dest_mode:       1;     //- (0)
        	uint32 delivery_status: 1;     //- (0)
        	uint32 reserved:        1;
        	uint32 level:           1;     //1
        	uint32 trigger_mode:    1;     //0

        	uint32 reserved2:       2;
        	uint32 destination:     2;     //01 = Self
        	uint32 reserved3:      12;
    	};
    	uint32 raw;
} ICR_LO;

typedef union{
	struct{
		uint32 vector: 8;		//200
		uint32 reserved0: 4;
		uint32 delivery_status: 1;	//0
		uint32 reserved1: 3;
		uint32 mask: 1;		//0
		uint32 mode: 2;		//01
		uint32 reserved2: 13;
	};
	uint32 raw;
} LVT_Entry;
#pragma pack(pop)

#define MY_VECTOR 0xC7                  //200
#define MY_ICR_COMMAND    0x000440C7    //self fixed vector 200 (look in ICR_LO)
#define MY_LVT_ENTRY 0x000200C7;

uint32 old_handler;
uint32 old_selector;
uint32 my_apic_count = 0;

uint32 apic_base;
uint32 apic_icr_lo;
uint32 apic_eoi;

uint32 __declspec(naked) get_apic_base_msr()
{
    __asm {
        mov ecx, IA32_APIC_BASE
        rdmsr    //edx:eax -- reading msr
        retn
    }
}

void __declspec(naked) apic_handler(void)
{
    __asm {
        push eax
	 push edx

        inc my_apic_count
	 mov eax, apic_eoi
        mov [eax], 0            //any value

	 pop edx
        pop eax
        iretd
    }
}

uint32 n = 0;
void main()
{
    	int i;
    	DTR _idt;
    	uint16 _cs;
    	PIDTENTRY p_apic_entry_in_idt;
	uint32 *apic_timer;
	uint32 *spur_register;
	uint32 *apic_timerdef;
	uint32 *apic_timerdiv;

	LVT_Entry my_apic_timer;

	int old_apic;

	uint32 my_handler = (uint32)&apic_handler;
    	uint32 value = get_apic_base_msr();


    	printf("APIC enabled %s, APIC base 0x%08x \n", value & 0x800 ? "yes" : "no", value & (~0xFFF)); //checking 11 bit if enabled
    	apic_base = value & (~0xFFF); //APIC BASE -- cut 12 bits with info

    	__asm {
        	sidt _idt
        	mov ax, cs
        	mov _cs, ax
    	}

    	p_apic_entry_in_idt = &(((PIDTENTRY)_idt.base)[MY_VECTOR]);

	printf("14.1: 0x%08X'%08X \n", p_apic_entry_in_idt->raw1, p_apic_entry_in_idt->raw0);

    	old_handler = (p_apic_entry_in_idt->offset_hi << 16) | (p_apic_entry_in_idt->offset_lo);
    	old_selector = p_apic_entry_in_idt->selector;
    	p_apic_entry_in_idt->offset_hi = (uint16)(my_handler >> 16);
    	p_apic_entry_in_idt->offset_lo = (uint16)(my_handler & 0xFFFF);
    	p_apic_entry_in_idt->selector = _cs;

	printf("14.2: 0x%08X'%08X \n", p_apic_entry_in_idt->raw1, p_apic_entry_in_idt->raw0);

    	printf("IDT: 0x%08x\n", _idt.base);
    	printf("old_handler: 0x%04X:0x%08x\n", old_selector, old_handler);
    	printf("my_handler 0x%04X:0x%08x\n", _cs, my_handler);

    	apic_icr_lo = apic_base + APIC_ICR_LO_REGISTER; //Get ICR placement
    	apic_eoi = apic_base + APIC_EOI_REGISTER;	  //Get EOI placement
    	apic_timer = (uint32*) (apic_base + APIC_LVT_TIMER);
	spur_register = (uint32*) (apic_base + APIC_SPUR_REGISTER);
	apic_timerdef = (uint32*) (apic_base + APIC_TMRINITCNT);

    	printf("APIC BASE 0x%08X, EOI addr 0x%08X \n", apic_base, apic_eoi);

	*apic_timer = MY_LVT_ENTRY;
	*apic_timerdef = 0x00FFFFFF;

	for(i = 0;; i++){
		if ((i%100000)==0) {
            		printf("%d ",my_apic_count); //#GF after sometime
        	}
	}
}
