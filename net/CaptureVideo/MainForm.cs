using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;
using System.Threading;
using System.Windows.Threading;


namespace CaptureVideo
{
    public partial class MainForm : Form
    {
        private readonly DispatcherTimer _timer = new DispatcherTimer();

        public MainForm()
        {
            InitializeComponent();
            Closed += (sender, args) => DeInitialize();
            _timer.Interval = new TimeSpan(0, 0, 0, 0, 500);
            _timer.Tick += (o, args) => {
                var iMemorySize = Process.GetCurrentProcess().WorkingSet64;
                var p = FormatBytes(iMemorySize);
                lblMemory.Text = p.Item1;
                lblMemory.ForeColor = p.Item2;
                lblMemory.Invalidate();
            };
            _timer.Start();
        }

        private static void DeInitialize()
        {
            
        }

        private static Tuple<string, Color> FormatBytes(long bytes)
        {
            string[] suffix = { "B", "KB", "MB", "GB", "TB" };
            int i;
            var startBytes = bytes;
            double dblSByte = bytes;
            for (i = 0; i < suffix.Length && bytes >= 1024; i++, bytes /= 1024)
                dblSByte = bytes / 1024.0;
            var text = $"{dblSByte:0.##}{suffix[i]}";
            Color color;
            if (startBytes < 80000000) {
                color = Color.DarkGreen;
            } else if (startBytes >= 100000000 && startBytes < 150000000) {
                color = Color.Chocolate;
            } else {
                color = Color.DarkRed;
                //GC.Collect();
            }
            return new Tuple<string, Color>(text, color);
        }
    }
}
