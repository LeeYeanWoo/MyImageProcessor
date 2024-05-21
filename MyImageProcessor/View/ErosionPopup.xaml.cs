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
using System.Windows.Shapes;

namespace MyImageProcessor.View
{
    /// <summary>
    /// MorphologyPopup.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class ErosionPopup : Window
    {
        public DataGetEventHandler DataSendEvent;
        public ErosionPopup()
        {
            InitializeComponent();
        }
        private void DataSendButtonClick(object sender, RoutedEventArgs e)
        {
            int threshold = -1;
            int kernelXSize = -1;
            int kernelYSize = -1;
            try
            {
                threshold = int.Parse(ThresholdTextBox.Text);
            }
            catch (FormatException)
            {
                ThresholdTextBox.Text = $"잘못된 입력입니다";
            }

            try
            {
                kernelXSize = int.Parse(KernelXTextBox.Text);
            }
            catch (FormatException)
            {
                KernelXTextBox.Text = $"잘못된 입력입니다";
            }

            try
            {
                kernelYSize = int.Parse(KernelYTextBox.Text);
            }
            catch (FormatException)
            {
                KernelYTextBox.Text = $"잘못된 입력입니다";
            }

            if (threshold >= 0 && threshold <= 255 && kernelXSize >= 2 && kernelXSize <=50 && kernelYSize >= 2 && kernelYSize <= 50)
            {
                DataSendEvent($"{ThresholdTextBox.Text},{KernelXTextBox.Text},{KernelYTextBox.Text}");
                Close();
            }
            else
                ThresholdTextBox.Text = $"잘못된 입력입니다";
        }
    }
}
