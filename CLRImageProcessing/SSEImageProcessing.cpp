#include "pch.h"
#include "SSEImageProcessing.h"
#include <intrin.h>
#include "math.h"
#include <complex>
#include <vector>
using namespace std;

// ��ȣ ���� 8��Ʈ ���� �񱳸� ���� �Լ�
__m256i _mm256_cmpgt_epu8(__m256i a, __m256i b)
{
    __m256i sign = _mm256_set1_epi8(128);
    __m256i subSign_a = _mm256_sub_epi8(a, sign);
    __m256i subSign_b = _mm256_sub_epi8(b, sign);
    return _mm256_cmpgt_epi8(subSign_a, subSign_b);
}
int _mm256_sum_epi32(__m256i a) {
    int sumResult = 0;
    sumResult += _mm256_extract_epi32(a, 0);
    sumResult += _mm256_extract_epi32(a, 1);
    sumResult += _mm256_extract_epi32(a, 2);
    sumResult += _mm256_extract_epi32(a, 3);
    sumResult += _mm256_extract_epi32(a, 4);
    sumResult += _mm256_extract_epi32(a, 5);
    sumResult += _mm256_extract_epi32(a, 6);
    sumResult += _mm256_extract_epi32(a, 7);

    return sumResult;
}

void FFT(vector<complex<double>>& x, bool inverse) {
    const double pi = System::Math::PI;
    int N = x.size();  // assume n is a power of 2

    // bit reversal permutation
    for (int i = 1, rev = 0; i < N; i++) {
        int bit = (N >> 1);
        while (rev >= bit) rev -= bit, bit >>= 1;
        rev += bit;
        // bit reversal is symmetric
        if (i < rev) swap(x[i], x[rev]);
    }

    // iterative implementation
    for (int i = 2; i <= N; i <<= 1) {
        for (int j = 0; j < N; j += i)
            for (int k = 0; k < i / 2; k++) {
                complex<double> Wk = polar(1.0, (inverse ? -1 : 1) * -2.0 * pi * k / i);
                complex<double> even = x[j + k], odd = x[j + k + i / 2];
                x[j + k] = even + Wk * odd;
                x[j + k + i / 2] = even - Wk * odd;
            }
    }

    // IFFT scaling
    if (inverse) for (int i = 0; i < N; i++) x[i] /= N;
}

void SSEBinarize(uint8_t* src, uint8_t* des, int width, int height, int threshold)
{
    int pixelNum = width * height;
    // �Ӱ谪�� ���ͷ� ��ȯ
    __m256i vecThreshold = _mm256_set1_epi8(threshold);

    for (int i = 0; i <= pixelNum - 32; i += 32) {
        // �ȼ� ���� ���ͷ� �ε�
        __m256i pixel = _mm256_loadu_si256((__m256i*) & src[i]);

        // �ȼ� ���� �Ӱ谪���� ũ�� 0xFF, �׷��� ������ 0x00���� ����
        __m256i cmpResult = _mm256_cmpgt_epu8(pixel, vecThreshold);

        // ����̹����� ����
        _mm256_storeu_si256((__m256i*) & des[i], cmpResult);
    }

    // ���� �ȼ� ó��
    for (int i = pixelNum - (pixelNum % 32); i < pixelNum; i++) {
        des[i] = src[i] > threshold ? 255 : 0;
    }
}

void SSEErosion(uint8_t* src, uint8_t* des, int width, int height, int threshold, int kernelX, int kernelY)
{
    // ����ȭ ����
	SSEBinarize(src, src, width, height, threshold);

    int halfKernelX = kernelX / 2;
    int halfKernelY = kernelY / 2;

    // ��ü �ȼ� ��ȸ
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x += 32) {
            // Ŀ�� ���� ��ȸ�ϸ� Ŀ�ο����� �ȼ��� �ϳ��� 0�ΰ��
            // ��� �ȼ��� ���� 0���� �����ϱ� ���� Ŀ�ο����� �ּҰ��� ������Ʈ �ϴ� ���� ����
            __m256i minVal = _mm256_set1_epi8(255);
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // �̹��� ���� ���� �ȼ��� ó��
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        __m256i pixel = _mm256_loadu_si256((__m256i*) & src[ny * width + nx]);
                        minVal = _mm256_min_epu8(minVal, pixel);
                    }
                }
            }
            // ����̹����� ����
            _mm256_storeu_si256((__m256i*) & des[y * width + x], minVal);
        }
    }
}

