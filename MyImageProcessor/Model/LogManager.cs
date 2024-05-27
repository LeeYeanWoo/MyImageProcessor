using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MyImageProcessor.Model
{
    class LogManager : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        private static ObservableCollection<LogManager> instance = new();

        public string Time
        {
            get; set;
        }        
        public string Contents
        {
            get; set;
        }

        public static ObservableCollection<LogManager> Instance
        {
            get
            {
                if (instance == null)
                    instance = new ObservableCollection<LogManager>();

                return instance;
            }
        }

        public static void WriteLog(string text)
        {
            instance.Add(new LogManager() { Time = DateTime.Now.ToString("MM-dd HH:mm:ss")
                , Contents = $"{text}"});
        }

        protected void OnPropertyChanged(String propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
