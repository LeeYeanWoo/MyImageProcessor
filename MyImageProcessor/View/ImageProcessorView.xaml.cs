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
    }
}
