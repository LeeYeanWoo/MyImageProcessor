#include "pch.h"

#include "CLRImageProcessing.h"
#include <intrin.h>
#include "SSEImageProcessing.h"
using namespace CLRImageProcessing;

Bitmap^ CLRImageProcessing::ImageProcessing::Binarize(Bitmap^ sourceBitmap, int threshold)
{	
	Bitmap^ targetBitmap = gcnew Bitmap(sourceBitmap);

	BitmapData^ sourceBitmapData = sourceBitmap->LockBits(System::Drawing::Rectangle(0, 0, sourceBitmap->Width
		, sourceBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);

	BitmapData^ targetBitmapData = targetBitmap->LockBits(System::Drawing::Rectangle(0, 0, targetBitmap->Width
		, targetBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	
	byte* sPtr = static_cast<byte*>(sourceBitmapData->Scan0.ToPointer());

	byte* tPtr = static_cast<byte*>(targetBitmapData->Scan0.ToPointer());

	//기본 Palette가 컬러로 들어가 있기 때문에 Palette를 그레이 스캐일 Palette로 변경
	ColorPalette^ palette = sourceBitmap->Palette;
	for (int i = 0; i < 256; i++)
		palette->Entries[i] = Color::FromArgb(i, i, i);

	sourceBitmap->Palette = palette;
	targetBitmap->Palette = palette;

	//이미지 처리 시작
	SSEBinarize(sPtr, tPtr, sourceBitmap->Width, sourceBitmap->Height, threshold);

	//이미지 처리 끝
	sourceBitmap->UnlockBits(sourceBitmapData);

	targetBitmap->UnlockBits(targetBitmapData);

	return targetBitmap;
}

