using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Emgu.CV;
using Emgu.CV.Structure;
using OfPackage.Library;

namespace CaptureConsole
{
    internal static class Program
    {
        private static void Main(string[] args)
        {
            var capture = new Capture(0);
            Native.Create(@"D:\GitHub\videotools\Debug\model\main_clm_general.txt");
            var id = 0;
            while (capture.Grab()) {
                var mat = capture.QueryFrame();
                Native.SessionPoint[] points;
                var result = Native.Process(mat, out points);
                Console.WriteLine(@"{0:D5}:{1}", id, Native.ToString(result));
                ++id;
            }
            Native.Destroy();
        }
    }
}
