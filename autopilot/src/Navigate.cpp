/******************************************************************************
 * Class Implementation
 *
 * Authors: James Swift, Luke Newmeyer
 * Copyright 2017
******************************************************************************/
/*************************** Include Files ***********************************/

#include "Navigate.hpp"

#include <iostream>
#include <opencv2/imgproc.hpp>

/*************************** Definitions *************************************/

using std::cout;
using std::endl;
using std::string;
using namespace cv;

string type2str(int type);

/*************************** Implementation **********************************/

Navigate::Navigate( void )
{
#ifdef NAVIGATE_USE_LUKE
#else
	namedWindow("debugE", CV_WINDOW_KEEPRATIO);
	namedWindow("debugO", CV_WINDOW_KEEPRATIO);
#endif
}

void Navigate::analyze_frame(cv::Mat frame)
{
#ifdef NAVIGATE_USE_LUKE
	analyze_frame_luke(frame);
#else
	analyze_frame_james(frame);
#endif
}

//Notes:
//Wheel base:	1/6 width from edge of frame at bottom (1/8)
//				1/6 width of frame at top of frame (1/5)
void Navigate::analyze_frame_james(cv::Mat frame)
{
	std::vector<int> lEdge;
	std::vector<int> rEdge;
	std::vector<int> mEdge;
	std::vector<int> obj;
	std::vector<int> compromoise;

	//convert frame to separate colors
	cv::Mat colors;
	cvtColor(frame, colors, CV_RGB2HSV);

	int satMin = 175;
	int hueCenter = 116; //lower is more orange
	int hueMin = hueCenter - 8;
	int hueMax = hueCenter + 8;
	Mat frameObstacles;
	inRange(colors, Scalar(hueMin, satMin, 40), Scalar(hueMax, 255, 255), frameObstacles);

	satMin = 80;
	hueCenter = 18;
	hueMin = hueCenter - 15;
	hueMax = hueCenter + 15;
	Mat frameEdges;
	inRange(colors, Scalar(hueMin, satMin, 50), Scalar(hueMax, 255, 255), frameEdges);

	//lets just start with following a basic path (edges only)
	int midWay = (frameEdges.cols / 2) - 1;
	int lMax, rMax; //pixels closest to center
	int nextPoint;
	int prevPoint = midWay;
	int pMin = frameEdges.cols / 10;
	int pMax = (9 * frameEdges.cols) / 10;
	bool obstacleFound = false;
	int oSum;
	int oSlopex = (frameEdges.cols * 11) / 20;
	int oSlopey = frameEdges.rows;
	int oOffset = (frameEdges.cols * 1) / 5;

	//create a debug image
	Mat debugEImg;
	cvtColor(frameEdges, debugEImg, CV_GRAY2BGR);
	Mat debugOImg;
	cvtColor(frameObstacles, debugOImg, CV_GRAY2BGR);

	//from bottom to top (closest = highest priority)
	for (int y = frameEdges.rows-1; y >= 0; y--) {
		oSum = 0;
		lMax = 0;
		rMax = frameEdges.cols - 1;
		for (int x = 0; x < frameEdges.cols - 1; x++) {
			//get pixel values
			unsigned char ePix = frameEdges.at<unsigned char>(Point(x, y));
			unsigned char oPix = frameObstacles.at<unsigned char>(Point(x, y));

			//get boarder boundaries
			if (ePix > 0) {
				//check if we need to update closest pixel to previous location
				if (x < prevPoint && lMax < x ) 
					lMax = x;
				else if( x > prevPoint && rMax > x ) 
					rMax = x;
			}

			//find which side has more obstacles
			if (oPix > 0) {
				//check if on left
				if (x < prevPoint )
					oSum++; //record we want to go right (positive)
				else if (x > prevPoint )
					oSum--;
			}
		}

		//check where mid-point is
		prevPoint = (lMax + rMax) / 2;
		int oPoint = prevPoint;

		//check if we should go left or right based on the obstacles
		if (oSum != 0) {
			int width = ((y * oSlopex) / oSlopey) + oOffset; //width of car at this point
			int testX = prevPoint;
			bool foundGap = false;
			while (!foundGap) {
				//get start position
				int startX = testX - (width / 2);

				//check if window is all clear
				foundGap = true;
				for (int i = 0; i < width; i++) {
					int pLoc = startX + i;
					if (pLoc > 0 && pLoc < frameObstacles.cols) {
						//debugOImg.at<Vec3b>(Point(pLoc, y)) = Vec3b(0, 0, 255);
						unsigned char pixel = frameObstacles.at<unsigned char>(Point(pLoc, y));
						if (pixel > 0) {
							foundGap = false;
							break;
						}
					}
					else
						break;
				}
				if (foundGap == false) {
					if (oSum > 0)
						testX++;
					else
						testX--;
				}
			}
			if (testX < 0)
				oPoint = 0;
			else if (testX > frameObstacles.cols - 1)
				oPoint = frameObstacles.cols - 1;
			else
				oPoint = testX;
		}

		//mid-point
		int midPoint = (prevPoint + oPoint) / 2;

		//record l and r max values
		lEdge.push_back(lMax);
		rEdge.push_back(rMax);
		mEdge.push_back(prevPoint);
		obj.push_back(oPoint);
		compromoise.push_back(midPoint);

		//color this pixel red
		debugEImg.at<Vec3b>(Point(lMax,y)) = Vec3b(255, 0, 0);
		debugEImg.at<Vec3b>(Point(rMax,y)) = Vec3b(255, 0, 0);
		debugEImg.at<Vec3b>(Point(prevPoint,y)) = Vec3b(0, 255, 0);
		debugEImg.at<Vec3b>(Point(oPoint,y)) = Vec3b(0, 0, 255);
		debugEImg.at<Vec3b>(Point(midPoint,y)) = Vec3b(0, 255, 255);

		debugOImg.at<Vec3b>(Point(lMax,y)) = Vec3b(255, 0, 0);
		debugOImg.at<Vec3b>(Point(rMax,y)) = Vec3b(255, 0, 0);
		debugOImg.at<Vec3b>(Point(prevPoint,y)) = Vec3b(0, 255, 0);
		debugOImg.at<Vec3b>(Point(oPoint,y)) = Vec3b(0, 0, 255);
		debugOImg.at<Vec3b>(Point(midPoint,y)) = Vec3b(0, 255, 255);
	}

	//determine direction

	//show image
	imshow("debugE", debugEImg);
	imshow("debugO", debugOImg);
	waitKey(1);
}

void Navigate::analyze_frame_luke(cv::Mat frame)
{
}

string type2str(int type) {
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:  r = "8U"; break;
	case CV_8S:  r = "8S"; break;
	case CV_16U: r = "16U"; break;
	case CV_16S: r = "16S"; break;
	case CV_32S: r = "32S"; break;
	case CV_32F: r = "32F"; break;
	case CV_64F: r = "64F"; break;
	default:     r = "User"; break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}
