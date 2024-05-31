#include "pch.h"

#include "CLRImageProcessing.h"
#include <intrin.h>
#include "SSEImageProcessing.h"
#include <malloc.h>
#include <math.h>
using namespace CLRImageProcessing;

double* GetGaussianFilter(int sigma);

Bitmap^ CLRImageProcessing::ImageProcessing::Cvt32GrayTo8Gray(Bitmap^ sourceBitmap)
{
	Bitmap^ newBitmap = gcnew Bitmap(sourceBitmap->Width, sourceBitmap->Height, Imaging::PixelFormat::Format8bppIndexed);

	// �ȷ�Ʈ ���� (�⺻ �׷��̽����� �ȷ�Ʈ ���)
	ColorPalette^ palette = newBitmap->Palette;
	for (int i = 0; i < 256; i++)
	{
		palette->Entries[i] = Color::FromArgb(i, i, i);
	}
	newBitmap->Palette = palette;

	// ���� �̹��� �����͸� ��װ� ����Ʈ �迭�� ��ȯ
	BitmapData^ originalData = sourceBitmap->LockBits(System::Drawing::Rectangle(0, 0, sourceBitmap->Width, sourceBitmap->Height), ImageLockMode::ReadOnly, sourceBitmap->PixelFormat);
	BitmapData^ newBitmapData = newBitmap->LockBits(System::Drawing::Rectangle(0, 0, newBitmap->Width, newBitmap->Height), ImageLockMode::WriteOnly, Imaging::PixelFormat::Format8bppIndexed);

	int bytesPerPixel = System::Drawing::Bitmap::GetPixelFormatSize(sourceBitmap->PixelFormat) / 8;
	int height = sourceBitmap->Height;
	int width = sourceBitmap->Width;

	byte* pixelData = static_cast<byte*>(originalData->Scan0.ToPointer());
	byte* newPixelData = static_cast<byte*>(newBitmapData->Scan0.ToPointer());

	// �ȼ� ������ ��ȯ
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int index32 = y * originalData->Stride + x * bytesPerPixel;
			byte gray = pixelData[index32];

			newPixelData[y * newBitmapData->Stride + x] = gray;
		}
	}

	// �̹��� ������ ��� ����
	sourceBitmap->UnlockBits(originalData);
	newBitmap->UnlockBits(newBitmapData);

	return newBitmap;
}

