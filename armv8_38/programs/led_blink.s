movz w0, #0x3f20, lsl #16
movz w1, #0x0800, lsl #16
str	w1, [x0]
add	w0, w0, #0x1c
add	w1, w0, #0xc

loop:
	movz w2, #0x0200
	str	w2, [x0]
	movz x3, #0x0040, lsl #16
	b delay1

turn_off:
	movz w2, #0x0200
	str	w2, [x1]
	movz x3, #0x0040, lsl #16
	b delay2

delay1:
	subs x3, x3, #1
	b.ne delay1
	b turn_off

delay2:
	subs x3, x3, #1
	b.ne delay2
	b loop
