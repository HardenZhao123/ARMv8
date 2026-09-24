.section ".text.boot"
.globl _start

_start:
        // Initialise stack pointer
        ldr     x0, =__stack_top
        mov     sp, x0

        // Initialise .bss
        ldr     x0, =__bss_start
        ldr     x1, =__bss_end
clear_loop:
        cmp     x0, x1
        b.hs    call_main
        strb	wzr, [x0], #1 // clear 1 byte only at a time
        b       clear_loop
    
call_main:
        bl      main

hang:
        wfi
        b       .

// vim: set expandtab shiftwidth=8 tabstop=8:
