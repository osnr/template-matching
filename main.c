#include "normxcorr2.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
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
image_t pngFileToImage(const char *filename, int downscale) {
    uint8_t *rgba; unsigned rows; unsigned cols;
    if (lodepng_decode32_file(&rgba, &cols, &rows, filename)) { exit(1); }
    return rgbaToImage((uint32_t *)rgba, rows, cols, cols * 4, downscale);
}
void imageToPngFile(image_t image, const char *filename) {
    uint8_t data8[image.width * image.height];
    for (int y = 0; y < image.height; y++) {
        for (int x = 0; x < image.width; x++) {
            data8[y * image.width + x] = image.data[y * image.width + x] * 255;
        }
    }
    lodepng_encode_file(filename, data8, image.width, image.height,
                        LCT_GREY, 8);
}

void hit(uint32_t *orig, unsigned w, int x0, int y0, int templWidth, int templHeight) {
    printf("hit at (%d, %d)\n", x0, y0);
    for (int y = 0; y < templHeight; y++) {
        orig[(y0 + y) * w + x0 + 0] = 0xFF0000FF; // left edge
        orig[(y0 + y) * w + x0 + templWidth] = 0xFF0000FF; // right edge
    }
    for (int x = 0; x < templWidth; x++) {
        orig[(y0 + 0) * w + x0 + x] = 0xFF0000FF; // top edge
        orig[(y0 + templHeight) * w + x0 + x] = 0xFF0000FF; // bottom edge
    }
}

int main(int argc, char* argv[]) {
    const char *templFilename, *imageFilename;
    const int DOWNSCALE = 2;

    image_t templ, image;
    if (argc == 3) {
        templFilename = argv[1];
        imageFilename = argv[2];
    } else if (argc == 1) {
        templFilename = "examples/template-traffic-lights.png";
        imageFilename = "examples/screen.png";
    } else {
        fprintf(stderr, "Usage: %s TEMPLATE IMAGE\n", argv[0]);
        exit(1);
    }
    templ = pngFileToImage(templFilename, DOWNSCALE);
    image = pngFileToImage(imageFilename, DOWNSCALE);

    // time matching:
    image_t result;
    {
        clock_t begin = clock();

        result = NORMXCORR2(templ, image);

        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("image %dx%d, templ %dx%d -> result %dx%d in %f sec\n",
               image.width, image.height, templ.width, templ.height,
               result.width, result.height,
               time_spent);
    }
    imageToPngFile(result, "result.png");

    // report results:
    uint32_t* orig; unsigned w; unsigned h;
    lodepng_decode32_file((unsigned char **)&orig, &w, &h, imageFilename);

    int hits = 0;
    for (int ry = 0; ry < result.height; ry++) {
        for (int rx = 0; rx < result.width; rx++) {
            if (result.data[ry * result.width + rx] > 0.98) {
                hit(orig, w, (rx - templ.width)*DOWNSCALE, (ry - templ.height)*DOWNSCALE,
                    templ.width*DOWNSCALE, templ.height*DOWNSCALE);
                hits++;
            }
        }
    }
    printf("hits: %d\n", hits);
    lodepng_encode32_file("hits.png", (unsigned char *)orig, w, h);

    return 0;
}
