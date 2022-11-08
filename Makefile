main: main.c normxcorr2.c
	cc -framework Accelerate -O2 -g -o $@ $<
