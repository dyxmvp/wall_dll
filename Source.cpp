// 07/02/2015
// This code is to calibrate cell deformability

// 08/17/2015
// This code will read the image from the camera RAM

// 11/22/2015
// This code is to obtain intensity of the image

// 12/01/2015
// This code is to find the position of the wall


#include "stdafx.h"
#include "PhCon.h"
#include "PhInt.h"
#include "PhFile.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using namespace cv;
using namespace std;

UINT imgSizeInBytes;
PBYTE m_pImageBuffer;

extern "C" {

	_declspec (dllexport) int imageCalibrate_wall(CINEHANDLE cineHandle, int imageN, int imageH, int imageW, int mIntensity, int YM);
}

Mat Morphology_Operations(Mat dst_binary, int morph_operator, int morph_elem, int morph_size);

int wall(CINEHANDLE cineHandle, int imageN, int imageH, int imageW, int mIntensity, int YM);


_declspec (dllexport) int imageCalibrate_wall(CINEHANDLE cineHandle, int imageN, int imageH, int imageW, int mIntensity, int YM)
{
	int Wall;
	
	PhGetCineInfo(cineHandle, GCI_MAXIMGSIZE, (PVOID)&imgSizeInBytes);
	m_pImageBuffer = (PBYTE)_aligned_malloc(imgSizeInBytes, 16);
	Wall = wall(cineHandle, imageN, imageH, imageW, mIntensity, YM);
	
	return Wall;
}



int wall(CINEHANDLE cineHandle, int imageN, int imageH, int imageW, int mIntensity, int YM)
{
	IH imgHeader;
	IMRANGE imrange;

	int Wall = 0;
	imrange.First = imageN;
	imrange.Cnt = 1;

	//read cine image into the buffer
	PhGetCineImage(cineHandle, &imrange, m_pImageBuffer, imgSizeInBytes, &imgHeader);
    Mat image = Mat(imageH, imageW, CV_8U, m_pImageBuffer);

	/*Intensity*/
	///
	for (int x = imageW; x > imageW - 10; --x)
	{
		Scalar intensity = image.at<uchar>(YM, x);
		if (intensity.val[0] < 0.5 * mIntensity)
		{
			Scalar intensity1 = image.at<uchar>(YM, x - 1);
			Scalar intensity2 = image.at<uchar>(YM, x - 2);
			Scalar intensity3 = image.at<uchar>(YM, x - 3);
			Scalar intensity4 = image.at<uchar>(YM, x - 4);

			if (intensity1.val[0] > 0.8 * mIntensity)
			{
				Wall = x;
				break;
			}

			if (intensity2.val[0] > 0.8 * mIntensity)
			{
				Wall = x - 1;
				break;
			}

			if (intensity3.val[0] > 0.8 * mIntensity)
			{
				Wall = x - 2;
				break;
			}

			if (intensity4.val[0] > 0.8 * mIntensity)
			{
				Wall = x - 3;
				break;
			}

		}
	}
	
	return Wall;
	///
}
