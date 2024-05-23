#include "pch.h"
#include "SSEImageProcessing.h"
#include <intrin.h>
#include "math.h"

// ��ȣ ���� 8��Ʈ ���� �񱳸� ���� �Լ�
__m256i _mm256_cmpgt_epu8(__m256i a, __m256i b)
{
    __m256i sign = _mm256_set1_epi8(128);
    __m256i subSign_a = _mm256_sub_epi8(a, sign);
    __m256i subSign_b = _mm256_sub_epi8(b, sign);
    return _mm256_cmpgt_epi8(subSign_a, subSign_b);
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
        for (int x = 0; x < width - 4; x += 4) { // 4���� �ȼ��� �� ���� ó��
            // Ŀ���� ���� ���� �ȼ� ���� ���� ���� ������ ����
            __m256d sumVal_X = _mm256_setzero_pd();
            __m256d sumVal_Y = _mm256_setzero_pd();
            __m256d sumVal_Result = _mm256_setzero_pd();
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // �̹��� ���� ���� �ȼ��� ó��
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        // ���� ���Ͱ� �ε�
                        double FilterVal_X = sobelXFilter[ky + halfKernelY][kx + halfKernelX];
                        double FilterVal_Y = sobelYFilter[ky + halfKernelY][kx + halfKernelX];

                        // 4�� �ȼ��� �ε� 
                        __m128i pixel8 = _mm_loadu_si32(& src[ny * width + nx]);
                        // 4���� �ȼ� �����͸� zeroExtend
                        __m256i pixel32 = _mm256_cvtepu8_epi32(pixel8);
                        // ���� ������ �����͸� ������ ����ؾ��� �ȼ� 4���� �ε��Ҽ��������� ����
                        __m256d pixel = _mm256_cvtepi32_pd(_mm256_castsi256_si128(pixel32));
                        // Ŀ�� ���� ����ȭ
                        __m256d filterXVec = _mm256_set1_pd(FilterVal_X);
                        __m256d filterYVec = _mm256_set1_pd(FilterVal_Y);
                        // ���� ��� ���(���� �����÷ο찡 �߻����� �ʴ´� Ȯ���� �����Ƿ� �ٷ� ���)
                        __m256d mulResultX = _mm256_mul_pd(pixel, filterXVec);
                        __m256d mulResultY = _mm256_mul_pd(pixel, filterYVec);
                        // ������ ���
                        sumVal_X = _mm256_add_pd(sumVal_X, mulResultX);
                        sumVal_Y = _mm256_add_pd(sumVal_Y, mulResultY);
                    }
                }
            }
            // X �̺а���� Y�̺� ����� ��ħ
            sumVal_Result = _mm256_add_pd(sumVal_X, sumVal_Y);

            // ����� uint8_t�� ��ȯ�Ͽ� ������ �̹����� ����
            // �ε� �Ҽ��� ���� 32��Ʈ ������ ��ȯ
            __m128i intSumVal128 = _mm256_cvtpd_epi32(sumVal_Result);
            // ��ģ ����� ���밪 ��ȯ
            intSumVal128 = _mm_abs_epi32(intSumVal128);
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
