NORMXCORR2 := normxcorr2
main: main.c normxcorr2.c vendor/lodepng.c
	cc -framework Accelerate -Ivendor/ -DNORMXCORR2=$(NORMXCORR2) -O2 -g -o $@ $^
