using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Imaging;
using HurlbertVisionLab.LibRawWrapper;
using HurlbertVisionLab.LibRawWrapper.Native;

namespace LibRawWrapperConsole
{
    class Program
    {
        [STAThread]
        static void Main(string[] args)
        {
            LibRawBitmapDecoder libraw = new LibRawBitmapDecoder(new Uri(@"C:\tmp\00_0011.CR2"), BitmapCreateOptions.None, BitmapCacheOption.None);

            BitmapFrame frame = libraw.Frames[0];
            Console.WriteLine(frame.Format);

            Window win = new Window { Content = new Image { Source = frame } };
            new Application().Run(win);
        }

        private static void OnProgressChanged(object sender, LibRawProgressEventArgs args)
        {
            Console.WriteLine($"{args.Description} ({args.Percent:P0})");
        }
    }
}
