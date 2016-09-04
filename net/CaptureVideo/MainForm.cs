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
using CaptureVideo.Library;
using Emgu.CV;
using Emgu.CV.Structure;


namespace CaptureVideo
{
    public partial class MainForm : Form
    {
        private static readonly object Locker = new object();
        private readonly WebCamera _camera = new WebCamera(0, Locker);
        private readonly DispatcherTimer _timer = new DispatcherTimer();
        private readonly DispatcherTimer _capture = new DispatcherTimer();
        private bool _initialized = false;

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
            _capture.Tick += (sender, args) => {
                lock (Locker) {
                    Native.SessionPoint[] points;
                    var frame = _camera.Frame;
                    videoImage.Image?.Dispose();
                    var result = Native.Process(frame, out points);
                    videoImage.Image = new Bitmap(frame.Bitmap);
                    lblDetected.Text = Native.ToString(result);
                }
            };
            lblDetected.Text = @"Не найдено";
        }

        private void DeInitialize()
        {
            if (!_initialized) return;
            Native.Destroy();
            _initialized = false;
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

        private void btnStart_Click(object sender, EventArgs e)
        {
            if (!_initialized) {
                lblInformation.Text = @"Запуск OpenFace...";
                lblInformation.Invalidate();
                Native.Create(@"D:\GitHub\videotools\Debug\model\main_clnf_general.txt");
                _initialized = true;
            }
            _camera.Start();
            _capture.Interval = new TimeSpan(30);
            _capture.Start();
            lblInformation.Text = @"Запуск захвата видео";
        }

        private void btnComplete_Click(object sender, EventArgs e)
        {
            _capture.Stop();
            _camera.Close();
            lblInformation.Text = @"Остановка захвата";
        }
    }

    internal sealed class WebCamera : IDisposable
    {
        private readonly int _deviceNo;
        private Capture _capture;
        private readonly object _locker;

        public WebCamera(int deviceNo, object locker)
        {
            _deviceNo = deviceNo;
            _locker = locker;
        }
        public int FramePerSecond
        {
            get {
                if (_capture != null)
                    return (int)_capture?.GetCaptureProperty(Emgu.CV.CvEnum.CapProp.Fps);
                return 0;
            }
            set {
                _capture?.SetCaptureProperty(Emgu.CV.CvEnum.CapProp.Fps, value);
            }
        }

        public Mat Frame
        {
            get {
                lock (_locker) {
                    return _capture?.QueryFrame();
                }
            }
        }

        public bool HasNext => true;

        public void Reset()
        {
            lock (_locker) {
                Close();
                Start();
            }
        }

        public void Start()
        {
            _capture = new Capture(_deviceNo);
        }

        public void Close()
        {
            _capture?.Dispose();
        }

        public void Dispose()
        {
            _capture?.Dispose();
        }
    }
}
