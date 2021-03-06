﻿using System;
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
using DirectShowLib;
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
                    if (frame == null) {
                        if (_camera.Eof && btnComplete.Enabled) {
                            btnComplete.PerformClick();
                        }
                        return;
                    }
                    videoImage.Image?.Dispose();

                    var result = Native.Process(frame, out points);
                    lblDetected.Text = Native.ToString(result);
                    lblDetected.Invalidate();
                    var bitmap = new Bitmap(frame.Bitmap);
                    if (cbShowDots.Checked) {
                        DrawDots(bitmap, points);
                    }
                    videoImage.Image = bitmap;
                    videoImage.Invalidate();
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

                    pNapNone.Value = (int)Convert.ToSingle(_calc.Napr[0]) * 100;
                    pNap1.Value = (int)Convert.ToSingle(_calc.Napr[1]) * 100;
                    pNap2.Value = (int)Convert.ToSingle(_calc.Napr[2]) * 100;
                }
            };
            lblDetected.Text = @"Лицо не найдено";
            _modelFileName = Directory.GetCurrentDirectory() + @"\model\main_clm_general.txt";

            var color = Color.Green;
            var text = @"Камера/Файл";
            if (!VideoDevice.HasDevice()) {
                color = Color.DarkRed;
                text = @"Файл";
            }
            lblDevice.ForeColor = color;
            lblDevice.Text = text;
            btnWeb.Enabled = VideoDevice.HasDevice();
            btnComplete.Enabled = false;
        }

        private static void DrawDots(Image bitmap, IEnumerable<Native.SessionPoint> points)
        {
            var pen = new Pen(Color.Red, 2);
            var g = Graphics.FromImage(bitmap);
            g.SmoothingMode = SmoothingMode.AntiAlias;
            foreach (var point in points) {
                g.DrawEllipse(pen, point.x, point.y, 2, 2);
            }
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
            }
            if (!_camera.StartWeb(this))
                return;
            ClearProgresses();
            btnWeb.Enabled = false;
            btnComplete.Enabled = true;
            btnFile.Enabled = false;

            if (!_initialized) {
                lblInformation.Text = @"Запуск зависимостей ...";
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
            btnWeb.Enabled = VideoDevice.HasDevice();
            btnComplete.Enabled = false;
            btnFile.Enabled = true;
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

        private void btnFile_Click(object sender, EventArgs e)
        {

            if (!File.Exists(_modelFileName)) {
                MessageBox.Show(this, $@"Файл модели {_modelFileName} не найден", @"Предупреждение", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            if (!_camera.StartFile(this))
                return;
            ClearProgresses();
            btnFile.Enabled = false;
            btnWeb.Enabled = false;
            btnComplete.Enabled = true;

            if (!_initialized) {
                lblInformation.Text = @"Запуск зависимостей ...";
                lblInformation.Invalidate();
                Native.Create(_modelFileName);
                _initialized = true;
            }

            _capture.Interval = new TimeSpan(300);
            _capture.Start();
            lblInformation.Text = @"Захват видео запущен";
            lblInformation.Invalidate();
        }

        private void ClearProgresses()
        {
            pNone.Value = 0;
            pAnger.Value = 0;
            pDisgust.Value = 0;
            pFear.Value = 0;
            pJoy.Value = 0;
            pSorrow.Value = 0;
            pSurprise.Value = 0;

            pNapNone.Value = 0;
            pNap1.Value = 0;
            pNap2.Value = 0;
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

        public bool Eof => _capture?.GetCaptureProperty(Emgu.CV.CvEnum.CapProp.FrameCount) ==
                           _capture?.GetCaptureProperty(Emgu.CV.CvEnum.CapProp.PosFrames);

        public Mat Frame
        {
            get {
                lock (_locker) {
                    return _capture?.QueryFrame();
                }
            }
        }

        public bool HasNext => _capture != null && _capture.Grab();


        public bool Reset(IWin32Window window)
        {
            lock (_locker) {
                Close();
                return StartWeb(window);
            }
        }

        public bool StartFile(IWin32Window window)
        {
            var d = new OpenFileDialog {
                CheckFileExists = true,
                InitialDirectory = Directory.GetCurrentDirectory(),
                Filter = @"AVI|*.avi|MPEG|*.mpeg;*.mpg;*.mp4|XDIV|*.xdiv"
            };
            var ret = d.ShowDialog(window);
            if (ret == DialogResult.Yes || ret == DialogResult.OK) {
                var filename = d.FileName;
                _capture = new Capture(filename);
            } else {
                return false;
            }
            return true;
        }

        public bool StartWeb(IWin32Window window)
        {
            if (VideoDevice.HasDevice()) {
                _capture = new Capture(_deviceNo);
            } else {
                var d = new OpenFileDialog {
                    CheckFileExists = true,
                    InitialDirectory = Directory.GetCurrentDirectory(),
                    Filter = @"AVI|*.avi|MPEG|*.mpeg;*.mpg;*.mp4|XDIV|*.xdiv"
                };
                var ret = d.ShowDialog(window);
                if (ret == DialogResult.Yes || ret == DialogResult.OK) {
                    var filename = d.FileName;
                    _capture = new Capture(filename);
                } else {
                    return false;
                }
            }
            return true;
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

    internal static class VideoDevice
    {
        internal static bool HasDevice()
        {
            var list = DsDevice.GetDevicesOfCat(FilterCategory.VideoInputDevice);
            return list.Length > 0;
        }
    }
}
