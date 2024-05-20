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
    /// ThresholdPopup.xaml에 대한 상호 작용 논리
    /// </summary>
    public delegate void DataGetEventHandler(string data);
    public partial class ThresholdPopup : Window
    {
        public DataGetEventHandler DataSendEvent;
        public ThresholdPopup()
        {
            InitializeComponent();

        }

        private void DataSendButtonClick(object sender, RoutedEventArgs e)
        {
            int threshold = -1;
            try
            {
                threshold = int.Parse(ThresholdTextBox.Text);
            }
            catch (FormatException)
            {
                ThresholdTextBox.Text = $"잘못된 입력입니다";
            }

            if(threshold >= 0 && threshold <= 255)
            {
                DataSendEvent(ThresholdTextBox.Text);
                Close();
            }
            else
                ThresholdTextBox.Text = $"잘못된 입력입니다";
        }
    }
}
