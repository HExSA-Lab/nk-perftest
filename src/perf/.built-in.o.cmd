cmd_src/perf/built-in.o :=  ld -z max-page-size=0x1000 -melf_x86_64 -dp  -r -o src/perf/built-in.o src/perf/perf.o
