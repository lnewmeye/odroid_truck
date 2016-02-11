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
//higher is less sensitive
#define STEERING_SENSITIVITY 9.5
//#define STEERING_SENSITIVITY 5.0
#define OBJ_IN_ROW_MULTIPLIER 2

//speed
#define SPEED_DIST_FROM_CENTER 9
#define SPEED_DIST0 70
#define SPEED_DIST1 30
#define SPEED_DIST2 15
#define SPEED_VAL0 15
#define SPEED_VAL1 13
#define SPEED_VAL2 10
#define SPEED_VAL3 9
#define SPEED_VAL_BAK -18
//battery level: high=0.6, med=0.8, low=1.0
#define SPEED_FACTOR 0.72

//bailing
#define BAIL_DISTANCE_FACTOR_TO_BAIL 0.4
#define BAIL_PORTION_BEFORE_TURN 0.7
#define BAIL_CENTER_OFFSET 0.25

#define WRITE_VIDEO_NAME "video_navigate.avi"
#define VIDEO_RATE 30

using std::cout;
using std::endl;
using std::string;
using namespace cv;

typedef enum EDGE_TYPE_E {
	EDGE_TYPE_EDGE = 0,
	EDGE_TYPE_OBJECT,
	EDGE_TYPE_IMG
} EDGE_TYPE_T;

/*************************** Implementation **********************************/

Navigate::Navigate( void )
{
	namedWindow("main", CV_WINDOW_KEEPRATIO);
	cout << "Creating Navigate Object!" << endl;
	p_bailCnt = 0;
	debugMode = false;
	showObjects = false;
	showEdges = false;
	p_writeVideo = false;
}

void Navigate::analyze_frame(cv::Mat frame)
{
	switch (p_navState) {
	case NAV_STATE_FORWARD:
		//analyze frame
		analyze_forward(frame);
		//check if it wants us to bail
		if (p_bail) {
			p_bailCnt++;
			cout << "BailCnt: " << p_bailCnt << endl;
			//check if we wanted to bail 5 times in a row
			if (p_bailCnt == 5) {
				cout << "Changing to bail state." << endl;
				p_bailCnt = 0;
				p_navState = NAV_STATE_BAIL;
			}
		}
		else {
			p_bailCnt = 0;
		}
		break;

	case NAV_STATE_BAIL:
		//analyze bail frame
		analyze_bail(frame);
		//check if it wants us to bail
		if (!p_bail) {
			p_bailCnt++;
			cout << "BailCnt: " << p_bailCnt << endl;
			//check if we wanted to bail 5 times in a row
			if (p_bailCnt == 5) {
				cout << "Changing to forward state." << endl;
				p_bailCnt = 0;
				p_navState = NAV_STATE_FORWARD;
			}
		}
		else {
			p_bailCnt = 0;
		}
		break;
	}
}

