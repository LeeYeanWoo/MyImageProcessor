#include "pch.h"
#include "SSEImageProcessing.h"
#include <intrin.h>
#include "math.h"
#include <complex>
#include <vector>
using namespace std;

void GetSpectrumData(vector<vector<complex<double>>>& data);

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
    for (int i = 0; i < 8; i++) {
        sumResult += a.m256i_i32[i];
    }

    return sumResult;
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
        int remainX = 0;
        for (int x = 0; x < width - 32; x += 32) {
            remainX = x;
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
        // 병렬 처리 불가능한 남은 픽셀 처리
        for (int i = remainX; i < width; i++) {
            int min = 255;
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = i + kx;

                    // 이미지 영역 안의 픽셀만 처리
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        uint8_t pixel = src[ny * width + nx];
                        min = min < (int)pixel ? min : (int)pixel;
                    }

                }
            }
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
        int remainX = 0;
        for (int x = 0; x < width - 32; x += 32) {
            remainX = x;
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
        // 병렬 처리 불가능한 남은 픽셀 처리
        for (int i = remainX; i < width; i++) {
            int max = 0;
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = i + kx;

                    // 이미지 영역 안의 픽셀만 처리
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        uint8_t pixel = src[ny * width + nx];
                        max = max > (int)pixel ? max : (int)pixel;
                    }

                }
            }
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
        int remainX = 0;
        for (int x = 0; x < width - halfKernelX - 4; x += 4) { // 4개의 픽셀을 한 번에 처리
            remainX = x;
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
        // 병렬 처리 불가능한 남은 픽셀 처리
        for (int i = remainX; i < width; i++) {
            double sumResult = 0;
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = i + kx;
                    // 이미지 영역 안의 픽셀만 처리
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        // 현재 커널의 가중치 로드
                        double kernelVal = kernel[(ky + halfKernelY) * kernelWidth + (kx + halfKernelX)];
                        // 픽셀 로드 
                        uint8_t pixel = src[ny * width + nx];
                        // 계산
                        sumResult += kernelVal * (double)pixel;
                    }
                }
            }
            sumResult = ceil(sumResult);
            des[y * width + i] = (uint8_t)sumResult;
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
    int sobelYFilter[3][3] = 
    {
        {-1,-2,-1},
        {0 ,0 ,0},
        {1 ,2 ,1}
    };
    //x축 편미분 필터
    int sobelXFilter[3][3] =
    {
        {-1 ,0 ,1},
        {-2 ,0 ,2},
        {-1 ,0 ,1}
    };
    int filterWidth = 3;

    int halfKernelX = filterWidth / 2;
    int halfKernelY = filterWidth / 2;

    // 전체 픽셀 순회
    for (int y = 1; y < height; y++) {
        int remainX = 0;
        for (int x = 1; x < width -16; x += 16) { // 4개의 픽셀을 한 번에 처리
            // 남은 픽셀의 start 지점 갱신
            remainX = x;
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
        // 병렬 처리 불가능한 남은 픽셀 처리
        for (int i = remainX; i < width; i++) {
            int sumX = 0, sumY = 0, sumResult = 0;
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = i + kx;

                    // 이미지 영역 안의 픽셀만 처리
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        // 현재 필터값 로드
                        int FilterVal_X = sobelXFilter[ky + halfKernelY][kx + halfKernelX];
                        int FilterVal_Y = sobelYFilter[ky + halfKernelY][kx + halfKernelX];
                        // 픽셀 로드 
                        uint8_t pixel = src[ny * width + nx];
                        // 계산
                        sumX += (FilterVal_X * pixel);
                        sumY += (FilterVal_Y * pixel);
                    }
                }
            }
            // X 축 미분 결과 Y축 미분 결과를 더하여 절대값 처리
            sumResult = abs(sumX + sumY) > 255 ? 255 : abs(sumX + sumY);
            des[y * width + i] = (uint8_t)sumResult;
        }
    }
}

