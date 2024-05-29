#pragma once
#include <stdint.h>

void SSEBinarize(uint8_t* src, uint8_t* des, int width, int height, int threshold);

void SSEErosion(uint8_t* src, uint8_t* des, int width, int height, int threshold, int kernelX, int kernelY);

void SSEDilation(uint8_t* src, uint8_t* des, int width, int height, int threshold, int kernelX, int kernelY);

void SSEGaussianBlur(uint8_t* src, uint8_t* des, int width, int height, double* kernel);

void SSEEqualization(uint8_t* src, uint8_t* des, int width, int height);

void SSESobel(uint8_t* src, uint8_t* des, int width, int height);

void SSELaplacian(uint8_t* src, uint8_t* des, int width, int height);

void SSEFFTransform(uint8_t* src, uint8_t* des, int width, int height, int filterSize, bool lowFilterUse);

void SSEFFTSpectrum(uint8_t* src, uint8_t* des, int width, int height);

void SSETempleteMatching(uint8_t* src, uint8_t* templete, int srcWidth, int srcHeight, int tmpWidth, int tmpHeight, int matchingRate ,int& matchPointX, int& matchPointY);