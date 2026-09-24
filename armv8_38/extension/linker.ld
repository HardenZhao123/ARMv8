SECTIONS {
	. = 0x80000;
	__stack_top = .;
	.text.boot : { *(.text.boot) }
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	.data : { *(.data) }
	__bss_start = .;
	.bss : { *(.bss*) }
	__bss_end = .;
	/DISCARD/ : { *(.comment) }
}
