#pragma once
#include <opencv2/core/core_c.h>
#include <opencv2/core.hpp>
#include <opencv2/objdetect/objdetect_c.h>

#define OBJECT_MINSIZE   80

void
__Detect(CvHaarClassifierCascade * classifier,
	CvMemStorage* storage,
	IplImage *i,
	std::vector<cv::Rect> &faces,
	CvSeq **seqFaces = nullptr,
	double scaleFactor = 1.1,
	int minn = 6,
	int flags = 0 | CV_HAAR_DO_ROUGH_SEARCH | CV_HAAR_FIND_BIGGEST_OBJECT,
	int minWidth = OBJECT_MINSIZE,
	int minHeight = OBJECT_MINSIZE);
