fast: main.c normxcorr2.c vendor/lodepng.c
	cc -framework Accelerate -Ivendor/ -DNORMXCORR2=normxcorr2 -O2 -g -o $@ $^

slow: main.c normxcorr2.c vendor/lodepng.c
	cc -framework Accelerate -Ivendor/ -DNORMXCORR2=normxcorr2_slow -O2 -g -o $@ $^

