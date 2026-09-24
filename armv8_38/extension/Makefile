CC	= clang
LD	= ld.lld
OBJCOPY	= llvm-objcopy

ASFLAGS	= --target=aarch64-none-elf
CFLAGS	= --target=aarch64-none-elf -O
LDFLAGS	= -T linker.ld --nostdlib

OBJ	= boot.o main.o
ELF	= kernel8.elf
IMG	= boot/kernel8.img

.PHONY: all
all: $(IMG)

$(IMG): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(ELF): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJ):

clean:
	rm -f $(OBJ) $(ELF) $(IMG)
