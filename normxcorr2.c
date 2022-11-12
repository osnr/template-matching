#import <Accelerate/Accelerate.h>

#include "normxcorr2.h"

image_t imageNewInShapeOf(image_t im) {
    return (image_t) {
            .width = im.width,
            .height = im.height,
            .data = (float *) calloc(im.width * im.height, sizeof(float))
    };
}

void imageAddScalarInPlace(image_t im, const float scalar) {
    vDSP_vsadd(im.data, 1, &scalar, im.data, 1, im.width * im.height);
}
void imageDivideScalarInPlace(image_t im, const float scalar) {
    vDSP_vsdiv(im.data, 1, &scalar, im.data, 1, im.width * im.height);
}

float imageMean(const image_t im) {
    float ret;
    vDSP_meanv(im.data, 1, &ret, im.width * im.height);
    return ret;
}
float imageSumOfSquares(const image_t im) {
    float ret;
    vDSP_svesq(im.data, 1, &ret, im.width * im.height);
    return ret;
}

image_t fftconvolve(const image_t f, const image_t g) {
    // size = np.array(f.shape) + np.array(g.shape) - 1
    int width = f.width + g.width - 1;
    int height = f.height + g.height - 1;

    // fsize = 2 ** np.ceil(np.log2(size)).astype(int)
    int log2n0 = ceil(log2f(width));
    int fwidth = 1 << (int) log2n0;
    int log2n1 = ceil(log2f(height));
    int fheight = 1 << (int) log2n1;

    // f_ = np.fft.fft2(f, fsize)
    DSPSplitComplex f_ = (DSPSplitComplex) {
        .realp = (float *) calloc(fwidth * fheight, sizeof(float)),
        .imagp = (float *) calloc(fwidth * fheight, sizeof(float))
    };
    for (int y = 0; y < f.height; y++) {
        memcpy(&f_.realp[y * fwidth], &f.data[y * f.width], f.width * sizeof(float));
    }
    FFTSetup fftSetup = vDSP_create_fftsetup(log2n0 > log2n1 ? log2n0 : log2n1,
                                             kFFTRadix2);
    vDSP_fft2d_zip(fftSetup,
                   &f_, 1, 0,
                   log2n0, log2n1,
                   kFFTDirection_Forward);

    // g_ = np.fft.fft2(g, fsize)
    DSPSplitComplex g_ = (DSPSplitComplex) {
        .realp = (float *) calloc(fwidth * fheight, sizeof(float)),
        .imagp = (float *) calloc(fwidth * fheight, sizeof(float))
    };
    for (int y = 0; y < g.height; y++) {
        memcpy(&g_.realp[y * fwidth], &g.data[y * g.width], g.width * sizeof(float));
    }
    vDSP_fft2d_zip(fftSetup,
                    &g_, 1, 0,
                    log2n0, log2n1,
                    kFFTDirection_Forward);

    // FG = f_ * g_
    DSPSplitComplex FG = (DSPSplitComplex) {
        .realp = (float *) calloc(fwidth * fheight, sizeof(float)),
        .imagp = (float *) calloc(fwidth * fheight, sizeof(float))
    };
    vDSP_zvmul(&f_, 1, &g_, 1, &FG, 1, fwidth * fheight, 1);

    // return np.real(np.fft.ifft2(FG))
    vDSP_fft2d_zip(fftSetup,
                    &FG, 1, 0,
                    log2n0, log2n1,
                    kFFTDirection_Inverse);
    image_t ret = (image_t) {
        .width = width,
        .height = height,
        .data = (float *) calloc(width * height, sizeof(float))
    };
    vDSP_mmov(FG.realp, ret.data, width, height, fwidth, width);

    imageDivideScalarInPlace(ret, fwidth * fheight); // ifft2 normalization
    return ret;
}


#define s_(x, y) (((x) < 0 || (y) < 0 || (x >= image.width) || (y >= image.height)) ? 0 : s[((y)*image.width) + (x)])
#define s2_(x, y) (((x) < 0 || (y) < 0 || (x >= image.width) || (y >= image.height)) ? 0 : s2[((y)*image.width) + (x)])