void SSELaplacian(uint8_t* src, uint8_t* des, int width, int height)
{
	// 라플라시안 필터
    int laplacianFilter[3][3] =
	{
		{1,1,1},
		{1,-8,1},
		{1,1,1}
	};
    int filterWidth = 3;

    int halfKernelX = filterWidth / 2;
    int halfKernelY = filterWidth / 2;

    // 전체 픽셀 순회
    for (int y = 1; y < height; y++) {
        int remainX = 0;
        for (int x = 1; x < width - 16; x += 16) { // 16개의 픽셀을 한 번에 처리
            // 남은 픽셀의 start 지점 갱신
            remainX = x;
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
        // 병렬 처리 불가능한 남은 픽셀 처리
        for (int i = remainX; i < width; i++) {
            int sumResult = 0;
            for (int ky = -halfKernelY; ky <= halfKernelY; ky++) {
                for (int kx = -halfKernelX; kx <= halfKernelX; kx++) {
                    int ny = y + ky;
                    int nx = i + kx;
                    // 이미지 영역 안의 픽셀만 처리
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        // 현재 필터값 로드
                        int FilterVal = laplacianFilter[ky + halfKernelY][kx + halfKernelX];
                        // 픽셀 로드 
                        uint8_t pixel = src[ny * width + nx];
                        // 계산
                        sumResult += (FilterVal * pixel);
                    }
                }
            }
            sumResult = abs(sumResult) > 255 ? 255 : abs(sumResult);
            des[y * width + i] = (uint8_t)sumResult;
        }
    }
}

// 1차원 FFT
void FFT(vector<complex<double>>& x, bool inverse) {
    const double pi = System::Math::PI;
    // N은 2의 거듭제곱 형식으로 나타나야함
    int N = x.size();  

    // 수열 데이터 반전 실행
    for (int i = 1, rev = 0; i < N; ++i) {
        int bit = N >> 1;
        while (rev >= bit) {
            rev -= bit;
            bit >>= 1;
        }
        rev += bit;
        if (i < rev) {
            swap(x[i], x[rev]);
        }
    }

    // FFT 실행
    for (int len = 2; len <= N; len <<= 1) {
        //e^i2(pi)ux = cos(2*pi*ux) + i sin(2*pi*ux)
        double angle = 2.0 * pi / len * (inverse ? -1 : 1);
        // Wlen = e^(i*angle)
        complex<double> Wlen(cos(angle), sin(angle)); 
        // Wlen을 주기가 같은 위치에 연산
        for (int i = 0; i < N; i += len) {
            complex<double> W(1);
            for (int j = 0; j < len / 2; ++j) {
                complex<double> even = x[i + j];
                complex<double> odd = x[i + j + len / 2] * W;
                // x[i + j] 는 인덱스 스왑 전 짝수 번쨰 인덱스
                // x[i + j + len / 2]는 인덱스 스왑전 x[i + j]의 인덱스 + 1 인 홀수 번쨰 인덱스
                //  A(k + N/2) = 홀수부를 음수로 처리한 것과 동일 
                x[i + j] = even + odd;
                x[i + j + len / 2] = even - odd;
                W *= Wlen;
            }
        }
    }

    // 역FFT시 사용
    if (inverse) {
        for (auto& val : x) {
            val /= N;
        }
    }
}

// 2차원 FFT
void FFT2D(vector<vector<complex<double>>>& data, bool inverse) {
    int rows = data.size();
    int cols = data[0].size();

    // 행단위 FFT 실행
    for (int i = 0; i < rows; ++i) {
        FFT(data[i], inverse);
    }

    // 열단위 FFT 실행
    for (int j = 0; j < cols; ++j) {
        vector<complex<double>> column(rows);
        for (int i = 0; i < rows; ++i) {
            column[i] = data[i][j];
        }
        FFT(column, inverse);
        for (int i = 0; i < rows; ++i) {
            data[i][j] = column[i];
        }
    }
}

// 저역 통과 필터
void LowPassFilter(vector<vector<complex<double>>>& data, int width, int height, int filterSize) {
    for (int u = 0; u < height; ++u) {
        for (int v = 0; v < width; ++v) {
            if ((u > filterSize && u < height - filterSize) || (v > filterSize && v < width - filterSize)) {
                data[u][v] = 0;
            }
        }
    }
}

// 고역 통과 필터
void HighPassFilter(vector<vector<complex<double>>>& data, int width, int height, int filterSize) {
    for (int u = 0; u < height; ++u) {
        for (int v = 0; v < width; ++v) {
            if (u > (height / 2 - filterSize) && u < (height / 2 + filterSize) && v >(width / 2 - filterSize) && v < (width / 2 + filterSize)) {
            }
            else
            {
                data[u][v] = 0;
            }
        }
    }
}

