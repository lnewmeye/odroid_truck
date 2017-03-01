/******************************************************************************
 * Class Implementation
 *
 * Authors: James Swift, Luke Newmeyer
 * Copyright 2017
 ******************************************************************************/
/*************************** Include Files ***********************************/

#include "Navigate.hpp"

#include <math.h>
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

/*************************** Definitions *************************************/

//steering
#define EXP_FACTOR 0.06
#define EXP_MULTIPLIER 9.0
#define STEERING_SENSITIVITY 5

//speed
#define SPEED_DIST_FROM_CENTER 9
#define SPEED_DIST0 70
#define SPEED_DIST1 30
#define SPEED_DIST2 15
#define SPEED_DIST3 7

#define SPEED_VAL0 15
#define SPEED_VAL1 13
#define SPEED_VAL2 10
#define SPEED_VAL3 9
#define SPEED_VAL_BAK -12
//low=0.6, med=0.8, high=1.0
#define SPEED_FACTOR 0.7

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
	namedWindow("main", CV_WINDOW_KEEPRATIO);
	bailCnt = 0;
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
	int valMin = 60;
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
	//cv::bitwise_not(frameEdges | frameObstacles, combined);
	combined = frameEdges | frameObstacles;
	//imshow( "edges", frameEdges );
	//imshow( "obstacles", frameObstacles );

	//create debug img
	//cv::cvtColor(frameEdges, p_debugImg, CV_GRAY2BGR);
	cv::cvtColor(combined, p_debugImg, CV_GRAY2BGR);

	//find route without anything in it
	int midPoint = combined.cols / 2;
	int prevX = midPoint;
	std::vector<int> route;
	bool problem = false;
	int y;
	for ( y = combined.rows - 1; y >= 0; y--) {
		//if next point is inside an object, we have a problem
		if( prevX >= 0 && combined.at<uchar>(Point(prevX, y) ) != 0 ) {
			problem = true;
			break;
		}

		if (prevX < 0)
			break;

		//find first edge point in both directions
		int edgeL = prevX;
		int edgeR = prevX;

		//while (edgeL > 0 && frameEdges.at<uchar>(Point(edgeL, y)) == 0) {}
		while (edgeL > 0 && combined.at<uchar>(Point(edgeL, y)) == 0) {
			p_debugImg.at<Vec3b>(Point(edgeL,y)) = Vec3b(255, 0, 0);
			edgeL--;
		}
		while (edgeR < (combined.cols - 1) && combined.at<uchar>(Point(edgeR, y)) == 0) {
			p_debugImg.at<Vec3b>(Point(edgeR,y)) = Vec3b(0, 255, 0);
			edgeR++;
		}
		Vec3b pt = frameObstacles.at<Vec3b>(Point(edgeL,y));
#if 0
		printf( "objValL(%i,%i): %i %i %i\n", 
				Point(edgeL,y).x,
				Point(edgeL,y).y,
				pt(0),
				pt(1),
				pt(2) );
#endif

		//find next point
		prevX = ((edgeL + edgeR)/2 + 1);

		//size
		int gapSize = edgeR - edgeL;
		if (gapSize < get_min_dist(y) && y != combined.rows - 1 ) {
			//gap is too small, bail
			break;
		}

		//push previous point onto route
		route.push_back( prevX );
	}

	//look at path and determine direction
	int dir = 0;
	int divisor = 0;
	speed = route.size();
	for (int i = 0; i < route.size(); i++) {
		//get direction we should go (weight according to where we are in image)
		int xLoc = route.at(i);
		int weight = (int)((EXP_MULTIPLIER / exp(EXP_FACTOR * i)) + 1);
		int distFromCenter = xLoc - midPoint;
		dir += (weight*distFromCenter);
		//keep track of our weight so we can divide accurately
		divisor += weight;

		//check if we should set the speed (non-middle point)
		if (speed == route.size() && i != 0 ) {
			if (distFromCenter > SPEED_DIST_FROM_CENTER || 
					distFromCenter < -SPEED_DIST_FROM_CENTER ) {
				speed = i;
				//cout << "Speed: " << i << endl;
			}
		}
	}

	//scale direction to weighted avg pixels per row
	dir = dir / divisor;
	//since we find the mid-point the maximum 1 direciton can be is 
	//1/4 the image, scale this to be between -100 and 100
	direction = (dir * 100)/STEERING_SENSITIVITY;
	if( direction > 100 )
		direction = 100;
	else if( direction < -100 )
		direction = -100;

	//change speed
	if (speed > SPEED_DIST0)
		speed = SPEED_VAL0;
	else if (speed > SPEED_DIST1)
		speed = SPEED_VAL1;
	else if (speed > SPEED_DIST2)
		speed = SPEED_VAL2;
	else if (speed > SPEED_DIST3)
		speed = SPEED_VAL3;
	else {
		speed = SPEED_VAL_BAK;
		direction = -direction;
	}
	speed = (int)((float)speed * SPEED_FACTOR);

	//increase speed if direction is sharp	
	if( direction > 50 )
		speed = (speed * 4 ) / 3;
	else if( direction < -50 )
		speed = (speed * 4 ) / 3;

	confidenceRow = y;
	//check if we bailed early
	if( y > ((combined.rows*3)/4) ) {
		//draw steering text on screen
		std::string pSteering = "Bail!";
		putText(p_debugImg, pSteering, Point(10, 20), FONT_HERSHEY_PLAIN, 
				1.0, CV_RGB(255, 0, 0), 1.0);
		bailCnt++;
		bailDirectionSet = false;
		speed = 0;
		direction = 0;
	}else {
		bailCnt = 0;
	}

	//draw steering text on screen
	std::string pSteering = "Direction: ";
	pSteering.append( std::to_string( direction ) );
	putText(p_debugImg, pSteering, Point(10, 10), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 0, 0), 1.0);

	//draw drive text on screen
	std::string pDrive= "Speed    : ";
	pDrive.append( std::to_string( speed ) );
	putText(p_debugImg, pDrive, Point(10, 30), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 0, 0), 1.0);

	//go up image
	imshow("main", p_debugImg);
}
//////////////////////

