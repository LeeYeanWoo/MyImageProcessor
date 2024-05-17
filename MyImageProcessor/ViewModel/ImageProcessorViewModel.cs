using Microsoft.WindowsAPICodePack.Dialogs;
using MyImageProcessor.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MyImageProcessor.ViewModel
{
    class ImageProcessorViewModel
    {
        ImageModel sourceImage;
        ImageModel targetImage;
        ImageModel preViewImage;

        public ImageModel SourceImage
        {
            get { return sourceImage; }
            set { sourceImage = value; }
        }

        public ImageModel TargetImage
        {
            get { return targetImage; }
            set { targetImage = value; }
        }

        public ImageModel PreViewImage
        {
            get { return preViewImage; }
            set { preViewImage = value; }
        }

        public ImageProcessorViewModel()
        {
            sourceImage = new();
            targetImage = new();
            preViewImage = new();
        }

        public void ClearData()
        {
            sourceImage.ImageLoad(@"..\..\Image\emptyImage.jpg");
            targetImage.ImageLoad(@"..\..\Image\emptyImage.jpg");
            preViewImage.ImageLoad(@"..\..\Image\emptyImage.jpg");
        }

        public void SourceImageLoad()
        {
            OpenFileDialog dlgOpenFile = new();
            dlgOpenFile.Filter = "Image Files (*.jpg, *.png, *.bmp) | *.jpg; *.png; *.bmp; | All files (*.*) | *.*";

            if (dlgOpenFile.ShowDialog().ToString() == "OK")
            {
                sourceImage.ImageLoad(dlgOpenFile.FileName);
                preViewImage.ImageLoad(dlgOpenFile.FileName);
            }
        }
        public void TargetImageSave()
        {
            CommonOpenFileDialog dlgOpenFile = new();

            if (dlgOpenFile.ShowDialog() == CommonFileDialogResult.Ok)
            {
                targetImage.ImageSave(dlgOpenFile.FileName);
            }
        }
    }
}
