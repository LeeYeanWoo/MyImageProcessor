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
    // 커널 크기 계산
    int arrSize = _msize(kernel) / sizeof(double);
    int kernelWidth = static_cast<int>(sqrt(arrSize));

    int halfKernelX = kernelWidth / 2;
    int halfKernelY = kernelWidth / 2;

    // 전체 픽셀 순회
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width - 4; x += 4) { // 4개의 픽셀을 한 번에 처리
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
                        // 4개의 픽셀 값 로드 및 변환
                        // 4개 픽셀이 로드 
                        __m128i pixel8 = _mm_loadu_si32(&src[ny * width + nx]);
                        // 4개의 픽셀 데이터를 zeroExtend
                        __m256i pixel32 = _mm256_cvtepu8_epi32(pixel8);
                        // 상위 쓰레기 데이터를 버리고 계산해야할 픽셀 4개를 부동소수형식으로 변경
                        __m256d pixel = _mm256_cvtepi32_pd(_mm256_castsi256_si128(pixel32));
                        // 커널 값을 벡터화
                        __m256d kernelValVec = _mm256_set1_pd(kernelVal);
                        // 곱셈 결과 계산(절대 오버플로우가 발생하지 않는단 확신이 있으므로 바로 계산)
                        __m256d mulResult = _mm256_mul_pd(pixel, kernelValVec);
                        // 누적합 계산
                        sumVal = _mm256_add_pd(sumVal, mulResult);
                    }
                }
            }

            // 결과를 uint8_t로 변환하여 목적지 이미지에 저장
            // 부동 소수점 값을 32비트 정수로 변환
            __m128i intSumVal128 = _mm256_cvtpd_epi32(sumVal); 
            // __m128i -> __m256i로 zero Expend 캐스팅
            __m256i intSumVal = _mm256_castsi128_si256(intSumVal128);
            // 데이터를 32bit -> 16bit으로 변환하기위해 zero data와 패킹
            __m128i packedSumVal = _mm_packus_epi32(_mm256_extracti128_si256(intSumVal, 0), _mm256_extracti128_si256(intSumVal, 1));
            // 데이터를 16bit -> 8bit으로 변환하고 상위데이터는 사용하지 않을 예정이므로 자기자신과 패킹
            packedSumVal = _mm_packus_epi16(packedSumVal, packedSumVal);
            // 사용할 하위 32bit 저장
            _mm_storeu_si32((__m128i*) & des[y * width + x], packedSumVal);
        }
    }
}

void SSEEqualization(uint8_t* src, uint8_t* des, int width, int height)
{
    int pixelNum = width * height;
    int pixelValCntCumulSum[256] = {0};

    // 1. 전체 픽셀의 그레이 값의 카운트 구하기
    for (int i = 0; i < pixelNum; i++)
    {
        uint8_t curPixelVal = src[i * sizeof(uint8_t)];
        pixelValCntCumulSum[curPixelVal]++;
    }

    // 2. 누적합 및 평활화 상수 계산
    // 평활화 상수 = 누적합 / 총 픽셀 갯수 * MAX 픽셀 값
    for (int i = 1; i < 256; i++)
    {
        pixelValCntCumulSum[i] += pixelValCntCumulSum[i - 1];
        double tempData = (double)pixelValCntCumulSum[i-1] / pixelNum * 255.0;
        pixelValCntCumulSum[i - 1] = floor(tempData + 0.5);
    }
    double tempData = (double)pixelValCntCumulSum[255] / pixelNum * 255.0;
    pixelValCntCumulSum[255] = floor(tempData + 0.5);

    // 3. 픽셀 맵핑
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            des[y * width + x] = (uint8_t)pixelValCntCumulSum[src[y * width + x]];
        }
    }
}

void SSESobel(uint8_t* src, uint8_t* des, int width, int height)
{
    //y축 편미분 필터
    double sobelYFilter[3][3] = 
    {
        {-1,-2,-1},
        {0 ,0 ,0},
        {1 ,2 ,1}
    };
    //x축 편미분 필터
    double sobelXFilter[3][3] =
    {
        {-1 ,0 ,1},
        {-2 ,0 ,2},
        {-1 ,0 ,1}
    };
    int filterWidth = 3;

    int halfKernelX = filterWidth / 2;
    int halfKernelY = filterWidth / 2;

    // 전체 픽셀 순회
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width - 4; x += 4) { // 4개의 픽셀을 한 번에 처리
            // 커널의 값과 인접 픽셀 값을 곱한 값을 저장할 벡터
            __m256d sumVal_X = _mm256_setzero_pd();
            __m256d sumVal_Y = _mm256_setzero_pd();
            __m256d sumVal_Result = _mm256_setzero_pd();
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // 이미지 영역 안의 픽셀만 처리
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        // 현재 필터값 로드
                        double FilterVal_X = sobelXFilter[ky + halfKernelY][kx + halfKernelX];
                        double FilterVal_Y = sobelYFilter[ky + halfKernelY][kx + halfKernelX];

                        // 4개 픽셀이 로드 
                        __m128i pixel8 = _mm_loadu_si32(& src[ny * width + nx]);
                        // 4개의 픽셀 데이터를 zeroExtend
                        __m256i pixel32 = _mm256_cvtepu8_epi32(pixel8);
                        // 상위 쓰레기 데이터를 버리고 계산해야할 픽셀 4개를 부동소수형식으로 변경
                        __m256d pixel = _mm256_cvtepi32_pd(_mm256_castsi256_si128(pixel32));
                        // 커널 값을 벡터화
                        __m256d filterXVec = _mm256_set1_pd(FilterVal_X);
                        __m256d filterYVec = _mm256_set1_pd(FilterVal_Y);
                        // 곱셈 결과 계산(절대 오버플로우가 발생하지 않는단 확신이 있으므로 바로 계산)
                        __m256d mulResultX = _mm256_mul_pd(pixel, filterXVec);
                        __m256d mulResultY = _mm256_mul_pd(pixel, filterYVec);
                        // 누적합 계산
                        sumVal_X = _mm256_add_pd(sumVal_X, mulResultX);
                        sumVal_Y = _mm256_add_pd(sumVal_Y, mulResultY);
                    }
                }
            }
            // X 미분결과와 Y미분 결과를 합침
            sumVal_Result = _mm256_add_pd(sumVal_X, sumVal_Y);

            // 결과를 uint8_t로 변환하여 목적지 이미지에 저장
            // 부동 소수점 값을 32비트 정수로 변환
            __m128i intSumVal128 = _mm256_cvtpd_epi32(sumVal_Result);
            // 합친 결과를 절대값 변환
            intSumVal128 = _mm_abs_epi32(intSumVal128);
            // __m128i -> __m256i로 zero Expend 캐스팅
            __m256i intSumVal = _mm256_castsi128_si256(intSumVal128);
            // 데이터를 32bit -> 16bit으로 변환하기위해 zero data와 패킹
            __m128i packedSumVal = _mm_packus_epi32(_mm256_extracti128_si256(intSumVal, 0), _mm256_extracti128_si256(intSumVal, 1));
            // 데이터를 16bit -> 8bit으로 변환하고 상위데이터는 사용하지 않을 예정이므로 자기자신과 패킹
            packedSumVal = _mm_packus_epi16(packedSumVal, packedSumVal);
            // 사용할 하위 32bit 저장
            _mm_storeu_si32((__m128i*) & des[y * width + x], packedSumVal);
        }
    }
}