image_t normxcorr2(image_t templ, image_t image) {
    // template = template - np.mean(template)
    imageAddScalarInPlace(templ, -1 * imageMean(templ));

    // image = image - np.mean(image)
    imageAddScalarInPlace(image, -1 * imageMean(image));

    // ar = np.flipud(np.fliplr(template))
    image_t ar = imageNewInShapeOf(templ);
    for (int y = 0; y < templ.height; y++) {
        for (int x = 0; x < templ.width; x++) {
            int flippedY = templ.height - 1 - y;
            int flippedX = templ.width - 1 - x;
            ar.data[flippedY * templ.width + flippedX] = templ.data[y * templ.width + x];
        }
    }

    // out = fftconvolve(image, ar.conj())
    image_t outi = fftconvolve(image, ar);

    // a1 = np.ones(template.shape)
    // image = fftconvolve(np.square(image), a1) - np.square(fftconvolve(image, a1)) / np.prod(template.shape)
    // summed-area tables
    double *s = (double*) calloc(image.width * image.height, sizeof(double));
    double *s2 = (double*) calloc(image.width * image.height, sizeof(double));
    for (int y = 0; y < image.height; y++) {
        for (int x = 0; x < image.width; x++) {
            s[y*image.width + x] = image.data[y*image.width + x] + s_(x-1, y) + s_(x, y-1) - s_(x-1, y-1);
            s2[y*image.width + x] = image.data[y*image.width + x]*image.data[y*image.width + x] + s2_(x-1, y) + s2_(x, y-1) - s2_(x-1, y-1);
        }
    }

    // image[np.where(image < 0)] = 0
    // template = np.sum(np.square(template))
    // out = out / np.sqrt(image * template)
    float templateSum = imageSumOfSquares(templ);
    for (int y = 0; y < outi.height; y++) {
        for (int x = 0; x < outi.width; x++) {
            double imageSum = s_(x, y)
                - s_(x - templ.width, y)
                - s_(x, y - templ.height)
                + s_(x - templ.width, y - templ.height);

            double energy = s2_(x, y)
                - s2_(x - templ.width, y)
                - s2_(x, y - templ.height)
                + s2_(x - templ.width, y - templ.height);

            float d = energy - 1.0f/(templ.width * templ.height) * imageSum * imageSum; // from Briechle (10)
            if (d < 0) d = 0;

            int i = y * outi.width + x;
            outi.data[i] /= sqrt(d * templateSum);

            // out[np.where(np.logical_not(np.isfinite(out)))] = 0
            if (isinf(outi.data[i]) || isnan(outi.data[i])) {
                outi.data[i] = 0.0f;
            }
        }
    }

    // return out
    return outi;
}

image_t normxcorr2_slow(image_t templ, image_t image) {
    float templMean = imageMean(templ);

    image_t result;
    result.width = image.width + templ.width - 1;
    result.height = image.height + templ.height - 1;
    result.data = (float *)calloc(result.width * result.height, sizeof(float));

    for (int v = 0; v < result.height; v++) {
        for (int u = 0; u < result.width; u++) {
            float mean = 0; // mean of image(x, y) in the region under the template
            for (int y = v; (y - v) < templ.height; y++) {
                for (int x = u; (x - u) < templ.width; x++) {
                    mean += image.data[y * image.width + x];
                }
            }
            mean /= templ.width * templ.height;

            float num = 0;
            float denomLeft = 0, denomRight = 0;
            for (int y = v; (y - v) < templ.height; y++) {
                for (int x = u; (x - u) < templ.width; x++) {
                    num += (image.data[y * image.width + x] - mean) *
                        (templ.data[(y - v) * templ.width + (x - u)] - templMean);
                    denomLeft += (image.data[y * image.width + x] - mean) *
                        (image.data[y * image.width + x] - mean);
                    denomRight += (templ.data[(y - v) * templ.width + (x - u)] - templMean) *
                        (templ.data[(y - v) * templ.width + (x - u)] - templMean);
                }
            }
            
            result.data[v * result.width + u] = num / sqrt(denomLeft * denomRight);
        }
    }
    return result;
}
