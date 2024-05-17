using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;

namespace MyImageProcessor.Model
{
    class ImageModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        string imagePath;
        BitmapImage image;

        public string ImagePath
        {
            get { return imagePath; }
            set
            {
                imagePath = value;
                OnPropertyChanged("ImagePath");
            }
        }
        public BitmapImage Image
        {
            get { return image; }
            set
            {
                image = value;
                OnPropertyChanged("Image");
            }
        }

        public ImageModel()
        {
            imagePath = @"..\..\Image\emptyImage.jpg";
            ImageLoad(imagePath);
        }

        public void ImageLoad(string strPath)
        {
            try
            {
                FileInfo fio = new FileInfo(strPath);
                if (fio.Exists)
                {
                    BitmapImage img = new BitmapImage();

                    // 이미지 객체의 초기화를 시작
                    img.BeginInit();
                    // 이미지 파일이 로드된 후에는 원본 파일에 대한 의존성이 없어지도록 이미지의 캐시 옵션을 설정
                    img.CacheOption = BitmapCacheOption.OnLoad;
                    //  이미지를 로드할 때 시스템의 이미지 캐시를 무시하고 항상 파일로부터 이미지를 로드하도록 이미지 생성 옵션을 설정
                    img.CreateOptions = BitmapCreateOptions.IgnoreImageCache;
                    // 로드할 이미지의 파일 경로를 지정
                    img.UriSource = new Uri(strPath, UriKind.RelativeOrAbsolute);
                    // 모든 속성 설정 후 EndInit 메서드를 호출하여 초기화 과정을 마무리
                    img.EndInit();

                    // Freeze 메서드를 호출하여 이미지 객체를 변경 불가능하게 생성
                    //if (img.CanFreeze) img.Freeze();

                    Image = img;
                    ImagePath = strPath;
                }
                else
                    return;
            }
            catch
            {
                return;
            }
        }

        public void ImageSave(string strPath)
        {
            BmpBitmapEncoder encoder = new();
            encoder.Frames.Add(BitmapFrame.Create(image));

            using var stream = new FileStream($"{strPath}.bmp", FileMode.Create);
            encoder.Save(stream);
        }

        protected void OnPropertyChanged(String propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
