#include "pch.h"
#include "SSEImageProcessing.h"
#include <intrin.h>
#include "math.h"
#include <complex>
#include <vector>
using namespace std;

// 부호 없는 8비트 정수 비교를 위한 함수
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
        for (int x = 0; x < width - 16; x += 16) { // 4개의 픽셀을 한 번에 처리
            // 커널의 값과 인접 픽셀 값을 곱한 값을 저장할 벡터
            __m256i sumVal_X = _mm256_setzero_si256();
            __m256i sumVal_Y = _mm256_setzero_si256();
            __m256i sumVal_Result = _mm256_setzero_si256();
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // 이미지 영역 안의 픽셀만 처리
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        // 현재 필터값 로드
                        int FilterVal_X = sobelXFilter[ky + halfKernelY][kx + halfKernelX];
                        int FilterVal_Y = sobelYFilter[ky + halfKernelY][kx + halfKernelX];

                        // 16개 픽셀이 로드 
                        __m128i pixel16 = _mm_loadu_si128((__m128i*) & src[ny * width + nx]);
                        // 16개의 픽셀 데이터를 zeroExtend
                        __m256i pixel = _mm256_cvtepu8_epi16(pixel16);
                        // 커널 값을 벡터화
                        __m256i filterXVec = _mm256_set1_epi16(FilterVal_X);
                        __m256i filterYVec = _mm256_set1_epi16(FilterVal_Y);
                        // 곱셈 결과 계산(절대 오버플로우가 발생하지 않는단 확신이 있으므로 바로 계산)
                        __m256i mulResultX = _mm256_mullo_epi16(pixel, filterXVec);
                        __m256i mulResultY = _mm256_mullo_epi16(pixel, filterYVec);
                        // 누적합 계산
                        sumVal_X = _mm256_add_epi16(sumVal_X, mulResultX);
                        sumVal_Y = _mm256_add_epi16(sumVal_Y, mulResultY);
                    }
                }
            }
            // 각 미분 결과를 합침
            sumVal_Result = _mm256_add_epi16(sumVal_X, sumVal_Y);
            // 합친 결과를 절대값 변환
            sumVal_Result = _mm256_abs_epi16(sumVal_Result);
            // 결과 값을 16 bit -> 8bit로 변환 빈공간은 0 Extend
            __m256i packResult = _mm256_packus_epi16(sumVal_Result, _mm256_setzero_si256());
            // 필요한 데이터 추출
            __m128i resultData1 = _mm256_extracti128_si256(packResult, 0);
            __m128i resultData2 = _mm256_extracti128_si256(packResult, 1);
            // 사용할 하위 64bit를 나누어 저장
            _mm_storeu_si64((__m128i*) & des[y * width + x], resultData1);
            _mm_storeu_si64((__m128i*) & des[y * width + x + 8], resultData2);
        }
    }
}

void SSELaplacian(uint8_t* src, uint8_t* des, int width, int height)
{
	// 라플라시안 필터
	double laplacianFilter[3][3] =
	{
		{1,1,1},
		{1,-8,1},
		{1,1,1}
	};
    int filterWidth = 3;

    int halfKernelX = filterWidth / 2;
    int halfKernelY = filterWidth / 2;

    // 전체 픽셀 순회
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width - 16; x += 16) { // 4개의 픽셀을 한 번에 처리
            // 커널의 값과 인접 픽셀 값을 곱한 값을 저장할 벡터
            __m256i sumVal_Result = _mm256_setzero_si256();

            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // 이미지 영역 안의 픽셀만 처리
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        // 현재 필터값 로드
                        int FilterVal = laplacianFilter[ky + halfKernelY][kx + halfKernelX];

                        // 16개 픽셀이 로드 
                        __m128i pixel16 = _mm_loadu_si128((__m128i*) & src[ny * width + nx]);
                        // 16개의 픽셀 데이터를 zeroExtend
                        __m256i pixel = _mm256_cvtepu8_epi16(pixel16);
                        // 커널 값을 벡터화
                        __m256i filterXVec = _mm256_set1_epi16(FilterVal);
                        // 곱셈 결과 계산(절대 오버플로우가 발생하지 않는단 확신이 있으므로 바로 계산)
                        __m256i mulResult = _mm256_mullo_epi16(pixel, filterXVec);
                        // 누적합 계산
                        sumVal_Result = _mm256_add_epi16(sumVal_Result, mulResult);
                    }
                }
            }

            // 합친 결과를 절대값 변환
            sumVal_Result = _mm256_abs_epi16(sumVal_Result);
            // 결과 값을 16 bit -> 8bit로 변환 빈공간은 0 Extend
            __m256i packResult = _mm256_packus_epi16(sumVal_Result, _mm256_setzero_si256());
            // 필요한 데이터 추출
            __m128i resultData1 = _mm256_extracti128_si256(packResult, 0);
            __m128i resultData2 = _mm256_extracti128_si256(packResult, 1);
            // 사용할 하위 64bit를 나누어 저장
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
    // 전체 픽셀 순회
    int maxRate = 0;
    int maxStartX = 0;
    int maxStartY = 0;

    for (int y = 0; y < srcHeight - tmpHeight; y++) {
        for (int x = 0; x < srcWidth - tmpWidth - 8; x++) { // 4개의 픽셀을 한 번에 처리

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


