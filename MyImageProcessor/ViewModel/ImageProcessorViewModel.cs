using CLRImageProcessing;
using Microsoft.WindowsAPICodePack.Dialogs;
using MyImageProcessor.Model;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using ImageConverter = MyImageProcessor.Model.ImageConverter;

namespace MyImageProcessor.ViewModel
{
    class ImageProcessorViewModel
    {
        private ImageProcessing imageProcessing;
        Stopwatch stopWatch;

        public ImageModel SourceImage { get; set; }

        public ImageModel TargetImage { get; set; }

        public ImageModel PreViewImage { get; set; }

        public ImageProcessorViewModel()
        {
            SourceImage = new();
            TargetImage = new();
            PreViewImage = new();
            imageProcessing = new();
            stopWatch = new();
            LogManager.WriteLog($"프로그램 시작");
        }

        public void ClearData()
        {
            SourceImage.ImageLoad(@"..\..\Image\emptyImage.jpg");
            TargetImage.ImageLoad(@"..\..\Image\emptyImage.jpg");
            PreViewImage.ImageLoad(@"..\..\Image\emptyImage.jpg");
        }

        public void SourceImageLoad()
        {
            OpenFileDialog dlgOpenFile = new();
            dlgOpenFile.Filter = "Image Files (*.jpg, *.png, *.bmp) | *.jpg; *.png; *.bmp; | All files (*.*) | *.*";

            if (dlgOpenFile.ShowDialog().ToString() == "OK")
            {
                SourceImage.ImageLoad(dlgOpenFile.FileName);
                PreViewImage.ImageLoad(dlgOpenFile.FileName);
                LogManager.WriteLog($"{dlgOpenFile.FileName} 이미지 불러오기 성공!");
            }
        }
        public void TargetImageSave()
        {
            CommonOpenFileDialog dlgOpenFile = new();

            if (dlgOpenFile.ShowDialog() == CommonFileDialogResult.Ok)
            {
                TargetImage.ImageSave(dlgOpenFile.FileName);
                LogManager.WriteLog($"{dlgOpenFile.FileName} 이미지 저장 성공!");
            }
        }
        public void ChangeSource()
        {
            Bitmap resultBitmap = ImageConverter.BitmapImageToBitmap(TargetImage.Image);
            Bitmap sourceBitmap = new(resultBitmap);
            sourceBitmap = imageProcessing.Cvt32GrayTo8Gray(sourceBitmap);
            SourceImage.Image = ImageConverter.BitmapToBitmapImage(sourceBitmap);

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void Binarization(int threshold)
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            stopWatch.Start();
            LogManager.WriteLog($"이진화 시작");
            Bitmap resultBitmap = imageProcessing.Binarize(sourceBitmap, threshold);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);
            stopWatch.Stop();
            LogManager.WriteLog($"이진화 끝 소요시간 : {stopWatch.ElapsedMilliseconds}ms");
            stopWatch.Reset();

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void Erosion(int threshold, int kernelX, int kernelY)
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            stopWatch.Start();
            LogManager.WriteLog($"침식 시작");
            Bitmap resultBitmap = imageProcessing.Erosion(sourceBitmap, threshold, kernelX, kernelY);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);
            stopWatch.Stop();
            LogManager.WriteLog($"침식 끝 소요시간 : {stopWatch.ElapsedMilliseconds}ms");
            stopWatch.Reset();

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void Dilation(int threshold, int kernelX, int kernelY)
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            stopWatch.Start();
            LogManager.WriteLog($"팽창 시작");
            Bitmap resultBitmap = imageProcessing.Dilation(sourceBitmap, threshold, kernelX, kernelY);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);
            stopWatch.Stop();
            LogManager.WriteLog($"팽창 끝 소요시간 : {stopWatch.ElapsedMilliseconds}ms");
            stopWatch.Reset();

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void GaussianBlur(int sigma)
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            stopWatch.Start();
            LogManager.WriteLog($"가우시안 블러 시작");
            Bitmap resultBitmap = imageProcessing.GaussianBlur(sourceBitmap, sigma);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);
            stopWatch.Stop();
            LogManager.WriteLog($"가우시안 블러 끝 소요시간 : {stopWatch.ElapsedMilliseconds}ms");
            stopWatch.Reset();

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void Equalization()
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            stopWatch.Start();
            LogManager.WriteLog($"평활화 시작");
            Bitmap resultBitmap = imageProcessing.Equalization(sourceBitmap);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);
            stopWatch.Stop();
            LogManager.WriteLog($"평활화 끝 소요시간 : {stopWatch.ElapsedMilliseconds}ms");
            stopWatch.Reset();

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void Sobel()
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            stopWatch.Start();
            LogManager.WriteLog($"소벨 엣지 추출 시작");
            Bitmap resultBitmap = imageProcessing.Sobel(sourceBitmap);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);
            stopWatch.Stop();
            LogManager.WriteLog($"소벨 엣지 추출 끝 소요시간 : {stopWatch.ElapsedMilliseconds}ms");
            stopWatch.Reset();

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void Laplacian()
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            stopWatch.Start();
            LogManager.WriteLog($"라플라시안 엣지 추출 시작");
            Bitmap resultBitmap = imageProcessing.Laplacian(sourceBitmap);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);
            stopWatch.Stop();
            LogManager.WriteLog($"라플라시안 엣지 추출 끝 소요시간 : {stopWatch.ElapsedMilliseconds}ms");
            stopWatch.Reset();

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void FFTransform(int filterSize, bool lowFilterUse)
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            stopWatch.Start();
            LogManager.WriteLog($"FFT 변환 시작");
            Bitmap resultBitmap = imageProcessing.FFTransform(sourceBitmap, filterSize, lowFilterUse);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);
            stopWatch.Stop();
            LogManager.WriteLog($"FFT 변환 끝 소요시간 : {stopWatch.ElapsedMilliseconds}ms");
            stopWatch.Reset();

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }
        public void GetFFTSpectrum()
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            stopWatch.Start();
            LogManager.WriteLog($"FFT 변환 시작");
            Bitmap resultBitmap = imageProcessing.GetFFTSpectrum(sourceBitmap);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);
            stopWatch.Stop();
            LogManager.WriteLog($"FFT 변환 끝 소요시간 : {stopWatch.ElapsedMilliseconds}ms");
            stopWatch.Reset();

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void TempleteMaching(string templeteImagePath, int matchingRate)
        {
            ImageModel templeteImage = new ImageModel();
            templeteImage.ImageLoad(templeteImagePath);

            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            Bitmap templeteBitmap = ImageConverter.BitmapImageToBitmap(templeteImage.Image);
            stopWatch.Start();
            LogManager.WriteLog($"템플릿 매칭 시작");
            Bitmap resultBitmap = new(sourceBitmap);
            Point matchingPoint = (Point)imageProcessing.TempleteMatching(sourceBitmap, templeteBitmap, matchingRate);
            DrawMatchingResult(matchingPoint, templeteBitmap.Width, templeteBitmap.Height, resultBitmap);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);
            stopWatch.Stop();
            LogManager.WriteLog($"매치 포인트 X : {matchingPoint.X} Y : {matchingPoint.Y}");
            LogManager.WriteLog($"템플릿 매칭 끝 소요시간 : {stopWatch.ElapsedMilliseconds}ms");
            stopWatch.Reset();

            sourceBitmap.Dispose();
            templeteBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void GetSimilarityImage(string templeteImagePath)
        {
            ImageModel templeteImage = new ImageModel();
            templeteImage.ImageLoad(templeteImagePath);

            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            Bitmap templeteBitmap = ImageConverter.BitmapImageToBitmap(templeteImage.Image);
            stopWatch.Start();
            LogManager.WriteLog($"유사도 추출 시작");
            Bitmap resultBitmap = imageProcessing.GetSimilarity(sourceBitmap, templeteBitmap);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);
            stopWatch.Stop();
            LogManager.WriteLog($"유사도 추출 끝 소요시간 : {stopWatch.ElapsedMilliseconds}ms");
            stopWatch.Reset();

            sourceBitmap.Dispose();
            templeteBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void DrawMatchingResult(Point point, int width, int height, Bitmap resultBitmap)
        {
            // 펜 생성
            Pen redPen = new System.Drawing.Pen(System.Drawing.Brushes.Red, 3);

            using (Graphics g = Graphics.FromImage(resultBitmap))
            {
                // 도형 그리기 - X,Y좌표입력 후 100x100 사각형을 그림
                g.DrawRectangle(redPen, new Rectangle(point.X, point.Y, width, height));
            }

            // Dispose
            redPen.Dispose();
        }
    }
}