void SSEDilation(uint8_t* src, uint8_t* des, int width, int height, int threshold, int kernelX, int kernelY)
{
    // ����ȭ ����
    SSEBinarize(src, src, width, height, threshold);

    int halfKernelX = kernelX / 2;
    int halfKernelY = kernelY / 2;

    // ��ü �ȼ� ��ȸ
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x += 32) {
            // Ŀ�� ���� ��ȸ�ϸ� Ŀ�ο����� �ȼ��� �ϳ��� 0�� �ƴѰ��
            // ��� �ȼ��� ���� 255���� �����ϱ� ���� Ŀ�ο����� �ּҰ��� ������Ʈ �ϴ� ���� ����
            __m256i maxVal = _mm256_setzero_si256();
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // �̹��� ���� ���� �ȼ��� ó��
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        __m256i pixel = _mm256_loadu_si256((__m256i*) & src[ny * width + nx]);
                        maxVal = _mm256_max_epu8(maxVal, pixel);
                    }
                }
            }
            // ����̹����� ����
            _mm256_storeu_si256((__m256i*) & des[y * width + x], maxVal);
        }
    }
}

void SSEGaussianBlur(uint8_t* src, uint8_t* des, int width, int height, double* kernel) {
    // Ŀ�� ũ�� ���
    int arrSize = _msize(kernel) / sizeof(double);
    int kernelWidth = static_cast<int>(sqrt(arrSize));

    int halfKernelX = kernelWidth / 2;
    int halfKernelY = kernelWidth / 2;

    // ��ü �ȼ� ��ȸ
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width - 4; x += 4) { // 4���� �ȼ��� �� ���� ó��
            // Ŀ���� ���� ���� �ȼ� ���� ���� ���� ������ ����
            __m256d sumVal = _mm256_setzero_pd();

            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // �̹��� ���� ���� �ȼ��� ó��
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        // ���� Ŀ���� ����ġ �ε�
                        double kernelVal = kernel[(ky + halfKernelY) * kernelWidth + (kx + halfKernelX)];
                        // 4���� �ȼ� �� �ε� �� ��ȯ
                        // 4�� �ȼ��� �ε� 
                        __m128i pixel8 = _mm_loadu_si32(&src[ny * width + nx]);
                        // 4���� �ȼ� �����͸� zeroExtend
                        __m256i pixel32 = _mm256_cvtepu8_epi32(pixel8);
                        // ���� ������ �����͸� ������ ����ؾ��� �ȼ� 4���� �ε��Ҽ��������� ����
                        __m256d pixel = _mm256_cvtepi32_pd(_mm256_castsi256_si128(pixel32));
                        // Ŀ�� ���� ����ȭ
                        __m256d kernelValVec = _mm256_set1_pd(kernelVal);
                        // ���� ��� ���(���� �����÷ο찡 �߻����� �ʴ´� Ȯ���� �����Ƿ� �ٷ� ���)
                        __m256d mulResult = _mm256_mul_pd(pixel, kernelValVec);
                        // ������ ���
                        sumVal = _mm256_add_pd(sumVal, mulResult);
                    }
                }
            }

            // ����� uint8_t�� ��ȯ�Ͽ� ������ �̹����� ����
            // �ε� �Ҽ��� ���� 32��Ʈ ������ ��ȯ
            __m128i intSumVal128 = _mm256_cvtpd_epi32(sumVal); 
            // __m128i -> __m256i�� zero Expend ĳ����
            __m256i intSumVal = _mm256_castsi128_si256(intSumVal128);
            // �����͸� 32bit -> 16bit���� ��ȯ�ϱ����� zero data�� ��ŷ
            __m128i packedSumVal = _mm_packus_epi32(_mm256_extracti128_si256(intSumVal, 0), _mm256_extracti128_si256(intSumVal, 1));
            // �����͸� 16bit -> 8bit���� ��ȯ�ϰ� ���������ʹ� ������� ���� �����̹Ƿ� �ڱ��ڽŰ� ��ŷ
            packedSumVal = _mm_packus_epi16(packedSumVal, packedSumVal);
            // ����� ���� 32bit ����
            _mm_storeu_si32((__m128i*) & des[y * width + x], packedSumVal);
        }
    }
}

