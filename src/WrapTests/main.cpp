#include <Windows.h>
#include <commctrl.h>
#include <strsafe.h>
#include <Shlwapi.h>

#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/videoio/videoio_c.h>

#include <wrap.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "Winmm.lib")


#pragma warning(disable: 4996)

int 
main(int argc, char **argv)
{
    create_wrap("C:\\work\\videotools\\Debug\\model\\main_clm_general.txt");
    CvCapture *c = cvCreateFileCapture("C:\\work\\videotools\\sample.avi");
    Point *p = nullptr;
    int size = 0;
    IplImage *i;

    while ((i = cvQueryFrame(c)) != nullptr) {
        auto ret = process_wrap(i, &p, &size);
        fprintf(stdout, "[%05d]: %d\n", (int)cvGetCaptureProperty(c, CV_CAP_PROP_POS_FRAMES), ret);
    }

    cvReleaseCapture(&c);
    destroy_wrap();
    return EXIT_SUCCESS;
}