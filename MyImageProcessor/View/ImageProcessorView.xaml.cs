using MyImageProcessor.View;
using MyImageProcessor.ViewModel;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace MyImageProcessor
{
    /// <summary>
    /// MainWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class ImageProcessorView : Window
    {
        ImageProcessorViewModel viewModel;
        public ImageProcessorView()
        {
            InitializeComponent();
            viewModel = new();
            DataContext = viewModel;
        }

        private void ImageLoadButtonClick(object sender, RoutedEventArgs e)
        {
            viewModel.SourceImageLoad();
            sourceScaleTransform.ScaleX = 1.0;
            sourceScaleTransform.ScaleY = 1.0;
        }

        private void ImageSaveButtonClick(object sender, RoutedEventArgs e)
        {
            viewModel.TargetImageSave();
        }

        private void SourceScaleUpButtonClick(object sender, RoutedEventArgs e)
        {
            sourceScaleTransform.ScaleX *= 1.1;
            sourceScaleTransform.ScaleY *= 1.1;
        }

        private void SourceScaleDownButtonClick(object sender, RoutedEventArgs e)
        {
            sourceScaleTransform.ScaleX /= 1.1;
            sourceScaleTransform.ScaleY /= 1.1;
        }

        private void TargetScaleUpButtonClick(object sender, RoutedEventArgs e)
        {
            targetScaleTransform.ScaleX *= 1.1;
            targetScaleTransform.ScaleY *= 1.1;
        }

        private void TargetScaleDownButtonClick(object sender, RoutedEventArgs e)
        {
            targetScaleTransform.ScaleX /= 1.1;
            targetScaleTransform.ScaleY /= 1.1;
        }

        private void ClearButtonClick(object sender, RoutedEventArgs e)
        {
            sourceScaleTransform.ScaleX = 1.0;
            sourceScaleTransform.ScaleY = 1.0;
            targetScaleTransform.ScaleX = 1.0;
            targetScaleTransform.ScaleY = 1.0;
            viewModel.ClearData();
        }

        private void BinarizeButtonClick(object sender, RoutedEventArgs e)
        {
            ThresholdPopup thresholdPopup = new();
            thresholdPopup.DataSendEvent += new DataGetEventHandler(this.ThresholdDataGet);
            thresholdPopup.WindowStartupLocation = WindowStartupLocation.CenterScreen;
            thresholdPopup.ShowDialog();
        }

        private void ThresholdDataGet(string data)
        {
            int threshold = int.Parse(data);
            viewModel.Binarization(threshold);
            targetScaleTransform.ScaleX = 1.0;
            targetScaleTransform.ScaleY = 1.0;
        }

        private void ErosionButtonClick(object sender, RoutedEventArgs e)
        {
            ErosionPopup erosionPopup = new();
            erosionPopup.DataSendEvent += new DataGetEventHandler(this.ErosionDataGet);
            erosionPopup.WindowStartupLocation = WindowStartupLocation.CenterScreen;
            erosionPopup.ShowDialog();
        }

        private void ErosionDataGet(string data)
        {
            string[]dataArr = data.Split(',');
            int threshold = int.Parse(dataArr[0]);
            int kernelX = int.Parse(dataArr[1]);
            int kernelY = int.Parse(dataArr[2]);
            viewModel.Erosion(threshold, kernelX, kernelY);
            targetScaleTransform.ScaleX = 1.0;
            targetScaleTransform.ScaleY = 1.0;
        }

        private void DilationButtonClick(object sender, RoutedEventArgs e)
        {
            DilationPopup dilationPopup = new();
            dilationPopup.DataSendEvent += new DataGetEventHandler(this.DilationDataGet);
            dilationPopup.WindowStartupLocation = WindowStartupLocation.CenterScreen;
            dilationPopup.ShowDialog();
        }

        private void DilationDataGet(string data)
        {
            string[] dataArr = data.Split(',');
            int threshold = int.Parse(dataArr[0]);
            int kernelX = int.Parse(dataArr[1]);
            int kernelY = int.Parse(dataArr[2]);
            viewModel.Dilation(threshold, kernelX, kernelY);
            targetScaleTransform.ScaleX = 1.0;
            targetScaleTransform.ScaleY = 1.0;
        }
        private void GaussianButtonClick(object sender, RoutedEventArgs e)
        {
            GaussianPopup gaussianPopup = new();
            gaussianPopup.DataSendEvent += new DataGetEventHandler(this.GaussianDataGet);
            gaussianPopup.WindowStartupLocation = WindowStartupLocation.CenterScreen;
            gaussianPopup.ShowDialog();
        }
        private void GaussianDataGet(string data)
        {
            int sigma = int.Parse(data);
            viewModel.GaussianBlur(sigma);
            targetScaleTransform.ScaleX = 1.0;
            targetScaleTransform.ScaleY = 1.0;
        }

        private void EqualizationButtonClick(object sender, RoutedEventArgs e)
        {
            viewModel.Equalization();
            targetScaleTransform.ScaleX = 1.0;
            targetScaleTransform.ScaleY = 1.0;
        }

        private void SobelButtonClick(object sender, RoutedEventArgs e)
        {
            viewModel.Sobel();
            targetScaleTransform.ScaleX = 1.0;
            targetScaleTransform.ScaleY = 1.0;
        }
    }
}
