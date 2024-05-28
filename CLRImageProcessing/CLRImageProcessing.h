#pragma once
#include<vcclr.h>
#include<Windows.h>

using namespace System;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;

namespace CLRImageProcessing {
	public ref class ImageProcessing
	{
	public:
		Bitmap^ Binarize(Bitmap^sourceBitmap , int threshold);
		Bitmap^ Erosion(Bitmap^ sourceBitmap, int threshold, int kernelX, int kernelY);
		Bitmap^ Dilation(Bitmap^ sourceBitmap, int threshold, int kernelX, int kernelY);
		Bitmap^ GaussianBlur(Bitmap^ sourceBitmap, int sigma);
		Bitmap^ Equalization(Bitmap^ sourceBitmap);
		Bitmap^ Sobel(Bitmap^ sourceBitmap);
		Bitmap^ Laplacian(Bitmap^ sourceBitmap);
		Point^ TempleteMatching(Bitmap^ sourceBitmap, Bitmap^ templeteBitmap, int matchingRate);
	};
}
