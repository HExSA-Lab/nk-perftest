CC:=gcc
WFLAGS:= -Wall -Wextra -Wno-unused-function
IFLAGS:=-I./include/
CFLAGS_COMMON:=-std=gnu99
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
               -m64
CFLAGS_OPT  :=$(IFLAGS) $(WFLAGS) $(CFLAGS_COMMON) $(CFLAGS_NAUT) -DNDEBUG
CFLAGS_DEBUG:=$(IFLAGS) $(WFLAGS) $(CFLAGS_COMMON) -Og -g -DVERBOSE -DSMALL

SOURCES:=$(shell find -L src/ -name '*.c' -printf '%P\n')
OBJECTS:=$(addprefix build/,$(SOURCES:.c=.o))
OBJECTS_DEBUG:=$(addprefix build/,$(SOURCES:.c=.o_debug))

build/%.o: src/%.c
	$(CC) $(CFLAGS_OPT) -c -o $@ $<

build/%.o_debug: src/%.c
	$(CC) $(CFLAGS_DEBUG) -c -o $@ $<

main: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

main_debug: $(OBJECTS_DEBUG)
	$(CC) $(LDFLAGS) -o $@ $^

run: main
	./main

run_small: main_debug
	./main_debug

memcheck: main_debug
	valgrind -q ./main_debug

debug: main_debug
	gdb -q main_debug -ex r ./main_debug

clean:
	@rm -rf main build src/app/*.o src/db/*.o src/app/.*.cmd src/db/.*.cmd
	mkdir -p build/database build/app

.PHONY: clean
