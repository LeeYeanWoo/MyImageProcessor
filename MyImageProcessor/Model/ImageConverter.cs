using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media.Imaging;

namespace MyImageProcessor.Model
{
    class ImageConverter
    {
        public static BitmapImage BitmapToBitmapImage(Bitmap bitmap)
        {
            using(MemoryStream outStream = new MemoryStream())
            {
                bitmap.Save(outStream, ImageFormat.Bmp);
                outStream.Position = 0;

                BitmapImage bitmapImage = new BitmapImage();
                bitmapImage.BeginInit();
                bitmapImage.StreamSource = outStream;
                bitmapImage.CacheOption = BitmapCacheOption.OnLoad;
                bitmapImage.EndInit();
                bitmapImage.Freeze();

                return bitmapImage;
            }
        }

        public static Bitmap BitmapImageToBitmap(BitmapImage bitmapImage)
        {
            using(MemoryStream outStream = new MemoryStream())
            {
                BitmapEncoder encoder = new BmpBitmapEncoder();
                encoder.Frames.Add(BitmapFrame.Create(bitmapImage));
                encoder.Save(outStream);
                Bitmap bitmap = new Bitmap(outStream);

                return bitmap;
            }
        }
    }
}