void SSEEqualization(uint8_t* src, uint8_t* des, int width, int height)
{
    int pixelNum = width * height;
    int pixelValCntCumulSum[256] = {0};

    // 1. ��ü �ȼ��� �׷��� ���� ī��Ʈ ���ϱ�
    for (int i = 0; i < pixelNum; i++)
    {
        uint8_t curPixelVal = src[i * sizeof(uint8_t)];
        pixelValCntCumulSum[curPixelVal]++;
    }

    // 2. ������ �� ��Ȱȭ ��� ���
    // ��Ȱȭ ��� = ������ / �� �ȼ� ���� * MAX �ȼ� ��
    for (int i = 1; i < 256; i++)
    {
        pixelValCntCumulSum[i] += pixelValCntCumulSum[i - 1];
        double tempData = (double)pixelValCntCumulSum[i-1] / pixelNum * 255.0;
        pixelValCntCumulSum[i - 1] = floor(tempData + 0.5);
    }
    double tempData = (double)pixelValCntCumulSum[255] / pixelNum * 255.0;
    pixelValCntCumulSum[255] = floor(tempData + 0.5);

    // 3. �ȼ� ����
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            des[y * width + x] = (uint8_t)pixelValCntCumulSum[src[y * width + x]];
        }
    }
}

void SSESobel(uint8_t* src, uint8_t* des, int width, int height)
{
    //y�� ��̺� ����
    double sobelYFilter[3][3] = 
    {
        {-1,-2,-1},
        {0 ,0 ,0},
        {1 ,2 ,1}
    };
    //x�� ��̺� ����
    double sobelXFilter[3][3] =
    {
        {-1 ,0 ,1},
        {-2 ,0 ,2},
        {-1 ,0 ,1}
    };
    int filterWidth = 3;

    int halfKernelX = filterWidth / 2;
    int halfKernelY = filterWidth / 2;

    // ��ü �ȼ� ��ȸ
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width - 16; x += 16) { // 4���� �ȼ��� �� ���� ó��
            // Ŀ���� ���� ���� �ȼ� ���� ���� ���� ������ ����
            __m256i sumVal_X = _mm256_setzero_si256();
            __m256i sumVal_Y = _mm256_setzero_si256();
            __m256i sumVal_Result = _mm256_setzero_si256();
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // �̹��� ���� ���� �ȼ��� ó��
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        // ���� ���Ͱ� �ε�
                        int FilterVal_X = sobelXFilter[ky + halfKernelY][kx + halfKernelX];
                        int FilterVal_Y = sobelYFilter[ky + halfKernelY][kx + halfKernelX];

                        // 16�� �ȼ��� �ε� 
                        __m128i pixel16 = _mm_loadu_si128((__m128i*) & src[ny * width + nx]);
                        // 16���� �ȼ� �����͸� zeroExtend
                        __m256i pixel = _mm256_cvtepu8_epi16(pixel16);
                        // Ŀ�� ���� ����ȭ
                        __m256i filterXVec = _mm256_set1_epi16(FilterVal_X);
                        __m256i filterYVec = _mm256_set1_epi16(FilterVal_Y);
                        // ���� ��� ���(���� �����÷ο찡 �߻����� �ʴ´� Ȯ���� �����Ƿ� �ٷ� ���)
                        __m256i mulResultX = _mm256_mullo_epi16(pixel, filterXVec);
                        __m256i mulResultY = _mm256_mullo_epi16(pixel, filterYVec);
                        // ������ ���
                        sumVal_X = _mm256_add_epi16(sumVal_X, mulResultX);
                        sumVal_Y = _mm256_add_epi16(sumVal_Y, mulResultY);
                    }
                }
            }
            // �� �̺� ����� ��ħ
            sumVal_Result = _mm256_add_epi16(sumVal_X, sumVal_Y);
            // ��ģ ����� ���밪 ��ȯ
            sumVal_Result = _mm256_abs_epi16(sumVal_Result);
            // ��� ���� 16 bit -> 8bit�� ��ȯ ������� 0 Extend
            __m256i packResult = _mm256_packus_epi16(sumVal_Result, _mm256_setzero_si256());
            // �ʿ��� ������ ����
            __m128i resultData1 = _mm256_extracti128_si256(packResult, 0);
            __m128i resultData2 = _mm256_extracti128_si256(packResult, 1);
            // ����� ���� 64bit�� ������ ����
            _mm_storeu_si64((__m128i*) & des[y * width + x], resultData1);
            _mm_storeu_si64((__m128i*) & des[y * width + x + 8], resultData2);
        }
    }
}