void Navigate::analyze_frame_luke(cv::Mat frame)
{
}
	
void Navigate::analyze_bail(cv::Mat frame)
{
	//convert frame to separate colors
	cv::Mat colors;
	cvtColor(frame, colors, CV_RGB2HSV);
	//get obstacles in image (orange)
	int satMin = 175;
	int hueCenter = 116; //lower is more orange
	int hueMin = hueCenter - 8;
	int hueMax = hueCenter + 8;
	int valMin = 60;
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
	frameEdges.copyTo(combined);
	//combined = frameEdges | frameObstacles;
	
	//create debug image
	cv::cvtColor(combined, p_debugImg, CV_GRAY2BGR);

	//find route without anything in it
	int midPoint = combined.cols / 2;
	int prevX = midPoint;
	std::vector<int> route;
	bool problem = false;
	int y;
	for ( y = combined.rows - 1; y >= 0; y--) {
		//if next point is inside an object, we have a problem
		if( prevX >= 0 && combined.at<uchar>(Point(prevX, y) ) != 0 ) {
			problem = true;
			break;
		}

		if (prevX < 0)
			break;

		//find first edge point in both directions
		int edgeL = prevX;
		int edgeR = prevX;

		//while (edgeL > 0 && frameEdges.at<uchar>(Point(edgeL, y)) == 0) {}
		while (edgeL > 0 && combined.at<uchar>(Point(edgeL, y)) == 0) {
			p_debugImg.at<Vec3b>(Point(edgeL,y)) = Vec3b(255, 0, 0);
			edgeL--;
		}
		while (edgeR < (combined.cols - 1) && combined.at<uchar>(Point(edgeR, y)) == 0) {
			p_debugImg.at<Vec3b>(Point(edgeR,y)) = Vec3b(0, 255, 0);
			edgeR++;
		}
		Vec3b pt = frameObstacles.at<Vec3b>(Point(edgeL,y));
#if 0
		printf( "objValL(%i,%i): %i %i %i\n", 
				Point(edgeL,y).x,
				Point(edgeL,y).y,
				pt(0),
				pt(1),
				pt(2) );
#endif

		//find next point
		prevX = ((edgeL + edgeR)/2 + 1);

		//size
		int gapSize = edgeR - edgeL;
		if (gapSize < get_min_dist(y) && y != combined.rows - 1 ) {
			//gap is too small, bail
			break;
		}

		//push previous point onto route
		route.push_back( prevX );
	}

	//look at path and determine direction
	int dir = 0;
	int divisor = 0;
	speed = route.size();
	for (int i = 0; i < route.size(); i++) {
		//get direction we should go (weight according to where we are in image)
		int xLoc = route.at(i);
		int weight = (int)((EXP_MULTIPLIER / exp(EXP_FACTOR * i)) + 1);
		int distFromCenter = xLoc - midPoint;
		dir += (weight*distFromCenter);
		//keep track of our weight so we can divide accurately
		divisor += weight;

		//check if we should set the speed (non-middle point)
		if (speed == route.size() && i != 0 ) {
			if (distFromCenter > SPEED_DIST_FROM_CENTER || 
					distFromCenter < -SPEED_DIST_FROM_CENTER ) {
				speed = i;
				//cout << "Speed: " << i << endl;
			}
		}
	}

	//scale direction to weighted avg pixels per row
	dir = dir / divisor;
	//since we find the mid-point the maximum 1 direciton can be is 
	//1/4 the image, scale this to be between -100 and 100
	direction = (dir * 100)/STEERING_SENSITIVITY;
	if( direction > 100 )
		direction = 100;
	else if( direction < -100 )
		direction = -100;

	//change speed
	if (speed > SPEED_DIST0)
		speed = SPEED_VAL0;
	else if (speed > SPEED_DIST1)
		speed = SPEED_VAL1;
	else if (speed > SPEED_DIST2)
		speed = SPEED_VAL2;
	else if (speed > SPEED_DIST3)
		speed = SPEED_VAL3;
	else {
		speed = SPEED_VAL_BAK;
		direction = -direction;
	}

	//increase speed if direction is sharp	
	if( direction > 50 )
		speed = (speed * 4 ) / 3;
	else if( direction < -25 )
		speed = (speed * 4 ) / 3;

	//set confidence
	confidenceRow = y;

	//check if we need to determine a bail direction
	if( !bailDirectionSet ) {
		bailDirectionSet = true;
		if( direction < 0 )
			//we want to go right
			bailDirectionRight = true;
		else
			//we want to go left
			bailDirectionRight = false;
	}

	//check if we 
	if( (bailDirectionRight && direction > 10 ) ) 
		bailCnt = 0;
	else if( (!bailDirectionRight && direction < -10 ) ) 
		bailCnt = 0;
	else {
		//set direction and speed	
		if( bailDirectionRight ) 
			direction = 100;
		else 
			direction = -100;
		speed = SPEED_VAL_BAK;
	}


	//draw bail text on screen
	std::string pBail = "Bail!";
	putText(p_debugImg, pBail, Point(10, 20), FONT_HERSHEY_PLAIN, 
			1.0, CV_RGB(255, 0, 0), 1.0);

	//draw steering text on screen
	std::string pSteering = "Direction: ";
	pSteering.append( std::to_string( direction ) );
	putText(p_debugImg, pSteering, Point(10, 10), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 0, 0), 1.0);

	//draw drive text on screen
	std::string pDrive= "Speed    : ";
	pDrive.append( std::to_string( speed ) );
	putText(p_debugImg, pDrive, Point(10, 30), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 0, 0), 1.0);

	//go up image
	imshow("main", p_debugImg);
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
