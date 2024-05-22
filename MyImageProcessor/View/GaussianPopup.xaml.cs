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
    /// GaussianPopup.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class GaussianPopup : Window
    {
        public DataGetEventHandler DataSendEvent;
        public GaussianPopup()
        {
            InitializeComponent();
        }
        private void DataSendButtonClick(object sender, RoutedEventArgs e)
        {
            int threshold = -1;
            try
            {
                threshold = int.Parse(SigmaTextBox.Text);
            }
            catch (FormatException)
            {
                SigmaTextBox.Text = $"잘못된 입력입니다";
            }

            if (threshold >= 1 && threshold <= 10)
            {
                DataSendEvent(SigmaTextBox.Text);
                Close();
            }
            else
                SigmaTextBox.Text = $"잘못된 입력입니다";
        }
    }
}
