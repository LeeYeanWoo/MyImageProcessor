#include "pch.h"
#include "SSEImageProcessing.h"
#include <intrin.h>

__m256i _mm256_cmpgt_epu8(__m256i a, __m256i b)
{
    // 부호 없는 8비트 정수 비교
    __m256i sign_bit = _mm256_set1_epi8(0x80); // 0x80 = 128
    __m256i a_signed = _mm256_sub_epi8(a, sign_bit);
    __m256i b_signed = _mm256_sub_epi8(b, sign_bit);
    return _mm256_cmpgt_epi8(a_signed, b_signed);
}

void SSEBinarize(uint8_t* src, uint8_t* des, int width, int height, int threshold)
{
    int pixelNum = width * height;
    // 임계값을 벡터로 변환
    __m256i vecThreshold = _mm256_set1_epi8(threshold);
    __m256i vecMaxValue = _mm256_set1_epi8(255);

    for (int i = 0; i <= pixelNum - 32; i += 32) {
        // 픽셀 값을 벡터로 로드
        __m256i pixel = _mm256_loadu_si256((__m256i*) & src[i]);

        // 픽셀 값이 임계값보다 크면 0xFF, 그렇지 않으면 0x00으로 설정
        __m256i cmpResult = _mm256_cmpgt_epu8(pixel, vecThreshold);

        // 결과 값을 0xFF로 설정 (255)
        __m256i result = _mm256_and_si256(cmpResult, vecMaxValue);

        _mm256_storeu_si256((__m256i*) & des[i], result);
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

    // SIMD로 침식 알고리즘 적용
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x += 16) {
            // 현재 픽셀 위치에서 주변 픽셀의 최소값 찾기
            __m128i minVal = _mm_set1_epi8(255);
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // 이미지 경계를 벗어나지 않도록 처리
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        __m128i pixel = _mm_loadu_si128((__m128i*) & src[ny * width + nx]);
                        minVal = _mm_min_epu8(minVal, pixel);
                    }
                }
            }
            // 최소값을 결과 배열에 저장
            _mm_storeu_si128((__m128i*) & des[y * width + x], minVal);
        }
    }

}

void SSEDilation(uint8_t* src, uint8_t* des, int width, int height, int threshold, int kernelX, int kernelY)
{
    // 이진화 수행
    SSEBinarize(src, src, width, height, threshold);

    int halfKernelX = kernelX / 2;
    int halfKernelY = kernelY / 2;

    // SIMD로 팽창 알고리즘 적용
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x += 16) {
            // 현재 픽셀 위치에서 주변 픽셀의 최대값 찾기
            __m128i maxVal = _mm_setzero_si128();
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // 이미지 경계를 벗어나지 않도록 처리
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        __m128i pixel = _mm_loadu_si128((__m128i*) & src[ny * width + nx]);
                        maxVal = _mm_max_epu8(maxVal, pixel);
                    }
                }
            }
            // 최대값을 결과 배열에 저장
            _mm_storeu_si128((__m128i*) & des[y * width + x], maxVal);
        }
    }
}
