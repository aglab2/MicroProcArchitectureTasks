#include "stdio.h"

typedef unsigned int uint32;
typedef unsigned short int uint16;

#define IA32_APIC_BASE      0x1B
#define APIC_EOI_REGISTER   0xB0
#define APIC_ICR_LO_REGISTER   0x300
#define APIC_ICR_HI_REGISTER   0x310

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
        uint32 dest_mode:       1;     //-
        uint32 delivery_status: 1;     //-
        uint32 reserved:        1;
        uint32 level:           1;     //1
        uint32 trigger_mode:    1;     //0
        uint32 reserved2:       2;
        uint32 destination:     2;     //01 = Self
        uint32 reserved3:      12;
    };
    uint32 raw;
} ICR_LO;
#pragma pack(pop)

#define MY_VECTOR 0xC8                  //200
#define MY_ICR_COMMAND    0x000440C8    //self fixed vector 200

uint32 old_handler;
uint32 old_selector;
uint32 my_apic_count = 0;

uint32 my_apic_base;
uint32 my_apic_icr_lo;
uint32 my_apic_eoi;

uint32 __declspec(naked) get_apic_base_msr()
{
    __asm {
        mov ecx, IA32_APIC_BASE
        rdmsr    //edx:eax
        retn
    }
}

void __declspec(naked) apic_handler(void)
{
    __asm {
        push eax

        inc my_apic_count
        mov eax, my_apic_eoi
        mov [eax], 0            //any value

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
    uint32 my_handler = (uint32)&apic_handler;

    uint32 value = get_apic_base_msr();
    printf("APIC enabled %s, APIC base 0x%08x \n", value & 0x800 ? "yes" : "no", value & (~0xFFF));
    my_apic_base = value & (~0xFFF);

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

    my_apic_icr_lo = my_apic_base + APIC_ICR_LO_REGISTER;
    my_apic_eoi = my_apic_base + APIC_EOI_REGISTER;

    printf("APIC BASE 0x%08X, EOI addr 0x%08X \n", my_apic_base, my_apic_eoi);

    for (i = 0;;i++)
    {
        uint32* p = (uint32*)my_apic_icr_lo;
        *p = MY_ICR_COMMAND;
        if ((i%10000)==0) {
            printf("my_apic_count = %d \n",my_apic_count);
        }
        if ((n%10000)==0) {
            printf("... %d \n", n++);
        }
    }
    printf("NEVER \n");
}
