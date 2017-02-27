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
#include <opencv2/highgui.hpp>

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
	//convert frame to separate colors
	cv::Mat colors;
	cvtColor(frame, colors, CV_RGB2HSV);
	//get obstacles in image (orange)
	int satMin = 175;
	int hueCenter = 116; //lower is more orange
	int hueMin = hueCenter - 8;
	int hueMax = hueCenter + 8;
	Mat frameObstacles;
	inRange(colors, Scalar(hueMin, satMin, 40), Scalar(hueMax, 255, 255), frameObstacles);
	//get course edges (blue)
	satMin = 80;
	hueCenter = 18;
	hueMin = hueCenter - 15;
	hueMax = hueCenter + 15;
	Mat frameEdges;
	inRange(colors, Scalar(hueMin, satMin, 50), Scalar(hueMax, 255, 255), frameEdges);
	//blend the two together
	Mat combined;
	//cv::bitwise_not(frameEdges | frameObstacles, combined);
	combined = frameEdges | frameObstacles;

	//create debug img
	//cv::cvtColor(frameEdges, p_debugImg, CV_GRAY2BGR);
	cv::cvtColor(combined, p_debugImg, CV_GRAY2BGR);

    //find route without anything in
    int midPoint = combined.cols / 2;
    int prevX = midPoint;
    std::vector<int> route;
	for (int y = combined.rows - 1; y >= 0; y--) {
        //push previous point onto route
        route.push_back( prevX );

        //find first edge point in both directions
        int edgeL = prevX;
        int edgeR = prevX;

		//if next point is in an object, move it to a side
		while (combined.at<uchar>(Point(edgeL, y)) != 0) prevX++;

		//while (edgeL > 0 && frameEdges.at<uchar>(Point(edgeL, y)) == 0) {
		while (edgeL > 0 && combined.at<uchar>(Point(edgeL, y)) == 0) {
			p_debugImg.at<Vec3b>(Point(edgeL,y)) = Vec3b(255, 0, 0);
			edgeL--;
		}
		while (edgeR < (combined.cols - 1) && combined.at<uchar>(Point(edgeR, y)) == 0) {
		//while (edgeR < (frameEdges.cols - 1) && frameEdges.at<uchar>(Point(edgeR, y)) == 0) {
			p_debugImg.at<Vec3b>(Point(edgeR,y)) = Vec3b(0, 255, 0);
			edgeR++;
		}

        //check if we should bail early
        if( edgeL > midPoint || edgeR < midPoint ) 
            break;

        //find next point
        prevX = ((edgeL + edgeR)/2 + 1);
		
		//size
		int gapSize = edgeR - edgeL;
		if (gapSize < get_min_dist(y)) {
			//gap is too small, bail
			break;
		}

		//add pixel to debug image
        //p_debugImg.at<Vec3b>(Point(prevX,y)) = Vec3b(255, 0, 255);
    }

	//look at path and determine direction
	int right = 0;
	int left = 0;
	int y = frameEdges.rows - 1;
	for (int i = 0; i < route.size(); i++) {
		//get weight
		int weight = (((frameEdges.rows - i) * 10) / frameEdges.rows) + 1;
		//check if we should go left or right
		int xLoc = route.at(i);
		if (xLoc > (midPoint + 10)) {
			p_debugImg.at<Vec3b>(Point(xLoc,y)) = Vec3b(0, 255, 255);
			right += weight;
		}
		if (xLoc < (midPoint - 10)) {
			p_debugImg.at<Vec3b>(Point(xLoc,y)) = Vec3b(255, 255, 0);
			left += weight;
		}
		y--;
	}

	//check how fast we can go
	//TODO:

	//draw text on screen
	if( right > left )
		putText(p_debugImg, "Right", Point(10, 10), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 0), 1.0);
	else 
		putText(p_debugImg, "Left", Point(10, 10), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 0), 1.0);
    //go up image
    imshow("main", p_debugImg);
    waitKey();
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

void Navigate::populate_areas( int x, int y )
{
}
