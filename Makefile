main: main.c normxcorr2.c vendor/lodepng.c
	cc -framework Accelerate -Ivendor/ -O2 -g -o $@ $^