void SSEFFTransform(uint8_t* src, uint8_t* des, int width, int height, int filterSize, bool lowFilterUse)
{
    // 2의 거듭 제곱 형식으로 변환
    int FFTWidth = pow(2, ceil(log2((double)width)));
    int FFTHeight = pow(2, ceil(log2((double)height)));
    
    // 이미지 데이터를 복소수 형태로 변환
    vector<vector<complex<double>>> data(FFTHeight, vector<complex<double>>(FFTWidth));
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            data[i][j] = complex<double>(src[i * width + j], 0.0);
        }
    }

    // 2D FFT 변환
    FFT2D(data, false);

    // 필터 적용
    if(lowFilterUse)
        LowPassFilter(data, width, height, filterSize);
    else
        HighPassFilter(data, width, height, filterSize);

    // 2D IFFT 변환
    FFT2D(data, true);

    // 결과를 다시 이미지 데이터로 변환
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            des[i * width + j] = static_cast<uint8_t>(round(real(data[i][j])));
        }
    }
}

void SSEFFTSpectrum(uint8_t* src, uint8_t* des, int width, int height)
{
    // 2의 거듭 제곱 형식으로 변환
    int FFTWidth = pow(2, ceil(log2((double)width)));
    int FFTHeight = pow(2, ceil(log2((double)height)));

    // 이미지 데이터를 복소수 형태로 변환
    vector<vector<complex<double>>> data(FFTHeight, vector<complex<double>>(FFTWidth));
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            data[i][j] = complex<double>(src[i * width + j], 0.0);
        }
    }

    // 2D FFT 변환
    FFT2D(data, false);

    GetSpectrumData(data);

    // 결과를 복원하지 않고 스펙트럼 이미지로 데이터로 변환
    for (int i = 0; i < FFTHeight; ++i) {
        for (int j = 0; j < FFTWidth; ++j) {
            des[i * FFTWidth + j] = static_cast<uint8_t>(round(real(data[i][j])));
        }
    }

    // 명암 대비를 위해 평활화 실행
    SSEEqualization(des, des, FFTWidth, FFTHeight);
}

void GetSpectrumData(vector<vector<complex<double>>>& data) {
    int width = data[0].size();
    int height = data.size();

    // 스펙트럼 변환 공식(F(u,v) = [R(u,v)^2 + I(u,v)^2]^1/2) 적용
    // log 스케일 전환
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            double temp = 1 + (sqrt(data[i][j].real() * data[i][j].real() + data[i][j].imag() * data[i][j].imag()));
            int logData = 20 * log(temp);
            data[i][j] = logData > 255 ? 255 : logData;
        }
    }
    vector<vector<complex<double>>> copyData(data);

    // 1사분면 - 3사분면 Swap
    // 1사분면
    for (int i = 0; i < height / 2; i++)
    {
        for (int j = 0; j < data[0].size() / 2; j++)
        {
            data[i][j] = copyData[i + (height / 2)][j + (width / 2)];
        }
    }
    // 3사분면
    for (int i = height / 2; i < height; i++)
    {
        for (int j = width / 2; j < width; j++)
        {
            data[i][j] = copyData[i - (height / 2)][j - (width / 2)];
        }
    }
    // 2사분면 - 4사분면 Swap
    // 2사분면
    for (int i = 0; i < height / 2; i++)
    {
        for (int j = width / 2; j < width; j++)
        {
            data[i][j] = copyData[i + (height / 2)][j - (width / 2)];
        }
    }
    // 4사분면
    for (int i = height / 2; i < height; i++)
    {
        for (int j = 0; j < width / 2; j++)
        {
            data[i][j] = copyData[i - (height / 2)][j + (width / 2)];
        }
    }
}

