#include "pch.h"
#include "SSEImageProcessing.h"
#include <intrin.h>

void SSEBinarize(uint8_t* src, uint8_t* des, int width, int height, int threshold)
{
	int pixelNum = width * height;
	//임계값을 벡터로 변환
	__m256i vecThreshold = _mm256_set1_epi8(threshold);
	__m256i vecMaxValue = _mm256_set1_epi8(255);

	for (int i = 0; i < pixelNum - 32; i += 32) {
		// 픽셀 값을 벡터로 로드
		__m256i pixel = _mm256_loadu_si256((__m256i*) & src[i]);

		__m256i cmpResult = _mm256_cmpgt_epi8(pixel, vecThreshold);

		__m256i result = _mm256_and_si256(cmpResult, vecMaxValue);

		_mm256_storeu_si256((__m256i*) & des[i], result);
	}

	for (int i = pixelNum - (pixelNum % 32); i < pixelNum; i++) {
		des[i] = src[i] > threshold ? 255 : 0;
	}
}

void SSEErosion(uint8_t* src, uint8_t* des, int width, int height, int threshold, int kernelX, int kernelY)
{
	SSEBinarize(src, src, width, height, threshold);

}

void SSEDilation(uint8_t* src, uint8_t* des, int width, int height, int threshold, int kernelX, int kernelY)
{

}
