CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic

.PHONY: all clean

all: assemble emulate

table.o: table.h
tokenizer.o: tokenizer.h
assemble_instructions.o: assemble_instructions.h intermediate_reps.h
parser.o: parser.h intermediate_reps.h table.h
pass.o: pass.h assemble_instructions.h intermediate_reps.h parser.h table.h
assemble.o: table.h pass.h
assemble: table.o tokenizer.o assemble_instructions.o parser.o pass.o

storage.o: storage.c storage.h
utilities.o: utilities.c utilities.h
instructions.o: instructions.c instructions.h utilities.h
pipeline.o: pipeline.c instructions.h pipeline.h storage.h utilities.h
emulate.o: emulate.c pipeline.h storage.h
emulate: storage.o utilities.o instructions.o pipeline.o

clean:
	$(RM) *.o assemble emulate
	$(RM) -r *.dSYM/