void SSETempleteMatching(uint8_t* src, uint8_t* templete, int srcWidth, int srcHeight, int tmpWidth, int tmpHeight, int matchingRate, int& matchPointX, int& matchPointY)
{
    // 전체 픽셀 순회
    int maxRate = 0;
    int maxStartX = 0;
    int maxStartY = 0;

    for (int y = 0; y < srcHeight - tmpHeight; y++) {
        for (int x = 0; x < srcWidth - tmpWidth - 8; x++) { // 8개의 픽셀을 한 번에 처리

            //합을 누적할 레지스터 선언
            __m256i sumVal = _mm256_setzero_si256();
            int difAvg = 0;
            // 템플릿 크기만큼 순회
            for (int ky = 0; ky < tmpHeight; ky++) {
                for (int kx = 0; kx < tmpWidth - 8; kx += 8) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // 8바이트 8픽셀 단위로 소스 이미지 로드
                    __m128i srcPixel8 = _mm_loadu_si64((__m128i*) & src[ny * srcWidth + nx]);
                    // 로드된 8바이트 8픽셀을 32비트 signed Intager로 변환
                    __m256i srcPixel = _mm256_cvtepu8_epi32(srcPixel8);
                    // 8바이트 8픽셀 단위로 템플릿 이미지 로드
                    __m128i tmpPixel8 = _mm_loadu_si64((__m128i*) & templete[ky * tmpWidth + kx]);
                    // 로드된 8바이트 8픽셀을 32비트 signed Intager로 변환
                    __m256i tmpPixel = _mm256_cvtepu8_epi32(tmpPixel8);
                    // 8개의 픽셀값이 든 레지스터 두개의 각 픽셀의 차이를 계산
                    __m256i subVal = _mm256_sub_epi32(srcPixel, tmpPixel);
                    // 픽셀의 차이를 제곱
                    __m256i mulVal = _mm256_mullo_epi32(subVal, subVal);
                    // 픽셀의 차이를 sumVal에 누적
                    sumVal = _mm256_add_epi32(sumVal, mulVal);
                }
            }
            // 픽셀값의 차이의 제곱 평균을 구하기
            difAvg = _mm256_sum_epi32(sumVal);
            difAvg /= (tmpHeight * (tmpWidth - 8));
            difAvg = sqrt(difAvg);
            int Rate = (255 - difAvg) * 100 / 255;

            // 현재 이미지가 템플릿과의 매칭율이 가장 높은 매칭일 경우 업데이트
            if (Rate > maxRate) {
                maxRate = Rate;
                maxStartX = x;
                maxStartY = y;
            }
        }
    }
    // 가장 높은 매칭율이 파라미터로 정한 매칭율보다 높은 경우 포인트 정보 입력
    if (maxRate > matchingRate) {
        matchPointX = maxStartX;
        matchPointY = maxStartY;
    }
}

void GetSimilarityImage(uint8_t* src, uint8_t* des, uint8_t* templete, int srcWidth, int srcHeight, int tmpWidth, int tmpHeight)
{
    // 전체 픽셀 순회
    int maxRate = 0;
    int maxStartX = 0;
    int maxStartY = 0;

    for (int y = 0; y < srcHeight; y++) {
        for (int x = 0; x < srcWidth - 8; x++) { // 8개의 픽셀을 한 번에 처리

            //합을 누적할 레지스터 선언
            __m256i sumVal = _mm256_setzero_si256();
            int difAvg = 0;
            // 템플릿 크기만큼 순회
            for (int ky = 0; ky < tmpHeight; ky++) {
                for (int kx = 0; kx < tmpWidth - 8; kx += 8) {
                    int ny = y + ky;
                    int nx = x + kx;
                    // 이미지 영역 안의 픽셀만 처리
                    if (ny >= 0 && ny < srcHeight && nx >= 0 && nx < srcWidth) {

                        // 8바이트 8픽셀 단위로 소스 이미지 로드
                        __m128i srcPixel8 = _mm_loadu_si64((__m128i*) & src[ny * srcWidth + nx]);
                        // 로드된 8바이트 8픽셀을 32비트 signed Intager로 변환
                        __m256i srcPixel = _mm256_cvtepu8_epi32(srcPixel8);
                        // 8바이트 8픽셀 단위로 템플릿 이미지 로드
                        __m128i tmpPixel8 = _mm_loadu_si64((__m128i*) & templete[ky * tmpWidth + kx]);
                        // 로드된 8바이트 8픽셀을 32비트 signed Intager로 변환
                        __m256i tmpPixel = _mm256_cvtepu8_epi32(tmpPixel8);
                        // 8개의 픽셀값이 든 레지스터 두개의 각 픽셀의 차이를 계산
                        __m256i subVal = _mm256_sub_epi32(srcPixel, tmpPixel);
                        // 픽셀의 차이를 제곱
                        __m256i mulVal = _mm256_mullo_epi32(subVal, subVal);
                        // 픽셀의 차이를 sumVal에 누적
                        sumVal = _mm256_add_epi32(sumVal, mulVal);
                    }
                }
            }
            // 픽셀값의 차이의 제곱 평균을 구하기
            difAvg = _mm256_sum_epi32(sumVal);
            difAvg /= (tmpHeight * (tmpWidth - 8));
            difAvg = sqrt(difAvg);
            int Rate = (255 - difAvg) * 100 / 255;

            des[y * srcWidth + x] = (uint8_t)Rate;
        }
    }

    // 명암 대비를 위해 평활화 실행
    SSEEqualization(des, des, srcWidth, srcHeight);
}


