 
MBALIGN  equ  1<<0            
MEMINFO  equ  1<<1              
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0x1BADB002      
CHECKSUM equ -(MAGIC + FLAGS)  
 
bits 32
section .text
        ;multiboot spec
        align 4
        dd MAGIC              ;magic
        dd FLAGS                    ;flags
        dd CHECKSUM
global start
global keyboard_handler
global read_port
global write_port
global load_idt
 
extern kmain        ;this is defined in the c file
extern keyboard_handler_main
 
read_port:
    mov edx, [esp + 4]
            ;al is the lower 8 bits of eax
    in al, dx   ;dx is the lower 16 bits of edx
    ret
 
write_port:
    mov   edx, [esp + 4]    
    mov   al, [esp + 4 + 4]  
    out   dx, al  
    ret
 
load_idt:
    mov edx, [esp + 4]
    lidt [edx]
    sti                 ;turn on interrupts
    ret
 
keyboard_handler:                
    call    keyboard_handler_main
    iretd
 
start:
    cli                 ;block interrupts
    mov esp, stack_top
    call kmain
    hlt                 ;halt the CPU
 
section .bss
align 4
stack_bottom:
resb 16384 ; 16 KiB
stack_top: