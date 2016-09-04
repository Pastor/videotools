using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

namespace CaptureVideo.Library
{
    internal static class Native
    {
        private const string LibraryName = @"OpenFaceWrap.dll";

        #region CreateSession
        [DllImport(LibraryName, EntryPoint = "create_wrap", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern void Create(string modelFile);
        #endregion

        #region DestroySession
        [DllImport(LibraryName, EntryPoint = "destroy_wrap", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Destroy();
        #endregion

        #region ProcessSession
        [DllImport(LibraryName, EntryPoint = "process_wrap", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Process(IntPtr pImage, [Out] out IntPtr outPoint, out int outSize);
        #endregion
    }
}