//Notes:
//Wheel base:	1/6 width from edge of frame at bottom (1/8)
//				1/6 width of frame at top of frame (1/5)
void Navigate::analyze_forward(cv::Mat frame)
{
	//convert frame to HSV Color Space
	cv::Mat colors;
	cvtColor(frame, colors, CV_RGB2HSV);

	//get obstacle and edge images
	Mat frameObstacles;
	Mat frameEdges;
	get_obstacles(colors, &frameObstacles);
	get_edges(colors, &frameEdges);

	//blend the two together
	//cv::bitwise_not(frameEdges | frameObstacles, combined);
	//create debug img
	//cv::cvtColor(frameEdges, p_debugImg, CV_GRAY2BGR);
	//cv::cvtColor((frameEdges | frameObstacles), p_debugImg, CV_GRAY2BGR);
	frame.copyTo(p_debugImg);

	//find best route in image
	int midPoint = frameEdges.cols / 2;
	std::vector<int> route;
	std::vector<bool> objInRow;
	int prevX = midPoint;
	int y;
	bool nextBail = false;
	for ( y = frameEdges.rows - 1; y >= 0; y--) {
		int targetX = prevX;
		bool objInThisRow = false;
		//ensure we're not at an edge 
		if (frameEdges.at<uchar>(Point(prevX, y)) == 0) {
			//check that we're not at an obstacle
			if (frameObstacles.at<uchar>(Point(prevX, y)) == 0) {
				//find first edge point in both directions
				int edgeL = prevX;
				int edgeR = prevX;
				while (edgeL > 0 &&
					frameEdges.at<uchar>(Point(edgeL, y)) == 0 &&
					frameObstacles.at<uchar>(Point(edgeL, y)) == 0) {
					p_debugImg.at<Vec3b>(Point(edgeL, y)) = Vec3b(255, 0, 255);
					edgeL--;
				}
					if( writeVideoVerbose && p_writeVideo && 
							p_debugImg.size() == p_videoSize ) {
						cout << "'";
						p_video << p_debugImg;
					}
				while (edgeR < (frameEdges.cols - 1) &&
					frameEdges.at<uchar>(Point(edgeR, y)) == 0 &&
					frameObstacles.at<uchar>(Point(edgeR, y)) == 0) {
					p_debugImg.at<Vec3b>(Point(edgeR, y)) = Vec3b(255, 255, 0);
					edgeR++;
				}
					if( writeVideoVerbose && p_writeVideo && 
							p_debugImg.size() == p_videoSize ) {
						cout << ".";
						p_video << p_debugImg;
					}

				//classify edge types and weight accordingly
				EDGE_TYPE_T edgeLType = EDGE_TYPE_IMG;
				EDGE_TYPE_T edgeRType = EDGE_TYPE_IMG;
				float lWeight = 10.0;
				float rWeight = 10.0;
				if (frameObstacles.at<uchar>(Point(edgeL, y)) != 0) {
					lWeight = 6.0;
					edgeLType = EDGE_TYPE_OBJECT;
					objInThisRow = true;
				}
				else if (frameEdges.at<uchar>(Point(edgeL, y)) != 0) {
					lWeight = 9.0;
					edgeLType = EDGE_TYPE_EDGE;
				}
				if (frameObstacles.at<uchar>(Point(edgeR, y)) != 0) {
					rWeight = 7.0;
					edgeRType = EDGE_TYPE_OBJECT;
					objInThisRow = true;
				}
				else if (frameEdges.at<uchar>(Point(edgeR, y)) != 0) {
					rWeight = 9.0;
					edgeRType = EDGE_TYPE_EDGE;
				}

				//get midpoint between points
				targetX = (int)((((lWeight * (float)edgeL) + (rWeight * (float)edgeR)) / (rWeight + lWeight)) + 1.0);
				//printf( "weighted edge ((l+r)/w): ((%f * %d) + (%f * %d))/(%f + %f)\n",
					//lWeight, edgeL, rWeight, edgeR, rWeight, lWeight);

				//check if our car doesn't fit through here
				int gapSize = edgeR - edgeL;
				if (gapSize < get_min_dist(y) && y != frameEdges.rows - 1) {
					//gap is too small, check if we're between and edge and an object
					if ((edgeLType == EDGE_TYPE_EDGE && edgeRType == EDGE_TYPE_OBJECT) ||
						(edgeLType == EDGE_TYPE_OBJECT && edgeRType == EDGE_TYPE_EDGE)) {
#if 0
						//lets flip everything on this side of the image to the other side of the image
						//are we currently on the right?
						bool rightSide = true;;
						if (targetX < midPoint)
							rightSide = false;
						//flip everything until the route wasn't on this side
						for (int i = route.size() - 1; i >= 0; i--) {
							//check if we got back to the midpoint
							if (rightSide && route.at(i) < midPoint)
								break;
							else if (!rightSide && route.at(i) > midPoint)
								break;

							route.at(i) = 2 * midPoint - route.at(i);
							p_debugImg.at<Vec3b>(Point(route.at(i), frameEdges.rows - 1 - i)) = Vec3b(0, 255, 255);
						}
#endif
						//only bail if we're at least somewhat close to the object
						cout << "Cannot fit between edge and object at " << y << endl;
						if (y > (int)((float)frameEdges.rows * BAIL_DISTANCE_FACTOR_TO_BAIL ) ) {
							//OK, it's close. We should bail.
							nextBail = true;
							//set bail direction
							if (edgeLType == EDGE_TYPE_EDGE && edgeRType == EDGE_TYPE_OBJECT)
								p_bailToTheRight = true;
							else
								p_bailToTheRight = false;
							//set bail state
							p_bailState = NAV_BAIL_STATE_BACKUP;
						}
						else {
							cout << "Not close enough to bail." << endl;
						}

						//stop anlyzing further away, we can't get there
						break;
					}
					//gap size is too small, but its not an object-edge scenario
					else {
						//dont bail, just set target x to image edge
						if (edgeLType == EDGE_TYPE_EDGE && edgeRType == EDGE_TYPE_IMG)
							targetX = edgeR;
						else if (edgeRType == EDGE_TYPE_EDGE && edgeLType == EDGE_TYPE_IMG)
							targetX = edgeL;
					}
				}
				//gap size not too small
				else {
					//do nothing
				}
			}
			//we ran straight into an obstacle, do something
			else {
				//if not within the first 1/2 of image, we really don't care
				if (y > frame.rows / 2) {
					//find first edge point in both directions
					int edgeL = prevX;
					int edgeR = prevX;
					while (edgeL > 0 &&
						frameEdges.at<uchar>(Point(edgeL, y)) == 0) {
						p_debugImg.at<Vec3b>(Point(edgeL, y)) = Vec3b(0, 255, 255);
						edgeL--;
					}
					while (edgeR < (frameEdges.cols - 1) &&
						frameEdges.at<uchar>(Point(edgeR, y)) == 0) {
						p_debugImg.at<Vec3b>(Point(edgeR, y)) = Vec3b(0, 255, 0);
						edgeR++;
					}

					//classify edge types and weight accordingly
					EDGE_TYPE_T edgeLType = EDGE_TYPE_IMG;
					EDGE_TYPE_T edgeRType = EDGE_TYPE_IMG;
					float lWeight = 10.0;
					float rWeight = 10.0;
					if (frameEdges.at<uchar>(Point(edgeL, y)) != 0) {
						lWeight = 9.0;
						edgeLType = EDGE_TYPE_EDGE;
					}
					if (frameEdges.at<uchar>(Point(edgeR, y)) != 0) {
						rWeight = 9.0;
						edgeRType = EDGE_TYPE_EDGE;
					}

					//is course edge only on the left
					if (edgeLType == EDGE_TYPE_EDGE &&
						edgeRType == EDGE_TYPE_IMG)
						//go halfway between object and edge of picture
						targetX = (prevX + (frame.cols - 1)) / 2;
					//is course edge only on the right
					else if (edgeLType == EDGE_TYPE_IMG &&
						edgeRType == EDGE_TYPE_EDGE)
						//go halfway between object and edge of picture
						targetX = prevX / 2;
					//is course edge further away on the right
					else if ((edgeR - prevX) > (prevX - edgeL))
						//go halfway between object and edge
						targetX = (edgeR + prevX) / 2;
					//is course edge further away on the left
					else
						//go halfway between object and edge
						targetX = (edgeL + prevX) / 2;

					p_debugImg.at<Vec3b>(Point(targetX, y)) = Vec3b(255, 255, 255);
					cout << "Ran straight into obstacle at ." << y << endl;
				}
				//not within first half of image,
				else {
					//we hit an object in the upper half of the image. 
					//Wait until we're closer to decide what to do.
					//We have enough informaiton to work with anyways.
					break;
				}
			}
		}
		//we ran straight into an edge here, just end. Next frame will have more information
		else {
			cout << "Ran straight into an edge. Waiting for new frame. " << Point(prevX, y) << endl;
			break;
		}

		//add next x to route
		p_debugImg.at<Vec3b>(Point(targetX, y)) = Vec3b(0, 255, 255);
		route.push_back( targetX );
		objInRow.push_back(objInThisRow);

		//push previous point onto route
		prevX = targetX;
	}

	//look at route and determine current direction and speed
	float dir = 0;
	int divisor = 0;
	int unchangedY = (int)route.size();
	for (int i = 0; i < route.size(); i++) {
		//get direction we should go (weight according to where we are in image)
		int xLoc = route.at(i);
		int weight = (int)((EXP_MULTIPLIER / exp(EXP_FACTOR * i)) + 1);
		if (objInRow.at(i))
			weight *= OBJ_IN_ROW_MULTIPLIER;
		int distFromCenter = xLoc - midPoint;
		dir += (weight*distFromCenter);
		//keep track of our weight so we can divide accurately
		divisor += weight;

		//check if we should set the speed (non-middle point)
		if (unchangedY == route.size() && i != 0 ) {
			if (distFromCenter > SPEED_DIST_FROM_CENTER || 
					distFromCenter < -SPEED_DIST_FROM_CENTER ) {
				unchangedY = i;
				//cout << "UnchangedY: " << i << endl;
			}
		}
	}

	//scale direction to weighted avg pixels per row
	dir = dir / divisor;
	//since we find the mid-point the maximum 1 direciton can be is 
	//1/4 the image, scale this to be between -100 and 100
	int nextDirection = (int)(((float)dir * 100.0)/STEERING_SENSITIVITY);
	if( nextDirection > 100 )
		nextDirection = 100;
	else if( nextDirection < -100 )
		nextDirection = -100;

	//change speed
	int nextSpeed;
	if (unchangedY > SPEED_DIST0)
		nextSpeed = SPEED_VAL0;
	else if (unchangedY > SPEED_DIST1)
		nextSpeed = SPEED_VAL1;
	else if (unchangedY > SPEED_DIST2)
		nextSpeed = SPEED_VAL2;
	else {
		nextSpeed = SPEED_VAL3;
	}
	nextSpeed = (int)((float)nextSpeed * SPEED_FACTOR);

	//increase speed if direction is sharp	
	if( nextDirection > 50 )
		nextSpeed = (nextSpeed * 4 ) / 3;
	else if( nextDirection < -50 )
		nextSpeed = (nextSpeed * 4 ) / 3;


	//set speed, direction, and bail values
	speed = nextSpeed;
	direction = nextDirection;
	p_bail = nextBail;

	//draw steering text on screen
	std::string pSteering = "Direction: ";
	pSteering.append( std::to_string( direction ) );
	putText(p_debugImg, pSteering, Point(10, 10), FONT_HERSHEY_PLAIN, 1, CV_RGB(255, 0, 0), 1);

	//draw drive text on screen
	std::string pDrive= "Speed    : ";
	pDrive.append( std::to_string( speed ) );
	putText(p_debugImg, pDrive, Point(10, 30), FONT_HERSHEY_PLAIN, 1, CV_RGB(255, 0, 0), 1);

	//show debug image
	if( debugMode )
		imshow("main", p_debugImg);
	if( p_writeVideo && p_debugImg.size() == p_videoSize ) {
		cout << "writing frame to video... " << p_debugImg.size() << endl;
		p_video << p_debugImg;
		cout << "frame complete... " << endl;
	}
	else if( p_writeVideo && p_debugImg.size() != p_videoSize)
		cout << "Weird, frame came in differently..." << endl;
}
	
