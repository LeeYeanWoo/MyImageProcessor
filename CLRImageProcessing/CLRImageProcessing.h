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
	};
}
