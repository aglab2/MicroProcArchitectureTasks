#include <stdio.h>

typedef unsigned int uint16;
typedef unsigned short int uint8;

typedef struct _IVT{
       uint16 seg;
       uint16 ofsset;
} IVT_ENTRY, *PIVT_ENTRY; //Interrupt vector table

void __stdcall c_handler (void)
{
       printf("Hi handler!\n");
}

void __declspec(naked) handler(){
       __asm{
              push ax
              push bx
              push cx
              push dx
              push bp
              push di
              push si
              call c_handler
              pop si
              pop di
              pop bp
              pop dx
              pop cx
              pop bx
              pop ax
              iret
       }
}

void __declspec(naked) bad(){
       __asm{
              mov eax, 0
       }
}

void __declspec(naked) safer(){
       again:
       __asm{
              int 17
       }
       goto again;
}

void main(){
       printf("hi!\n");
       __asm {
              push es
              mov ax, 0
              mov es, ax
              mov es:[200*4], offset handler
              mov es:[200*4+2], seg handler
              pop es
              int 200
       }
       printf("by!\n");
}
