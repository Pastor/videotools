#include "mainwindow.h"
#include <QApplication>

#include <opencv2/core/ocl.hpp>

int main(int argc, char *argv[])
{
#ifdef OPENCL_SPEEDUP
    cv::ocl::Context context;
    if (!context.create(cv::ocl::Device::TYPE_GPU))
        qWarning("Failed creating the context...");
    else
        qWarning("%d GPU devices are detected", context.ndevices());
    for (int i = 0; i < context.ndevices(); i++)
    {
        cv::ocl::Device device = context.device(i);
        qWarning( "Name                 : %s ", device.name());
        qWarning( "Available            : %d", device.available());
        qWarning( "ImageSupport         : %d", device.imageSupport());
        qWarning( "OpenCL_C_Version     : %s", device.OpenCL_C_Version());
    }
    cv::ocl::setUseOpenCL(true);
    qWarning(cv::ocl::useOpenCL() ? "OpenCL is used" : "OpenCL is not used");
#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
