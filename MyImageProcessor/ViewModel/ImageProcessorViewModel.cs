using CLRImageProcessing;
using Microsoft.WindowsAPICodePack.Dialogs;
using MyImageProcessor.Model;
using System;
using System.Collections.Generic;
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

        public ImageModel SourceImage { get; set; }

        public ImageModel TargetImage { get; set; }

        public ImageModel PreViewImage { get; set; }

        public ImageProcessorViewModel()
        {
            SourceImage = new();
            TargetImage = new();
            PreViewImage = new();
            imageProcessing = new();
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
            }
        }
        public void TargetImageSave()
        {
            CommonOpenFileDialog dlgOpenFile = new();

            if (dlgOpenFile.ShowDialog() == CommonFileDialogResult.Ok)
            {
                TargetImage.ImageSave(dlgOpenFile.FileName);
            }
        }

        public void Binarization(int threshold)
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            Bitmap resultBitmap = imageProcessing.Binarize(sourceBitmap, threshold);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void Erosion(int threshold, int kernelX, int kernelY)
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            Bitmap resultBitmap = imageProcessing.Erosion(sourceBitmap, threshold, kernelX, kernelY);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void Dilation(int threshold, int kernelX, int kernelY)
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            Bitmap resultBitmap = imageProcessing.Dilation(sourceBitmap, threshold, kernelX, kernelY);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void GaussianBlur(int sigma)
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            Bitmap resultBitmap = imageProcessing.GaussianBlur(sourceBitmap, sigma);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void Equalization()
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            Bitmap resultBitmap = imageProcessing.Equalization(sourceBitmap);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void Sobel()
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            Bitmap resultBitmap = imageProcessing.Sobel(sourceBitmap);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }

        public void Laplacian()
        {
            Bitmap sourceBitmap = ImageConverter.BitmapImageToBitmap(SourceImage.Image);
            Bitmap resultBitmap = imageProcessing.Laplacian(sourceBitmap);
            TargetImage.Image = ImageConverter.BitmapToBitmapImage(resultBitmap);

            sourceBitmap.Dispose();
            resultBitmap.Dispose();
        }
    }
}
