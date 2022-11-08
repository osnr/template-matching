#include "normxcorr2.h"

static const float MATCH_SCALE = 0.5f;

image_t toImage(cv::Mat matOrig) {
    cv::Mat mat;
    cv::resize(matOrig, mat, cv::Size(), MATCH_SCALE, MATCH_SCALE);
    
    image_t ret = (image_t) {
        .width = mat.cols,
        .height = mat.rows,
        .data = (float *) malloc(mat.cols * mat.rows * sizeof(float))
    };
    for (int y = 0; y < ret.height; y++) {
        for (int x = 0; x < ret.width; x++) {
            int i = ((y * ret.width) + x) * 3;
            uint8 r = mat.data[i];
            uint8 g = mat.data[i + 1];
            uint8 b = mat.data[i + 2];
            ret.data[y * ret.width + x] = (r/255.0)*0.3 + (g/255.0)*0.58 + (b/255.0)*0.11;
        }
    }
    return ret;
}

// int main() {
//     float inputData[] = {
//         1, 2, 3,
//         4, 5, 6,
//         7, 8, 9
//     };
//     image_t input = (image_t) { .data = inputData, .width = 3, .height = 3 };
//     float kernelData[] = {
//         -1, -2, -1,
//         0, 0, 0,
//         1, 2, 1
//     };
//     image_t kernel = (image_t) { .data = kernelData, .width = 3, .height = 3 };

//     image_t output = fftconvolve(input, kernel);
//     imagePrint("output", output);
// }

/* void hit(cv::Mat& orig, image_t templ, int x, int y) { */
/*     cv::Point origin((x - templ.width)*(1/MATCH_SCALE), (y - templ.height)*(1/MATCH_SCALE)); */
/*     cv::Point to((x - templ.width + templ.width)*(1/MATCH_SCALE), (y - templ.height + templ.height)*(1/MATCH_SCALE)); */
/*     cv::rectangle(orig, origin, to, cv::Scalar(255, 0, 255)); */
/* } */

int main() {
    image_t templ = toImage(cv::imread("template-traffic-lights.png"));
    image_t image = toImage(cv::imread("screen.png"));

    cv::TickMeter tm;
    tm.start();
    image_t result = normxcorr2(templ, image);
    tm.stop();
    std::cout << "Total time: " << tm.getTimeSec() << std::endl;

    cv::Mat orig = cv::imread("screen.png");
    
    if (0) { // max-value strategy
        int maxX, maxY;
        float maxValue = -10000.0f;
        for (int y = 0; y < result.height; y++) {
            for (int x = 0; x < result.width; x++) {
                if (result.data[y * result.width + x] > maxValue) {
                    maxX = x;
                    maxY = y;
                    maxValue = result.data[y * result.width + x];
                }
            }
        }
        printf("maxValue (%d, %d) = %f\n", maxX, maxY, maxValue);
    }
    // imageShow("result", result);

    if (1) { // threshold strategy
        int hits = 0;
        for (int y = 0; y < result.height; y++) {
            for (int x = 0; x < result.width; x++) {
                if (result.data[y * result.width + x] > 0.98) {
                    hits++;
                    hit(orig, templ, x, y);
                }
            }
        }

        std::cout << "hits: " << hits << std::endl;
    }

    cv::imshow("orig", orig);
    while (cv::waitKey(0) != 27) {}

    return 0;
}
