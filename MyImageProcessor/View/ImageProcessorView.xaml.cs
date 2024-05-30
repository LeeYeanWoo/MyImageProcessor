using MyImageProcessor.Model;
using MyImageProcessor.View;
using MyImageProcessor.ViewModel;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
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
            logListView.ItemsSource = LogManager.Instance;
            LogManager.Instance.CollectionChanged += LogItems_CollectionChanged;
        }
        private void LogItems_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (e.Action == NotifyCollectionChangedAction.Add && e.NewItems != null && e.NewItems.Count > 0)
            {
                // 새로운 항목이 추가될 때 스크롤을 최신 항목으로 이동
                logListView.ScrollIntoView(e.NewItems[e.NewItems.Count - 1]);
            }
        }

        private void TargetScrollViewer_ScrollChanged(object sender, ScrollChangedEventArgs e)
        {
            UpdatePreviewRectangle(targetScrollViewer, targetImage, PreviewRectangle);
        }

        private void UpdatePreviewRectangle(ScrollViewer scrollViewer, Image image, Rectangle rectangle)
        {
            if (image.Source is BitmapSource bitmap)
            {
                double scale = targetScaleTransform.ScaleX;

                // Adjust the scale to fit the preview rectangle within the canvas
                double canvasWidth = overlayCanvas.ActualWidth;
                double canvasHeight = overlayCanvas.ActualHeight;
                double imageWidth = bitmap.PixelWidth * scale;
                double imageHeight = bitmap.PixelHeight * scale;

                double viewportWidth = scrollViewer.ViewportWidth;
                double viewportHeight = scrollViewer.ViewportHeight;
                double offsetX = scrollViewer.HorizontalOffset;
                double offsetY = scrollViewer.VerticalOffset;

                double previewScaleX = canvasWidth / imageWidth;
                double previewScaleY = canvasHeight / imageHeight;
                double previewScale = Math.Min(previewScaleX, previewScaleY);

                if (previewScale < 0) previewScale = 0.0;

                rectangle.Width = viewportWidth * previewScale;
                rectangle.Height = viewportHeight * previewScale;
                Canvas.SetLeft(rectangle, offsetX * previewScale);
                Canvas.SetTop(rectangle, offsetY * previewScale);
            }
        }
        private void AdjustSourceScale()
        {
            if (sourceImage.Source is BitmapSource bitmap)
            {
                double xScale = sourceScrollViewer.ViewportWidth / bitmap.PixelWidth;
                double yScale = sourceScrollViewer.ViewportHeight / bitmap.PixelHeight;
                double scale = Math.Min(xScale, yScale);

                sourceScaleTransform.ScaleX = scale;
                sourceScaleTransform.ScaleY = scale;
            }
        }
        private void AdjustTargetScale()
        {
            if (targetImage.Source is BitmapSource bitmap)
            {
                double xScale = targetScrollViewer.ViewportWidth / bitmap.PixelWidth;
                double yScale = targetScrollViewer.ViewportHeight / bitmap.PixelHeight;
                double scale = Math.Min(xScale, yScale);

                targetScaleTransform.ScaleX = scale;
                targetScaleTransform.ScaleY = scale;
            }
        }
        private void SourceImage_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            if (e.Delta > 0)
            {
                sourceScaleTransform.ScaleX *= 1.1;
                sourceScaleTransform.ScaleY *= 1.1;
            }
            else if (e.Delta < 0)
            {
                sourceScaleTransform.ScaleX /= 1.1;
                sourceScaleTransform.ScaleY /= 1.1;
            }
        }

        private void TargetImage_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            if (e.Delta > 0)
            {
                targetScaleTransform.ScaleX *= 1.1;
                targetScaleTransform.ScaleY *= 1.1;
            }
            else if (e.Delta < 0)
            {
                targetScaleTransform.ScaleX /= 1.1;
                targetScaleTransform.ScaleY /= 1.1;
            }
        }

        private void ImageLoadButtonClick(object sender, RoutedEventArgs e)
        {
            viewModel.SourceImageLoad();
            AdjustSourceScale();
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
            AdjustSourceScale();
            AdjustTargetScale();
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
            AdjustSourceScale();
            AdjustTargetScale();
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
            AdjustSourceScale();
            AdjustTargetScale();
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
            AdjustSourceScale();
            AdjustTargetScale();
        }

        private void EqualizationButtonClick(object sender, RoutedEventArgs e)
        {
            viewModel.Equalization();
            AdjustSourceScale();
            AdjustTargetScale();
        }

        private void SobelButtonClick(object sender, RoutedEventArgs e)
        {
            viewModel.Sobel();
            AdjustSourceScale();
            AdjustTargetScale();
        }

        private void LaplacianButtonClick(object sender, RoutedEventArgs e)
        {
            viewModel.Laplacian();
            AdjustSourceScale();
            AdjustTargetScale();
        }

        private void TempleteMatchingButtonClick(object sender, RoutedEventArgs e)
        {
            TempleteMatchingPopup templeteMatchingPopup = new();
            templeteMatchingPopup.DataSendEvent += new DataGetEventHandler(this.TempleteMatchingDataGet);
            templeteMatchingPopup.WindowStartupLocation = WindowStartupLocation.CenterScreen;
            templeteMatchingPopup.ShowDialog();
        }
        private void TempleteMatchingDataGet(string data)
        {
            string[] dataArr = data.Split(',');
            string templeteImagePath = dataArr[0];
            int matchingRate = int.Parse(dataArr[1]);
            viewModel.TempleteMaching(templeteImagePath, matchingRate);
            AdjustSourceScale();
            AdjustTargetScale();
        }

        private void FFTButtonClick(object sender, RoutedEventArgs e)
        {
            FFTPopup fFTPopup = new();
            fFTPopup.DataSendEvent += new DataGetEventHandler(this.FFTDataGet);
            fFTPopup.SpectrumGetEvent += new SpectrumGetEventHandler(this.FFTSpectrumGet);
            fFTPopup.WindowStartupLocation = WindowStartupLocation.CenterScreen;
            fFTPopup.ShowDialog();
        }

        private void FFTDataGet(string data)
        {
            string[] dataArr = data.Split(',');
            int filterSize = int.Parse(dataArr[0]);
            bool lowFilterUse = bool.Parse(dataArr[1]);
            viewModel.FFTransform(filterSize, lowFilterUse);
            AdjustSourceScale();
            AdjustTargetScale();
        }

        private void FFTSpectrumGet()
        {
            viewModel.GetFFTSpectrum();
            AdjustSourceScale();
            AdjustTargetScale();
        }

        private void ChangeSourceButtonClick(object sender, RoutedEventArgs e)
        {
            viewModel.ChangeSource();
        }
    }
}
