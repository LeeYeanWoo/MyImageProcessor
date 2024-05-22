#include "pch.h"
#include "SSEImageProcessing.h"
#include <intrin.h>
#include "math.h"

// 부호 없는 8비트 정수 비교를 위한 함수
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
    // 임계값을 벡터로 변환
    __m256i vecThreshold = _mm256_set1_epi8(threshold);

    for (int i = 0; i <= pixelNum - 32; i += 32) {
        // 픽셀 값을 벡터로 로드
        __m256i pixel = _mm256_loadu_si256((__m256i*) & src[i]);

        // 픽셀 값이 임계값보다 크면 0xFF, 그렇지 않으면 0x00으로 설정
        __m256i cmpResult = _mm256_cmpgt_epu8(pixel, vecThreshold);

        // 결과이미지에 저장
        _mm256_storeu_si256((__m256i*) & des[i], cmpResult);
    }

    // 남은 픽셀 처리
    for (int i = pixelNum - (pixelNum % 32); i < pixelNum; i++) {
        des[i] = src[i] > threshold ? 255 : 0;
    }
}

void SSEErosion(uint8_t* src, uint8_t* des, int width, int height, int threshold, int kernelX, int kernelY)
{
    // 이진화 수행
	SSEBinarize(src, src, width, height, threshold);

    int halfKernelX = kernelX / 2;
    int halfKernelY = kernelY / 2;

    // 전체 픽셀 순회
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x += 32) {
            // 커널 영역 순회하며 커널영역에 픽셀중 하나라도 0인경우
            // 대상 픽셀의 값을 0으로 변경하기 위해 커널영역의 최소값을 업데이트 하는 벡터 생성
            __m256i minVal = _mm256_set1_epi8(255);
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // 이미지 영역 안의 픽셀만 처리
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        __m256i pixel = _mm256_loadu_si256((__m256i*) & src[ny * width + nx]);
                        minVal = _mm256_min_epu8(minVal, pixel);
                    }
                }
            }
            // 결과이미지에 저장
            _mm256_storeu_si256((__m256i*) & des[y * width + x], minVal);
        }
    }
}

void SSEDilation(uint8_t* src, uint8_t* des, int width, int height, int threshold, int kernelX, int kernelY)
{
    // 이진화 수행
    SSEBinarize(src, src, width, height, threshold);

    int halfKernelX = kernelX / 2;
    int halfKernelY = kernelY / 2;

    // 전체 픽셀 순회
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x += 32) {
            // 커널 영역 순회하며 커널영역에 픽셀중 하나라도 0이 아닌경우
            // 대상 픽셀의 값을 255으로 변경하기 위해 커널영역의 최소값을 업데이트 하는 벡터 생성
            __m256i maxVal = _mm256_setzero_si256();
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // 이미지 영역 안의 픽셀만 처리
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        __m256i pixel = _mm256_loadu_si256((__m256i*) & src[ny * width + nx]);
                        maxVal = _mm256_max_epu8(maxVal, pixel);
                    }
                }
            }
            // 결과이미지에 저장
            _mm256_storeu_si256((__m256i*) & des[y * width + x], maxVal);
        }
    }
}

void SSEGaussianBlur(uint8_t* src, uint8_t* des, int width, int height, double* kernel) {
    // 커널영역 계산
    int arrSize = _msize(kernel) / sizeof(double);
    int kernelWidth = sqrt(arrSize);

    int halfKernelX = kernelWidth / 2;
    int halfKernelY = kernelWidth / 2;
    
    // 전체 픽셀 순회
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x += 4) {
            // 커널의 값과 인접 픽셀 값을 곱한 값을 저장할 벡터
            __m256d sumVal = _mm256_setzero_pd();

            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // 이미지 영역 안의 픽셀만 처리
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        // 현재 커널의 가중치 로드
                        double kernelVal = kernel[(ky + halfKernelY) * kernelWidth + (kx + halfKernelX)];
                        // 픽셀 로드
                        __m256i pixel32 = _mm256_loadu_si256((__m256i*) & src[ny * width + nx]);
                        // 32개의 벡터에서 4개의 부동소수점 벡터로 변환
                        __m256i pixel16 = _mm256_unpacklo_epi8(pixel32, _mm256_setzero_si256());
                        __m256i pixel8 = _mm256_unpacklo_epi8(pixel16, _mm256_setzero_si256());
                        __m128i pixel4 = _mm256_extracti128_si256(pixel8, 0);
                        __m256d pixel = _mm256_cvtepi32_pd(pixel4);
                        // 커널 값을 벡터화
                        __m256d kernelValVec = _mm256_set1_pd(kernelVal);
                        // 곱 계산(계산결과가 OverFlow가 나지 않는다는 확신이 있기 때문에 바로 계산
                        __m256d mulResult = _mm256_mul_pd(pixel, kernelValVec);
                        
                        // 곱결과를 벡터에 저장

                    }
                }
            }

        }
    }
}
