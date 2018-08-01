CC:=gcc
WFLAGS:= -Wall -Wextra -Wno-unused-function
IFLAGS:=-I./include/
CVERSION := -std=gnu99
include src/app/macros.mk
# from nautilus/Makefile
CFLAGS_NAUT := -O2 \
               -fno-omit-frame-pointer \
               -fno-stack-protector \
               -fno-strict-aliasing \
               -fno-strict-overflow \
               -fno-delete-null-pointer-checks \
               -mno-red-zone \
               -mno-sse2 \
               -mcmodel=large \
               -fno-common \
               -Wstrict-overflow=5 \
               -fgnu89-inline \
               -g \
               -m64 \
                $(MACROS)
CFLAGS_OPT  := $(CVERSION) $(IFLAGS) $(WFLAGS) $(CFLAGS_NAUT)
# that line ^ should have no extra flags.
# -std=* -I* and -W* won't affect the outputed code, so they are fine.
# All flags which do (eg. -f* -O* -m*) should be a part of CFLAGS_NAUT
CFLAGS_DEBUG:= $(CVERSION) $(IFLAGS) $(WFLAGS) -fno-inline -static -Og -g -DVERBOSE -DSMALL -DREPLACE_MALLOC
# -DREPLACE_MALLOC

SOURCES:=$(shell find -L src/ -name '*.c' -printf '%P\n')
OBJECTS:=$(addprefix build/,$(SOURCES:.c=.o))
OBJECTS_DEBUG:=$(addprefix build/,$(SOURCES:.c=.o_debug))

build/%.o: src/%.c
	$(CC) $(CFLAGS_OPT) -c -o $@ $<

build/%.o_debug: src/%.c
	$(CC) $(CFLAGS_DEBUG) -c -o $@ $<

main: $(OBJECTS)
	$(CC) $(CFLAGS_OPT) $(LDFLAGS) -o $@ $^

main_debug: $(OBJECTS_DEBUG)
	$(CC) $(LDFLAGS) -o $@ $^

run: main
	./main

run_small: main_debug
	./main_debug

memcheck: main_debug
	valgrind -q ./main_debug
#  --track-origins=yes --leak-check=full

debug: main_debug
	gdb -q main_debug -ex r ./main_debug

clean:
	@rm -rf main main_debug build src/app/*.o src/db/*.o src/app/.*.cmd src/db/.*.cmd
	mkdir -p build/database build/app build/perf

.PHONY: clean
