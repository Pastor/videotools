/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef __OPENCV_SUPERRES_HPP__
#define __OPENCV_SUPERRES_HPP__

#include "opencv2/core.hpp"

/**
  @defgroup superres Super Resolution

The Super Resolution module contains a set of functions and classes that can be used to solve the
problem of resolution enhancement. There are a few methods implemented, most of them are descibed in
the papers @cite Farsiu03 and @cite Mitzel09 .

 */

namespace cv
{
    namespace superres
    {

//! @addtogroup superres
//! @{

        CV_EXPORTS bool initModule_superres();

        class CV_EXPORTS FrameSource
        {
        public:
            virtual ~FrameSource();

            virtual void nextFrame(OutputArray frame) = 0;
            virtual void reset() = 0;
        };

        CV_EXPORTS Ptr<FrameSource> createFrameSource_Empty();

        CV_EXPORTS Ptr<FrameSource> createFrameSource_Video(const String& fileName);
        CV_EXPORTS Ptr<FrameSource> createFrameSource_Video_CUDA(const String& fileName);

        CV_EXPORTS Ptr<FrameSource> createFrameSource_Camera(int deviceId = 0);

        /** @brief Base class for Super Resolution algorithms.

        The class is only used to define the common interface for the whole family of Super Resolution
        algorithms.
         */
        class CV_EXPORTS SuperResolution : public cv::Algorithm, public FrameSource
        {
        public:
            /** @brief Set input frame source for Super Resolution algorithm.

            @param frameSource Input frame source
             */
            void setInput(const Ptr<FrameSource>& frameSource);

            /** @brief Process next frame from input and return output result.

            @param frame Output result
             */
            void nextFrame(OutputArray frame);
            void reset();

            /** @brief Clear all inner buffers.
            */
            virtual void collectGarbage();

        protected:
            SuperResolution();

            virtual void initImpl(Ptr<FrameSource>& frameSource) = 0;
            virtual void processImpl(Ptr<FrameSource>& frameSource, OutputArray output) = 0;

            bool isUmat_;

        private:
            Ptr<FrameSource> frameSource_;
            bool firstCall_;
        };

        /** @brief Create Bilateral TV-L1 Super Resolution.

        This class implements Super Resolution algorithm described in the papers @cite Farsiu03 and
        @cite Mitzel09 .

        Here are important members of the class that control the algorithm, which you can set after
        constructing the class instance:

        -   **int scale** Scale factor.
        -   **int iterations** Iteration count.
        -   **double tau** Asymptotic value of steepest descent method.
        -   **double lambda** Weight parameter to balance data term and smoothness term.
        -   **double alpha** Parameter of spacial distribution in Bilateral-TV.
        -   **int btvKernelSize** Kernel size of Bilateral-TV filter.
        -   **int blurKernelSize** Gaussian blur kernel size.
        -   **double blurSigma** Gaussian blur sigma.
        -   **int temporalAreaRadius** Radius of the temporal search area.
        -   **Ptr\<DenseOpticalFlowExt\> opticalFlow** Dense optical flow algorithm.
         */
        CV_EXPORTS Ptr<SuperResolution> createSuperResolution_BTVL1();
        CV_EXPORTS Ptr<SuperResolution> createSuperResolution_BTVL1_CUDA();
        CV_EXPORTS Ptr<SuperResolution> createSuperResolution_BTVL1_OCL();

//! @} superres

    }
}

#endif // __OPENCV_SUPERRES_HPP__
