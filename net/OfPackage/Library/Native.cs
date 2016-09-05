using System;
using System.Runtime.InteropServices;
using Emgu.CV;
using Emgu.CV.Structure;

namespace OfPackage.Library
{
    public static class Native
    {
        private const string LibraryName = @"OpenFaceWrap.dll";

        #region CreateSession
        [DllImport(LibraryName, EntryPoint = "create_wrap", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern void Create(string modelFile);
        #endregion

        #region DestroySession
        [DllImport(LibraryName, EntryPoint = "destroy_wrap", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Destroy();
        #endregion

        #region ProcessSession
        [DllImport(LibraryName, EntryPoint = "process_wrap", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        private static extern int Process(IntPtr pImage, [Out] out IntPtr outPoint, out int outSize);
        #endregion

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct SessionPoint
        {
            public int x;
            public int y;
        }

        public enum ProcessResult
        {
            Error,
            EmptyImage,
            NotDetected,
            Success
        }

        public static string ToString(ProcessResult result)
        {
            switch (result) {
                case ProcessResult.EmptyImage:
                    return @"Ошибочное изображение";
                case ProcessResult.NotDetected:
                    return @"Не определено лицо";
                case ProcessResult.Success:
                    return @"Удачно";
                default:
                    return @"Ошибка обработки";
            }
        }

        private static readonly int Size = Marshal.SizeOf(new SessionPoint());
        private static readonly Type Type = typeof (SessionPoint);

        public static ProcessResult Process(Mat image, out SessionPoint[] points)
        {
            IntPtr outPtr;
            int outSize;
            int result;

            using (var i = image.ToImage<Bgr, byte>()) {
                result = Process(i.Ptr, out outPtr, out outSize);
            }
            points = new SessionPoint[outSize];
            for (var i = 0; i < outSize; i++) {
                points[i] = (SessionPoint)Marshal.PtrToStructure(new IntPtr(outPtr.ToInt64() + (i * Size)), Type);
            }
            switch (result) {
                case 1:
                    return ProcessResult.EmptyImage;
                case 2:
                    return ProcessResult.NotDetected;
                case 3:
                    return ProcessResult.Success;
                default:
                    return ProcessResult.Error;
            }
        }
    }
}
