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
    /// FFTPopup.xaml에 대한 상호 작용 논리
    /// </summary>
    public delegate void SpectrumGetEventHandler();
    
    public partial class FFTPopup : Window
    {
        public DataGetEventHandler DataSendEvent;
        public SpectrumGetEventHandler SpectrumGetEvent;
        public FFTPopup()
        {
            InitializeComponent();
        }
        private void DataSendButtonClick(object sender, RoutedEventArgs e)
        {
            int filterSize = -1;
            bool lowLevelFilterUse = (bool)lowFilterUse.IsChecked;
            try
            {
                filterSize = int.Parse(FilterSizeTextBox.Text);
            }
            catch (FormatException)
            {
                FilterSizeTextBox.Text = $"잘못된 입력입니다";
            }

            if (filterSize > 0)
            {
                DataSendEvent($"{FilterSizeTextBox.Text},{lowLevelFilterUse}");
                Close();
            }
            else
                FilterSizeTextBox.Text = $"잘못된 입력입니다";
        }

        private void SpectrumGetButtonClick(object sender, RoutedEventArgs e)
        {
            SpectrumGetEvent();
            Close();
        }
    }
}
