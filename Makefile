CC=gcc
CFLAGS=-std=gnu99 -g -I./include/
OPT_FLAGS=-O2 -fno-omit-frame-pointer -fno-stack-protector -fno-strict-aliasing -fno-strict-overflow -fno-delete-null-pointer-checks -fno-common -fgnu89-inline -mno-red-zone -mno-sse2 -mcmodel=large -m64

main: main.c
	$(CC) $(CFLAGS) $(OPT_FLAGS) -o main main.c

run: main
	./main

clean:
	$(RM) main

.PHONY: clean