void SSELaplacian(uint8_t* src, uint8_t* des, int width, int height)
{
	// ���ö�þ� ����
	double laplacianFilter[3][3] =
	{
		{1,1,1},
		{1,-8,1},
		{1,1,1}
	};
    int filterWidth = 3;

    int halfKernelX = filterWidth / 2;
    int halfKernelY = filterWidth / 2;

    // ��ü �ȼ� ��ȸ
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width - 16; x += 16) { // 4���� �ȼ��� �� ���� ó��
            // Ŀ���� ���� ���� �ȼ� ���� ���� ���� ������ ����
            __m256i sumVal_Result = _mm256_setzero_si256();

            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // �̹��� ���� ���� �ȼ��� ó��
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        // ���� ���Ͱ� �ε�
                        int FilterVal = laplacianFilter[ky + halfKernelY][kx + halfKernelX];

                        // 16�� �ȼ��� �ε� 
                        __m128i pixel16 = _mm_loadu_si128((__m128i*) & src[ny * width + nx]);
                        // 16���� �ȼ� �����͸� zeroExtend
                        __m256i pixel = _mm256_cvtepu8_epi16(pixel16);
                        // Ŀ�� ���� ����ȭ
                        __m256i filterXVec = _mm256_set1_epi16(FilterVal);
                        // ���� ��� ���(���� �����÷ο찡 �߻����� �ʴ´� Ȯ���� �����Ƿ� �ٷ� ���)
                        __m256i mulResult = _mm256_mullo_epi16(pixel, filterXVec);
                        // ������ ���
                        sumVal_Result = _mm256_add_epi16(sumVal_Result, mulResult);
                    }
                }
            }

            // ��ģ ����� ���밪 ��ȯ
            sumVal_Result = _mm256_abs_epi16(sumVal_Result);
            // ��� ���� 16 bit -> 8bit�� ��ȯ ������� 0 Extend
            __m256i packResult = _mm256_packus_epi16(sumVal_Result, _mm256_setzero_si256());
            // �ʿ��� ������ ����
            __m128i resultData1 = _mm256_extracti128_si256(packResult, 0);
            __m128i resultData2 = _mm256_extracti128_si256(packResult, 1);
            // ����� ���� 64bit�� ������ ����
            _mm_storeu_si64((__m128i*) & des[y * width + x], resultData1);
            _mm_storeu_si64((__m128i*) & des[y * width + x + 8], resultData2);
        }
    }
}

void SSEFFTransform(uint8_t* src, uint8_t* des, int width, int height)
{
    complex<double> com1;
}

void SSETempleteMatching(uint8_t* src, uint8_t* templete, int srcWidth, int srcHeight, int tmpWidth, int tmpHeight, int matchingRate, int& matchPointX, int& matchPointY)
{
    // ��ü �ȼ� ��ȸ
    int maxRate = 0;
    int maxStartX = 0;
    int maxStartY = 0;

    for (int y = 0; y < srcHeight - tmpHeight; y++) {
        for (int x = 0; x < srcWidth - tmpWidth - 8; x++) { // 4���� �ȼ��� �� ���� ó��

            __m256i sumVal = _mm256_setzero_si256();
            int difAvg = 0;
            for (int ky = 0; ky < tmpHeight; ky++) {
                for (int kx = 0; kx < tmpWidth - 8; kx += 8) {
                    int ny = y + ky;
                    int nx = x + kx;

                    __m128i srcPixel16 = _mm_loadu_si64((__m128i*) & src[ny * srcWidth + nx]);
                    __m256i srcPixel = _mm256_cvtepu8_epi32(srcPixel16);
                    __m128i tmpPixel16 = _mm_loadu_si64((__m128i*) & templete[ky * tmpWidth + kx]);
                    __m256i tmpPixel = _mm256_cvtepu8_epi32(tmpPixel16);

                    __m256i subVal = _mm256_sub_epi32(srcPixel, tmpPixel);
                    __m256i absVal = _mm256_abs_epi32(subVal);

                    sumVal = _mm256_add_epi32(sumVal, absVal);
                }
            }
            difAvg += _mm256_sum_epi32(sumVal);
            difAvg /= (tmpHeight * (tmpWidth - 8));
            int Rate = (255 - difAvg) * 100 / 255;

            if (Rate > maxRate) {
                maxRate = Rate;
                maxStartX = x;
                maxStartY = y;
            }
        }
    }
    if (maxRate > matchingRate) {
        matchPointX = maxStartX;
        matchPointY = maxStartY;
    }
}


