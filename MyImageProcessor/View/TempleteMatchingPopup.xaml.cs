using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace MyImageProcessor.View
{
    /// <summary>
    /// TampleteMatchingPopup.xaml에 대한 상호 작용 논리
    /// </summary>
    public delegate void SimilarityGetEventHandler(string path);

    public partial class TempleteMatchingPopup : Window
    {
        public DataGetEventHandler DataSendEvent;
        public SimilarityGetEventHandler SimilarityGetEvent;
        public TempleteMatchingPopup()
        {
            InitializeComponent();
        }
        private void TempleteImageLoadButtonClick(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dlgOpenFile = new();
            dlgOpenFile.Filter = "Image Files (*.jpg, *.png, *.bmp) | *.jpg; *.png; *.bmp; | All files (*.*) | *.*";

            if (dlgOpenFile.ShowDialog().ToString() == "OK")
            {
                TempleteImagePathTextBox.Text = dlgOpenFile.FileName;
            }
        }
        private void DataSendButtonClick(object sender, RoutedEventArgs e)
        {
            int matchingRate = 0;
            try
            {
                matchingRate = int.Parse(MatchingRateTextBox.Text);
            }
            catch (FormatException)
            {
                MatchingRateTextBox.Text = $"잘못된 입력입니다";
            }
            if(TempleteImagePathTextBox.Text.Length > 0 && matchingRate > 0 && matchingRate <= 100)
            {
                DataSendEvent($"{TempleteImagePathTextBox.Text},{MatchingRateTextBox.Text}");
                Close();
            }
            else
                TempleteImagePathTextBox.Text = $"잘못된 입력입니다";
        }
        private void SimilarityButtonClick(object sender, RoutedEventArgs e)
        {
            if (TempleteImagePathTextBox.Text.Length > 0)
            {
                SimilarityGetEvent($"{TempleteImagePathTextBox.Text}");
                Close();
            }
            else
                TempleteImagePathTextBox.Text = $"잘못된 입력입니다";
        }
    }
}
