#include "normxcorr2.h"

#include <stdlib.h>
#include <stdio.h>
#include "lodepng.h"

void imageDivideScalarInPlace(image_t im, const float scalar);
image_t rgbaToImage(uint32_t *rgba32, unsigned rows, unsigned cols, unsigned bytesPerRow, int downscale) {
    image_t ret = (image_t) {
        .width = cols/downscale,
        .height = rows/downscale,
        .data = (float *) calloc(cols/downscale * rows/downscale, sizeof(float))
    };
    // downsample and accumulate in ret
    uint8_t *rgba = (uint8_t *) rgba32;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            int i = (y * bytesPerRow) + x*4;
            uint8_t r = rgba[i];
            uint8_t g = rgba[i + 1];
            uint8_t b = rgba[i + 2];
            int reti = (y/downscale)*ret.width + x/downscale;
            ret.data[reti] += (r/255.0)*0.3 + (g/255.0)*0.58 + (b/255.0)*0.11;
        }
    }
    imageDivideScalarInPlace(ret, downscale * downscale);
    return ret;
}
image_t pngFileToImage(const char *filename) {
    uint8_t *rgba; unsigned rows; unsigned cols;
    if (lodepng_decode32_file(&rgba, &cols, &rows, filename)) { exit(1); }
    return rgbaToImage((uint32_t *)rgba, rows, cols, cols*4, 2);
}

int main(int argc, char* argv[]) {
    image_t templ, image;
    if (argc == 3) {
        templ = pngFileToImage(argv[1]);
        image = pngFileToImage(argv[2]);
    } else if (argc == 1) {
        templ = pngFileToImage("examples/template-traffic-lights.png");
        image = pngFileToImage("examples/screen.png");
    } else {
        fprintf(stderr, "Usage: %s TEMPLATE IMAGE\n", argv[0]);
        exit(1);
    }

    image_t result = normxcorr2(templ, image);

    int hits = 0;
    for (int y = 0; y < result.height; y++) {
        for (int x = 0; x < result.width; x++) {
            if (result.data[y * result.width + x] > 0.98) {
                hits++;
                /* hit(image, templ, x, y); */
            }
        }
    }
    printf("hits: %d\n", hits);

    return 0;
}
