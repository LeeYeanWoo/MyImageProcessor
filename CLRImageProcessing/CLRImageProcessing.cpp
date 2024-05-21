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

	//�⺻ Palette�� �÷��� �� �ֱ� ������ Palette�� �׷��� ��ĳ�� Palette�� ����
	ColorPalette^ palette = sourceBitmap->Palette;
	for (int i = 0; i < 256; i++)
		palette->Entries[i] = Color::FromArgb(i, i, i);

	sourceBitmap->Palette = palette;
	targetBitmap->Palette = palette;

	//�̹��� ó�� ����
	SSEBinarize(sPtr, tPtr, sourceBitmap->Width, sourceBitmap->Height, threshold);

	//�̹��� ó�� ��
	sourceBitmap->UnlockBits(sourceBitmapData);

	targetBitmap->UnlockBits(targetBitmapData);

	return targetBitmap;
}

