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

typedef enum DIR {
	RIGHT = 0,
	UP,
	LEFT,
	DOWN
} DIR_T;

/*************************** Implementation **********************************/

Navigate::Navigate( void )
{
#ifdef NAVIGATE_USE_LUKE
#else
	//namedWindow("debugE", CV_WINDOW_KEEPRATIO);
	//namedWindow("debugO", CV_WINDOW_KEEPRATIO);
	namedWindow("main", CV_WINDOW_KEEPRATIO);
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

	//get obstacles in image (orange)
	int satMin = 175;
	int hueCenter = 116; //lower is more orange
	int hueMin = hueCenter - 8;
	int hueMax = hueCenter + 8;
	int valMin = 40;
	int valMax = 255;
	Mat frameObstacles;
	inRange(colors, Scalar(hueMin, satMin, valMin), Scalar(hueMax, 255, valMax), frameObstacles);
	//get course edges (blue)
	satMin = 100;
	hueCenter = 18;
	hueMin = hueCenter - 14;
	hueMax = hueCenter + 14;
	valMin = 50;
	valMax = 255;
	Mat frameEdges;
	inRange(colors, Scalar(hueMin, satMin, valMin), Scalar(hueMax, 255, valMax), frameEdges);
	//blend the two together
	Mat combined;
	cv::bitwise_not(frameEdges | frameObstacles, combined);

	//create debug img
	cv::cvtColor(combined, p_debugImg, CV_GRAY2BGR);

	//scan drive path for obstacles. Report first obstacle found.
	int xEnd = combined.cols - 1;
	int yEnd = 0;
	int minDist;
	int xStart;
	for (int y = combined.rows - 1; y >= yEnd; y--) {
		//only scan drive path, ignore rest
		minDist = get_min_dist(y); //get drive path width
		xStart = (combined.cols / 2) - (minDist/2); 
		//scan drive path for objects
		for (int x = xStart; x < (xStart + minDist); x++) {
			//check for object
			if (combined.at<uchar>(Point(x, y)) == 0) {
				xEnd = x;
				yEnd = y;
				break;
			}
			else {
				p_debugImg.at<Vec3b>(Point(x, y)) = Vec3b(255, 0, 255);
			}
		}
	}

	//DEBUG variables
	Point mid= Point(combined.cols / 2, yEnd);
	Point end;
	Scalar color;

	//check where we're at
	int backupY = combined.rows - NAVIGATE_BACKUP_ROW;
	int leftY = combined.rows - NAVIGATE_LEFT_ROW;

	//update speed and direction
	if (yEnd > backupY ) {
		//if its not critical, lets just go left (on far right side)
		if (yEnd < (combined.rows - ((NAVIGATE_BACKUP_ROW / 2)) ) &&
			xEnd >= (xStart + ((minDist * 9) / 10))) {
			direction = -NAVIGATE_MAX_DIRECTION;
			speed = NAVIGATE_MAX_SPEED/10;
		}
		else {
			//we're going backwards, we will always go right when going backwards
			direction = NAVIGATE_MAX_DIRECTION;
			speed = -(NAVIGATE_MAX_SPEED / 10);
			//mid = Point(0, yEnd);
		}
		color = Scalar(0, 0, 255);
	}
	else if (yEnd > leftY ) {
		direction = (-NAVIGATE_MAX_DIRECTION * (yEnd - leftY) / (backupY - leftY));
		speed = NAVIGATE_MAX_SPEED/5;
		color = Scalar(0, 255, 0);
	}
	else {
		direction = (NAVIGATE_MAX_DIRECTION * (leftY - yEnd) / leftY);
		speed = NAVIGATE_MAX_SPEED/2;
		//if object was found on right side, don't turn hard
		if (xEnd > ((combined.cols / 2)-4) )
			direction /= 3;

		//we're going fast, cap max direction
		if (direction > (NAVIGATE_MAX_DIRECTION / 4))
			direction = (NAVIGATE_MAX_DIRECTION / 4);

		//we're going fast, cap the max direction
		color = Scalar(255, 0, 0);
	}

	//draw line (scale max dirVal to helf-width of image)
	int scaledVal = ((direction * 79) / NAVIGATE_MAX_DIRECTION);
	cout << "dirVal: " << direction << " scaled: " << scaledVal << endl;
	cout << "speedVal: " << speed << endl;
	end = Point((combined.cols/2) + scaledVal, yEnd);
	line(p_debugImg, mid, end, color, 1, 8, 0);
	
	//go up image
	imshow("main", p_debugImg);
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

//Wheel base:	1/6 width from edge of frame at bottom (1/8)
//				1/6 width of frame at top of frame (1/5)
// 3/4 image width at bottom
// 1/5 image width at top
// image 160x90
int Navigate::get_min_dist(int y)
{
	int topW = (160 / 6);
	int botW = ((160 * 3) / 4) - 20;
	int diff = botW - topW;

	int dist = ((( y * diff ) / 90) + topW);

	return (dist);
}
