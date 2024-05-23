#pragma once
#include <stdint.h>

void SSEBinarize(uint8_t* src, uint8_t* des, int width, int height, int threshold);

void SSEErosion(uint8_t* src, uint8_t* des, int width, int height, int threshold, int kernelX, int kernelY);

void SSEDilation(uint8_t* src, uint8_t* des, int width, int height, int threshold, int kernelX, int kernelY);

void SSEGaussianBlur(uint8_t* src, uint8_t* des, int width, int height, double* kernel);

void SSEEqualization(uint8_t* src, uint8_t* des, int width, int height);

void SSESobel(uint8_t* src, uint8_t* des, int width, int height);
