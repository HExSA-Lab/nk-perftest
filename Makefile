CFLAGS=-std=gnu99 -g -I./include/

main: main.c
	$(CC) $(CFLAGS) $(OPT_FLAGS) -o main main.c


clean:
	$(RM) main

.PHONY: clean