void Navigate::analyze_bail(cv::Mat frame)
{

	//check bail state
	switch (p_bailState) {
	case NAV_BAIL_STATE_BACKUP:
	{
		//back straight out until no objects are in bottom of image
		speed = SPEED_VAL_BAK;
		direction = 0;

		//get obstacles
		cv::Mat colors;
		cvtColor(frame, colors, CV_RGB2HSV);
		Mat frameObstacles;
		get_obstacles(colors, &frameObstacles);
		
		//get lower portion of image
		Rect R(Point(0, (int)((float)frame.rows*BAIL_PORTION_BEFORE_TURN)),
			Point(frame.cols - 1, frame.rows - 1));
		Mat noBailPortion = frameObstacles(R);

		//change state once no more object in this section of the image
		int numObjPix = countNonZero(noBailPortion);
		if ( numObjPix == 0) {
			p_bailState = NAV_BAIL_STATE_TURN;
		}

		//debug
		Mat noBailColor;
		cvtColor(noBailPortion, noBailColor, CV_GRAY2RGB);
		frame.copyTo( p_debugImg );
		Mat lowerPortion = p_debugImg(R);
		noBailColor.copyTo(lowerPortion);
		//draw steering text on screen
		std::string pSteering = "Direction: ";
		pSteering.append(std::to_string(direction));
		putText(p_debugImg, pSteering, Point(10, 10), FONT_HERSHEY_PLAIN, 1, CV_RGB(255, 0, 0), 1);

		//draw drive text on screen
		std::string pDrive = "Speed    : ";
		pDrive.append(std::to_string(speed));
		putText(p_debugImg, pDrive, Point(10, 30), FONT_HERSHEY_PLAIN, 1, CV_RGB(255, 0, 0), 1);
		
		if( debugMode )
			imshow("main", p_debugImg);

		if( p_writeVideo && p_debugImg.size() == p_videoSize ) {
			cout << "writing frame to video..." << endl;
			p_video << p_debugImg;
		}
		else if( p_writeVideo && p_debugImg.size() != p_videoSize)
			cout << "Weird, frame came in differently..." << endl;
	}
	break;

	case NAV_BAIL_STATE_TURN:
	{
		//backup and set direction of tires to avoid obstacle
		speed = SPEED_VAL_BAK;
		if (p_bailToTheRight)
			direction = -50;
		else
			direction = 50;

		//get edges and obstacles
		cv::Mat colors;
		Mat frameObstacles;
		Mat frameEdges;
		cvtColor(frame, colors, CV_RGB2HSV);
		get_obstacles(colors, &frameObstacles);
		get_edges(colors, &frameEdges);

		//create debug image
		frame.copyTo(p_debugImg);

		//check if we've turned enough (first object edge is on opposite side)
		int y;
		int center = frameEdges.cols / 2;
		int centerOffset = (int)((float)center * BAIL_CENTER_OFFSET);
		p_bail = true;
		for (y = frameEdges.rows - 1; y >= 0; y--) {
			//ensure we're not at an edge 
			if (frameEdges.at<uchar>(Point(center, y)) == 0) {
				//check that we're not at an obstacle
				if (frameObstacles.at<uchar>(Point(center, y)) == 0) {
					//find first edge point in both directions from center
					int edgeL = center;
					int edgeR = center;
					while (edgeL > 0 &&
							frameEdges.at<uchar>(Point(edgeL, y)) == 0 &&
							frameObstacles.at<uchar>(Point(edgeL, y)) == 0) {
						p_debugImg.at<Vec3b>(Point(edgeL, y)) = Vec3b(255, 0, 255);
						edgeL--;
					}
					while (edgeR < (frameEdges.cols - 1) &&
							frameEdges.at<uchar>(Point(edgeR, y)) == 0 &&
							frameObstacles.at<uchar>(Point(edgeR, y)) == 0) {
						p_debugImg.at<Vec3b>(Point(edgeR, y)) = Vec3b(255, 255, 0);
						edgeR++;
					}

					//classify edge types and weight accordingly
					EDGE_TYPE_T edgeLType = EDGE_TYPE_IMG;
					EDGE_TYPE_T edgeRType = EDGE_TYPE_IMG;
					if (frameObstacles.at<uchar>(Point(edgeL, y)) != 0)
						edgeLType = EDGE_TYPE_OBJECT;
					else if (frameEdges.at<uchar>(Point(edgeL, y)) != 0)
						edgeLType = EDGE_TYPE_EDGE;
					if (frameObstacles.at<uchar>(Point(edgeR, y)) != 0)
						edgeRType = EDGE_TYPE_OBJECT;
					else if (frameEdges.at<uchar>(Point(edgeR, y)) != 0)
						edgeRType = EDGE_TYPE_EDGE;

					//check if any edge is an object (only look at first object)
					if (edgeLType == EDGE_TYPE_OBJECT || edgeRType == EDGE_TYPE_OBJECT) {
						//check which direction we're trying to bail
						if (p_bailToTheRight) {
							//direction = -50;
							//check if we turned enough
							int distFromCenterL = center - edgeL;
							if (edgeLType == EDGE_TYPE_OBJECT &&
									distFromCenterL > centerOffset) {
								//turned enough, don't bail
								p_bail = false;
							}
						}
						else {
							//direction = 50;
							//check if we turned enough
							int distFromCenterR = edgeR - center;
							if (edgeRType == EDGE_TYPE_OBJECT &&
									distFromCenterR > centerOffset) {
								//turned enough, don't bail
								p_bail = false;
							}
						}

						//we found the first object, wait for next frame
						break;
					}
					//didn't find an object, look higher in image
				}
				//straight into an obstacle
				else {
					//keep backing up (wait for next frame)
					break;
				}
			}
			//we hit an edge immediately.  What the?
			else {
				//crap, are we going the wrong way? No, we really don't even care about edges here..
				//waitKey();
			}
		}

		//draw steering text on screen
		std::string pSteering = "Direction: ";
		pSteering.append(std::to_string(direction));
		putText(p_debugImg, pSteering, Point(10, 10), FONT_HERSHEY_PLAIN, 1, CV_RGB(255, 0, 0), 1);

		//draw drive text on screen
		std::string pDrive = "Speed    : ";
		pDrive.append(std::to_string(speed));
		putText(p_debugImg, pDrive, Point(10, 30), FONT_HERSHEY_PLAIN, 1, CV_RGB(255, 0, 0), 1);

		//show debug image
		if( debugMode )
			imshow("main", p_debugImg);
		if( p_writeVideo && p_debugImg.size() == p_videoSize ) {
			cout << "writing frame to video..." << endl;
			p_video << p_debugImg;
		}
		else if( p_writeVideo && p_debugImg.size() != p_videoSize)
			cout << "Weird, frame came in differently..." << endl;
	}
	break;
	}
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
void Navigate::get_obstacles(cv::Mat hsvImg, cv::Mat *frameObstacles)
{
	//get obstacles in image (orange)
	int satMin = 175;
	int hueCenter = 116; //lower is more orange
	int hueMin = hueCenter - 8;
	int hueMax = hueCenter + 8;
	int valMin = 60;
	int valMax = 255;
	inRange(hsvImg, Scalar(hueMin, satMin, valMin), Scalar(hueMax, 255, valMax), *frameObstacles);
	if( showObjects )
		imshow( "obstacles", *frameObstacles );
}

void Navigate::get_edges(cv::Mat hsvImg, cv::Mat *frameEdges)
{
	//get course edges (blue)
	int satMin = 100;
	int hueCenter = 18;
	int hueMin = hueCenter - 14;
	int hueMax = hueCenter + 14;
	int valMin = 50;
	int valMax = 255;
	inRange(hsvImg, Scalar(hueMin, satMin, valMin), Scalar(hueMax, 255, valMax), *frameEdges);
	if( showEdges )
		imshow( "edges", *frameEdges );
}

void Navigate::start_video( cv::Size videoSize )
{
	p_writeVideo = true;
	p_video.open(	WRITE_VIDEO_NAME, 
			VideoWriter::fourcc('M', 'J', 'P', 'G'), 
			VIDEO_RATE, 
			videoSize, 
			true);
	p_videoSize = cv::Size(videoSize);
}

void Navigate::end_video()
{
	p_writeVideo = false;
}
