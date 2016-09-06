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
using Emgu.CV;
using Emgu.CV.Structure;
using OfPackage;
using OfPackage.Library;


namespace CaptureVideo
{
    public partial class MainForm : Form
    {
        private static readonly object Locker = new object();
        private readonly WebCamera _camera = new WebCamera(0, Locker);
        private readonly DispatcherTimer _timer = new DispatcherTimer();
        private readonly DispatcherTimer _capture = new DispatcherTimer();
        private bool _initialized = false;
        private readonly Neutroncalc _calc = new Neutroncalc();

        private readonly double[] _x = new double[68];
        private readonly double[] _y = new double[68];

        private readonly string _modelFileName;

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
                    if (!_camera.HasNext) return;
                    Native.SessionPoint[] points;
                    var frame = _camera.Frame;
                    videoImage.Image?.Dispose();
                    videoImage.Image = new Bitmap(frame.Bitmap);
                    videoImage.Invalidate();
                    var result = Native.Process(frame, out points);
                    lblDetected.Text = Native.ToString(result);
                    lblDetected.Invalidate();
                    for (var i = 0; i < points.Length; i++) {
                        var point = points[i];
                        _x[i] = point.x;
                        _y[i] = point.y;
                    }
                    _calc.calcEmo2d(_x, _y);
                    /** */
                    pNone.Value = (int)Convert.ToSingle(_calc.Emo[0]) * 100;
                    pAnger.Value = (int)Convert.ToSingle(_calc.Emo[1]) * 100;
                    pDisgust.Value = (int)Convert.ToSingle(_calc.Emo[2]) * 100;
                    pFear.Value = (int)Convert.ToSingle(_calc.Emo[3]) * 100;
                    pJoy.Value = (int)Convert.ToSingle(_calc.Emo[4]) * 100;
                    pSorrow.Value = (int)Convert.ToSingle(_calc.Emo[5]) * 100;
                    pSurprise.Value = (int)Convert.ToSingle(_calc.Emo[6]) * 100;
                }
            };
            lblDetected.Text = @"";
            _modelFileName = Directory.GetCurrentDirectory() + @"\model\main_clm_general.txt";
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
            } else if (startBytes >= 100000000 && startBytes < 200000000) {
                color = Color.Chocolate;
            } else {
                color = Color.DarkRed;
                //GC.Collect();
            }
            return new Tuple<string, Color>(text, color);
        }

        private void btnStart_Click(object sender, EventArgs e)
        {

            if (!File.Exists(_modelFileName)) {
                MessageBox.Show(this, $@"Файл модели {_modelFileName} не найден", @"Предупреждение", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
                ;
            }
            _camera.Start();
            btnStart.Enabled = false;
            btnComplete.Enabled = true;

            if (!_initialized) {
                lblInformation.Text = @"Запуск OpenFace...";
                lblInformation.Invalidate();
                Native.Create(_modelFileName);
                _initialized = true;
            }

            _capture.Interval = new TimeSpan(300);
            _capture.Start();
            lblInformation.Text = @"Захват видео запущен";
            lblInformation.Invalidate();
        }

        private void btnComplete_Click(object sender, EventArgs e)
        {
            lblDetected.Text = @"";
            btnStart.Enabled = true;
            btnComplete.Enabled = false;
            lblInformation.Text = @"Закрывается камера";
            lblInformation.Invalidate();
            _camera.Close();
            lblInformation.Text = @"Останавливается процесс обработки";
            lblInformation.Invalidate();
            _capture.Stop();
            lblInformation.Text = @"Захват останавливается";
            lblInformation.Invalidate();
            _initialized = false;
            Native.Destroy();
            lblInformation.Text = @"Захват остановлен";
            lblInformation.Invalidate();
        }

        private void btnGC_Click(object sender, EventArgs e)
        {
            GC.Collect();
            GC.WaitForFullGCApproach();
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

        public bool HasNext => _capture != null && _capture.Grab();


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
            _capture?.Stop();
            _capture?.Dispose();
            _capture = null;
        }

        public void Dispose()
        {
            _capture?.Dispose();
        }
    }
}
