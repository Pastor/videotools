#include <Windows.h>
#include <strsafe.h>
#include "haar.h"


void
__Detect(CvHaarClassifierCascade * classifier,
	CvMemStorage* storage,
	IplImage *i,
	std::vector<cv::Rect> &faces,
	CvSeq **seqFaces,
	double scaleFactor,
	int minn,
	int flags,
	int minWidth,
	int minHeight)
{
	CvSeq *finded;
	IplImage *detect;
	IplImage *gray = nullptr;

	detect = i;
	if (i->nChannels > 1) {
		auto size = cvSize(i->width, i->height);
		gray = cvCreateImage(size, IPL_DEPTH_8U, 1);
		cvCvtColor(i, gray, CV_BGR2GRAY);
		detect = gray;
	}
	faces.clear();
	finded = cvHaarDetectObjects(detect, classifier, storage, scaleFactor, minn, flags, cvSize(minWidth, minHeight));
	if (finded != nullptr && finded->total > 0) {
		for (auto k = 0; k < finded->total; ++k) {
			auto rect = reinterpret_cast<CvRect *>(cvGetSeqElem(finded, k));
			faces.push_back(cv::Rect(rect->x, rect->y, rect->width, rect->height));
		}
	}
	if (gray != nullptr)
		cvReleaseImage(&gray);
	if (seqFaces)
		(*seqFaces) = finded;
}