Bitmap^ CLRImageProcessing::ImageProcessing::Binarize(Bitmap^ sourceBitmap, int threshold)
{	
	Bitmap^ targetBitmap = gcnew Bitmap(sourceBitmap);

		//�⺻ Palette�� �÷��� �� �ֱ� ������ Palette�� �׷��� ��ĳ�� Palette�� ����
	ColorPalette^ palette = sourceBitmap->Palette;
	for (int i = 0; i < 256; i++)
		palette->Entries[i] = Color::FromArgb(i, i, i);
	sourceBitmap->Palette = palette;
	targetBitmap->Palette = palette;

	BitmapData^ sourceBitmapData = sourceBitmap->LockBits(System::Drawing::Rectangle(0, 0, sourceBitmap->Width
		, sourceBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	BitmapData^ targetBitmapData = targetBitmap->LockBits(System::Drawing::Rectangle(0, 0, targetBitmap->Width
		, targetBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	byte* sPtr = static_cast<byte*>(sourceBitmapData->Scan0.ToPointer());
	byte* tPtr = static_cast<byte*>(targetBitmapData->Scan0.ToPointer());

	SSEBinarize(sPtr, tPtr, sourceBitmap->Width, sourceBitmap->Height, threshold);

	sourceBitmap->UnlockBits(sourceBitmapData);
	targetBitmap->UnlockBits(targetBitmapData);

	return targetBitmap;
}

Bitmap^ CLRImageProcessing::ImageProcessing::Erosion(Bitmap^ sourceBitmap, int threshold, int kernelX, int kernelY)
{
	Bitmap^ targetBitmap = gcnew Bitmap(sourceBitmap);

	//�⺻ Palette�� �÷��� �� �ֱ� ������ Palette�� �׷��� ��ĳ�� Palette�� ����
	ColorPalette^ palette = sourceBitmap->Palette;
	for (int i = 0; i < 256; i++)
		palette->Entries[i] = Color::FromArgb(i, i, i);
	sourceBitmap->Palette = palette;
	targetBitmap->Palette = palette;

	BitmapData^ sourceBitmapData = sourceBitmap->LockBits(System::Drawing::Rectangle(0, 0, sourceBitmap->Width
		, sourceBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	BitmapData^ targetBitmapData = targetBitmap->LockBits(System::Drawing::Rectangle(0, 0, targetBitmap->Width
		, targetBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	byte* sPtr = static_cast<byte*>(sourceBitmapData->Scan0.ToPointer());
	byte* tPtr = static_cast<byte*>(targetBitmapData->Scan0.ToPointer());

	SSEErosion(sPtr, tPtr, sourceBitmap->Width, sourceBitmap->Height, threshold, kernelX, kernelY);

	sourceBitmap->UnlockBits(sourceBitmapData);
	targetBitmap->UnlockBits(targetBitmapData);

	return targetBitmap;
}

Bitmap^ CLRImageProcessing::ImageProcessing::Dilation(Bitmap^ sourceBitmap, int threshold, int kernelX, int kernelY)
{
	Bitmap^ targetBitmap = gcnew Bitmap(sourceBitmap);

	//�⺻ Palette�� �÷��� �� �ֱ� ������ Palette�� �׷��� ��ĳ�� Palette�� ����
	ColorPalette^ palette = sourceBitmap->Palette;
	for (int i = 0; i < 256; i++)
		palette->Entries[i] = Color::FromArgb(i, i, i);
	sourceBitmap->Palette = palette;
	targetBitmap->Palette = palette;

	BitmapData^ sourceBitmapData = sourceBitmap->LockBits(System::Drawing::Rectangle(0, 0, sourceBitmap->Width
		, sourceBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	BitmapData^ targetBitmapData = targetBitmap->LockBits(System::Drawing::Rectangle(0, 0, targetBitmap->Width
		, targetBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	byte* sPtr = static_cast<byte*>(sourceBitmapData->Scan0.ToPointer());
	byte* tPtr = static_cast<byte*>(targetBitmapData->Scan0.ToPointer());

	SSEDilation(sPtr, tPtr, sourceBitmap->Width, sourceBitmap->Height, threshold, kernelX, kernelY);

	sourceBitmap->UnlockBits(sourceBitmapData);
	targetBitmap->UnlockBits(targetBitmapData);

	return targetBitmap;
}

Bitmap^ CLRImageProcessing::ImageProcessing::GaussianBlur(Bitmap^ sourceBitmap, int sigma)
{
	Bitmap^ targetBitmap = gcnew Bitmap(sourceBitmap);

	//�⺻ Palette�� �÷��� �� �ֱ� ������ Palette�� �׷��� ��ĳ�� Palette�� ����
	ColorPalette^ palette = sourceBitmap->Palette;
	for (int i = 0; i < 256; i++)
		palette->Entries[i] = Color::FromArgb(i, i, i);
	sourceBitmap->Palette = palette;
	targetBitmap->Palette = palette;

	BitmapData^ sourceBitmapData = sourceBitmap->LockBits(System::Drawing::Rectangle(0, 0, sourceBitmap->Width
		, sourceBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	BitmapData^ targetBitmapData = targetBitmap->LockBits(System::Drawing::Rectangle(0, 0, targetBitmap->Width
		, targetBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	byte* sPtr = static_cast<byte*>(sourceBitmapData->Scan0.ToPointer());
	byte* tPtr = static_cast<byte*>(targetBitmapData->Scan0.ToPointer());

	double* kernel = GetGaussianFilter(sigma);

	SSEGaussianBlur(sPtr, tPtr, sourceBitmap->Width, sourceBitmap->Height, kernel);

	sourceBitmap->UnlockBits(sourceBitmapData);
	targetBitmap->UnlockBits(targetBitmapData);

	delete kernel;
	return targetBitmap;
}

Bitmap^ CLRImageProcessing::ImageProcessing::Equalization(Bitmap^ sourceBitmap)
{
	Bitmap^ targetBitmap = gcnew Bitmap(sourceBitmap);

	//�⺻ Palette�� �÷��� �� �ֱ� ������ Palette�� �׷��� ��ĳ�� Palette�� ����
	ColorPalette^ palette = sourceBitmap->Palette;
	for (int i = 0; i < 256; i++)
		palette->Entries[i] = Color::FromArgb(i, i, i);
	sourceBitmap->Palette = palette;
	targetBitmap->Palette = palette;

	BitmapData^ sourceBitmapData = sourceBitmap->LockBits(System::Drawing::Rectangle(0, 0, sourceBitmap->Width
		, sourceBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	BitmapData^ targetBitmapData = targetBitmap->LockBits(System::Drawing::Rectangle(0, 0, targetBitmap->Width
		, targetBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	byte* sPtr = static_cast<byte*>(sourceBitmapData->Scan0.ToPointer());
	byte* tPtr = static_cast<byte*>(targetBitmapData->Scan0.ToPointer());
	//ó�� ����
	SSEEqualization(sPtr, tPtr, sourceBitmap->Width, sourceBitmap->Height);
	//ó�� ��
	sourceBitmap->UnlockBits(sourceBitmapData);
	targetBitmap->UnlockBits(targetBitmapData);

	return targetBitmap;
}

Bitmap^ CLRImageProcessing::ImageProcessing::Sobel(Bitmap^ sourceBitmap)
{
	Bitmap^ targetBitmap = gcnew Bitmap(sourceBitmap);

	//�⺻ Palette�� �÷��� �� �ֱ� ������ Palette�� �׷��� ��ĳ�� Palette�� ����
	ColorPalette^ palette = sourceBitmap->Palette;
	for (int i = 0; i < 256; i++)
		palette->Entries[i] = Color::FromArgb(i, i, i);
	sourceBitmap->Palette = palette;
	targetBitmap->Palette = palette;

	BitmapData^ sourceBitmapData = sourceBitmap->LockBits(System::Drawing::Rectangle(0, 0, sourceBitmap->Width
		, sourceBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	BitmapData^ targetBitmapData = targetBitmap->LockBits(System::Drawing::Rectangle(0, 0, targetBitmap->Width
		, targetBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	byte* sPtr = static_cast<byte*>(sourceBitmapData->Scan0.ToPointer());
	byte* tPtr = static_cast<byte*>(targetBitmapData->Scan0.ToPointer());
	// width�� 4����Ʈ�� ����� �����ֱ� ���� �е� �߰�
	int padding = (sourceBitmap->Width % 4) == 0 ? 0 : 4 - (sourceBitmap->Width % 4);
	int paddingWidth = sourceBitmap->Width + padding;
	//ó�� ����
	SSESobel(sPtr, tPtr, paddingWidth, sourceBitmap->Height);
	//ó�� ��
	sourceBitmap->UnlockBits(sourceBitmapData);
	targetBitmap->UnlockBits(targetBitmapData);

	return targetBitmap;
}

Bitmap^ CLRImageProcessing::ImageProcessing::Laplacian(Bitmap^ sourceBitmap)
{
	Bitmap^ targetBitmap = gcnew Bitmap(sourceBitmap);

	//�⺻ Palette�� �÷��� �� �ֱ� ������ Palette�� �׷��� ��ĳ�� Palette�� ����
	ColorPalette^ palette = sourceBitmap->Palette;
	for (int i = 0; i < 256; i++)
		palette->Entries[i] = Color::FromArgb(i, i, i);
	sourceBitmap->Palette = palette;
	targetBitmap->Palette = palette;

	BitmapData^ sourceBitmapData = sourceBitmap->LockBits(System::Drawing::Rectangle(0, 0, sourceBitmap->Width
		, sourceBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	BitmapData^ targetBitmapData = targetBitmap->LockBits(System::Drawing::Rectangle(0, 0, targetBitmap->Width
		, targetBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	byte* sPtr = static_cast<byte*>(sourceBitmapData->Scan0.ToPointer());
	byte* tPtr = static_cast<byte*>(targetBitmapData->Scan0.ToPointer());
	// width�� 4����Ʈ�� ����� �����ֱ� ���� �е� �߰�
	int padding = (sourceBitmap->Width % 4) == 0 ? 0 : 4 - (sourceBitmap->Width % 4);
	int paddingWidth = sourceBitmap->Width + padding;
	//ó�� ����
	SSELaplacian(sPtr, tPtr, paddingWidth, sourceBitmap->Height);
	//ó�� ��
	sourceBitmap->UnlockBits(sourceBitmapData);
	targetBitmap->UnlockBits(targetBitmapData);

	return targetBitmap;
}

Bitmap^ CLRImageProcessing::ImageProcessing::FFTransform(Bitmap^ sourceBitmap, int filterSize, bool lowFilterUse)
{
	Bitmap^ targetBitmap = gcnew Bitmap(sourceBitmap);

	//�⺻ Palette�� �÷��� �� �ֱ� ������ Palette�� �׷��� ��ĳ�� Palette�� ����
	ColorPalette^ palette = sourceBitmap->Palette;
	for (int i = 0; i < 256; i++)
		palette->Entries[i] = Color::FromArgb(i, i, i);
	sourceBitmap->Palette = palette;
	targetBitmap->Palette = palette;

	BitmapData^ sourceBitmapData = sourceBitmap->LockBits(System::Drawing::Rectangle(0, 0, sourceBitmap->Width
		, sourceBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	BitmapData^ targetBitmapData = targetBitmap->LockBits(System::Drawing::Rectangle(0, 0, targetBitmap->Width
		, targetBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	byte* sPtr = static_cast<byte*>(sourceBitmapData->Scan0.ToPointer());
	byte* tPtr = static_cast<byte*>(targetBitmapData->Scan0.ToPointer());
	//ó�� ����
	SSEFFTransform(sPtr, tPtr, sourceBitmap->Width, sourceBitmap->Height, filterSize, lowFilterUse);
	//ó�� ��
	sourceBitmap->UnlockBits(sourceBitmapData);
	targetBitmap->UnlockBits(targetBitmapData);

	return targetBitmap;
}

Bitmap^ CLRImageProcessing::ImageProcessing::GetFFTSpectrum(Bitmap^ sourceBitmap)
{
	int spectrumWidth = pow(2, ceil(log2((double)sourceBitmap->Width)));
	int spectrumHeight = pow(2, ceil(log2((double)sourceBitmap->Height)));
	Bitmap^ targetBitmap = gcnew Bitmap(spectrumWidth, spectrumHeight);

	//�⺻ Palette�� �÷��� �� �ֱ� ������ Palette�� �׷��� ��ĳ�� Palette�� ����
	ColorPalette^ palette = sourceBitmap->Palette;
	for (int i = 0; i < 256; i++)
		palette->Entries[i] = Color::FromArgb(i, i, i);
	sourceBitmap->Palette = palette;
	targetBitmap->Palette = palette;

	BitmapData^ sourceBitmapData = sourceBitmap->LockBits(System::Drawing::Rectangle(0, 0, sourceBitmap->Width
		, sourceBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	BitmapData^ targetBitmapData = targetBitmap->LockBits(System::Drawing::Rectangle(0, 0, targetBitmap->Width
		, targetBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	byte* sPtr = static_cast<byte*>(sourceBitmapData->Scan0.ToPointer());
	byte* tPtr = static_cast<byte*>(targetBitmapData->Scan0.ToPointer());
	//ó�� ����
	SSEFFTSpectrum(sPtr, tPtr, sourceBitmap->Width, sourceBitmap->Height);
	//ó�� ��
	sourceBitmap->UnlockBits(sourceBitmapData);
	targetBitmap->UnlockBits(targetBitmapData);

	return targetBitmap;
}

Bitmap^ CLRImageProcessing::ImageProcessing::GetSimilarity(Bitmap^ sourceBitmap, Bitmap^ templeteBitmap)
{
	Bitmap^ targetBitmap = gcnew Bitmap(sourceBitmap);

	//�⺻ Palette�� �÷��� �� �ֱ� ������ Palette�� �׷��� ��ĳ�� Palette�� ����
	ColorPalette^ palette = sourceBitmap->Palette;
	for (int i = 0; i < 256; i++)
		palette->Entries[i] = Color::FromArgb(i, i, i);
	sourceBitmap->Palette = palette;
	targetBitmap->Palette = palette;

	BitmapData^ sourceBitmapData = sourceBitmap->LockBits(System::Drawing::Rectangle(0, 0, sourceBitmap->Width
		, sourceBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	BitmapData^ templeteBitmapData = templeteBitmap->LockBits(System::Drawing::Rectangle(0, 0, templeteBitmap->Width
		, templeteBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	BitmapData^ targetBitmapData = targetBitmap->LockBits(System::Drawing::Rectangle(0, 0, targetBitmap->Width
		, targetBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);

	byte* sPtr = static_cast<byte*>(sourceBitmapData->Scan0.ToPointer());
	byte* tPtr = static_cast<byte*>(targetBitmapData->Scan0.ToPointer());
	byte* templetePtr = static_cast<byte*>(templeteBitmapData->Scan0.ToPointer());
	//ó�� ����

	GetSimilarityImage(sPtr, tPtr, templetePtr, sourceBitmap->Width, sourceBitmap->Height, templeteBitmap->Width, templeteBitmap->Height);
	//ó�� ��
	sourceBitmap->UnlockBits(sourceBitmapData);
	templeteBitmap->UnlockBits(templeteBitmapData);
	targetBitmap->UnlockBits(targetBitmapData);

	return targetBitmap;
}

Point^ CLRImageProcessing::ImageProcessing::TempleteMatching(Bitmap^ sourceBitmap, Bitmap^ templeteBitmap, int matchingRate)
{
	//�⺻ Palette�� �÷��� �� �ֱ� ������ Palette�� �׷��� ��ĳ�� Palette�� ����
	ColorPalette^ palette = sourceBitmap->Palette;
	for (int i = 0; i < 256; i++)
		palette->Entries[i] = Color::FromArgb(i, i, i);
	sourceBitmap->Palette = palette;

	BitmapData^ sourceBitmapData = sourceBitmap->LockBits(System::Drawing::Rectangle(0, 0, sourceBitmap->Width
		, sourceBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	BitmapData^ templeteBitmapData = templeteBitmap->LockBits(System::Drawing::Rectangle(0, 0, templeteBitmap->Width
		, templeteBitmap->Height), ImageLockMode::ReadWrite, sourceBitmap->PixelFormat);
	byte* sPtr = static_cast<byte*>(sourceBitmapData->Scan0.ToPointer());
	byte* templetePtr = static_cast<byte*>(templeteBitmapData->Scan0.ToPointer());
	//ó�� ����
	int matchPointX = -1;
	int matchPointY = -1;
	SSETempleteMatching(sPtr, templetePtr, sourceBitmap->Width, sourceBitmap->Height, templeteBitmap->Width, templeteBitmap->Height, matchingRate, matchPointX, matchPointY);
	Point^ matchPoint = gcnew Point(matchPointX, matchPointY);
	//ó�� ��
	sourceBitmap->UnlockBits(sourceBitmapData);
	templeteBitmap->UnlockBits(templeteBitmapData);

	return matchPoint;
}

double* GetGaussianFilter(int sigma) 
{
	// Ŀ���� ������ 1�� �Ǵ� ���� �̻����� ������� ����
	const int recommendedSize = sigma * 4 * 2 + 1;
	int* weightArr = new int[recommendedSize]();
	int weight = -recommendedSize / 2;
	double* result = new double[recommendedSize * recommendedSize]();

	// ����ġ �迭 EX)) {-3,-2,-1,0,1,2,3}
	for (int i = 0; i < recommendedSize; i++, weight++)
		weightArr[i] = weight;

	// ���콺 �Լ� ���� G(x,y)=1/2(Sigma^2)(PI)(E)^(-(x^2+y^2)/2(Sigma^2))
	double c = 2 * sigma * sigma;
	double sc = 1.0 / (c * Math::PI);

	for (int i = 0; i < recommendedSize; i++)
	{
		for (int j = 0; j < recommendedSize; j++)
		{
			double xValue = weightArr[i];
			double yValue = weightArr[j];

			result[i*recommendedSize + j] = sc * Math::Pow(Math::E, -(xValue * xValue + yValue * yValue) / c);
		}
	}
	
	delete weightArr;

	return result;
}