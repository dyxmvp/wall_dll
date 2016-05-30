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

	_declspec (dllexport) int imageCalibrate_wall(CINEHANDLE cineHandle, int imageN, int imageH, int imageW, int YM, int X0_def, int Y0_def, int Xlength_def, int Ylength_def);
}

Mat Morphology_Operations(Mat dst_binary, int morph_operator, int morph_elem, int morph_size);

int wall(CINEHANDLE cineHandle, int imageN, int imageH, int imageW, int YM, int X0_def, int Y0_def, int Xlength_def, int Ylength_def);
double GetMedian(double daArray[], int iSize);   // get median intensity


_declspec (dllexport) int imageCalibrate_wall(CINEHANDLE cineHandle, int imageN, int imageH, int imageW, int YM, int X0_def, int Y0_def, int Xlength_def, int Ylength_def)
{
	int Wall;
	
	PhGetCineInfo(cineHandle, GCI_MAXIMGSIZE, (PVOID)&imgSizeInBytes);
	m_pImageBuffer = (PBYTE)_aligned_malloc(imgSizeInBytes, 16);
	Wall = wall(cineHandle, imageN, imageH, imageW, YM, X0_def, Y0_def, Xlength_def, Ylength_def);
	
	return Wall;
}

// Get median of intensity
double GetMedian(double daArray[], int iSize) {
	// Allocate an array of the same size and sort it.
	double* dpSorted = new double[iSize];
	for (int i = 0; i < iSize; ++i) {
		dpSorted[i] = daArray[i];
	}
	for (int i = iSize - 1; i > 0; --i) {
		for (int j = 0; j < i; ++j) {
			if (dpSorted[j] > dpSorted[j + 1]) {
				double dTemp = dpSorted[j];
				dpSorted[j] = dpSorted[j + 1];
				dpSorted[j + 1] = dTemp;
			}
		}
	}

	// Middle or average of middle values in the sorted array.
	double dMedian = 0.0;
	if ((iSize % 2) == 0) {
		dMedian = (dpSorted[iSize / 2] + dpSorted[(iSize / 2) - 1]) / 2.0;
	}
	else {
		dMedian = dpSorted[iSize / 2];
	}
	delete[] dpSorted;
	return dMedian;
}

int wall(CINEHANDLE cineHandle, int imageN, int imageH, int imageW, int YM, int X0_def, int Y0_def, int Xlength_def, int Ylength_def)
{
	IH imgHeader;
	IMRANGE imrange;

	int Wall = 0;
	imrange.First = imageN;
	imrange.Cnt = 1;

	//read cine image into the buffer
	PhGetCineImage(cineHandle, &imrange, m_pImageBuffer, imgSizeInBytes, &imgHeader);
	Mat image = Mat(imageH, imageW, CV_8U, m_pImageBuffer);

	Mat imcrop(image, Rect(X0_def, Y0_def, Xlength_def, Ylength_def));
	Mat dimage;
	equalizeHist(imcrop, dimage);
	imshow("wallImage", dimage);

	int ym = YM;  // window coordinate in y_middle
	
	const int msize_b = 6; // length of intensity to get median of background intensity
	double I_b[msize_b] = { 0.0 };

	for (int i = 0; i < 6; ++i)
	{
		Scalar intensity = dimage.at<uchar>(ym, i);
		I_b[i] = intensity.val[0];
	}

	int backIntensity;  // backgroud intensity
	backIntensity = GetMedian(I_b, msize_b);

	int cutI = backIntensity * 0.3;  // threshold to find wall
	double th_t = 0; // temp threshold to find wall
	const int msize_w = 3;
	double I_w[msize_w] = { 0.0 }; // intensity to find wall

	for (int x = Xlength_def - 10; x < Xlength_def; x++)
	{
		I_w[0] = dimage.at<uchar>(ym, x);
		I_w[1] = dimage.at<uchar>(ym - 1, x);
		I_w[2] = dimage.at<uchar>(ym + 1, x);

		th_t = GetMedian(I_w, msize_w);

		if (th_t < cutI)
		{
			Wall = x;
			break;
		}
	}

	return Wall;
}