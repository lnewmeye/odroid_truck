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
#include <vector>

/*************************** Definitions *************************************/

//steering
#define EXP_FACTOR 0.06
#define EXP_MULTIPLIER 9.0
#define STEERING_SENSITIVITY 5.0
#define OBJ_IN_ROW_MULTIPLIER 3

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
//battery level: low=0.6, med=0.8, high=1.0
#define SPEED_FACTOR 0.7

//bailing
#define BAIL_DISTANCE_FACTOR_TO_BAIL 0.4
#define BAIL_PORTION_BEFORE_TURN 0.5
#define BAIL_CENTER_OFFSET 0.25

using std::cout;
using std::endl;
using std::string;
using std::vector;

using cv::Mat;
using cv::Size;
using cv::Scalar;

/*************************** Implementation **********************************/

void Navigate::navigateFrame(Mat frame)
{
	// Run initial processing on frame
	processEdges(frame);
	processCones(frame);

	// Find cone positions
	

	// Update state according to positions
	switch(navigate_state) {
		case NAVIGATE_FOLLOW_INNER:
			if(cone_front) {
				if(cone_right)
					navigate_state = NAVIGATE_REVERSE_INNER;
				else
					navigate_state = NAVIGATE_AVOID_INNER;
			}
			break;

		case NAVIGATE_FOLLOW_OUTER:
			if(cone_front) {
				if(cone_left)
					navigate_state = NAVIGATE_REVERSE_OUTER;
				else
					navigate_state = NAVIGATE_AVOID_OUTER; 
			}
			else {
				if(outer_count > OUTER_COUNT_MAX && !cone_left)
					navigate_state = NAVIGATE_FOLLOW_INNER;
			}
			break;

		case NAVIGATE_AVOID_INNER:
			if(!cone_left)
				navigate_state = NAVIGATE_FOLLOW_INNER;
			break;

		case NAVIGATE_AVOID_OUTER:
			if(!cone_right)
				navigate_state = NAVIGATE_FOLLOW_OUTER;
			break;

		case NAVIGATE_REVERSE_INNER:
			if(!cone_left)
				navigate_state = NAVIGATE_FOLLOW_OUTER;
			break;

		case NAVIGATE_REVERSE_OUTER:
			if(!cone_right)
				navigate_state = NAVIGATE_FOLLOW_INNER;
			break;
	}
	
	// Follow current navigation state
	switch(navigate_state) {
		case NAVIGATE_FOLLOW_INNER:
			followInner();
			break;

		case NAVIGATE_FOLLOW_OUTER:
			followOuter();
			outer_count++;
			break;

		case NAVIGATE_AVOID_INNER:
			avoidInner();
			break;

		case NAVIGATE_AVOID_OUTER:
			avoidOuter();
			break;

		case NAVIGATE_REVERSE_INNER:
			reverseInner();
			outer_count = 0;
			break;

		case NAVIGATE_REVERSE_OUTER:
			reverseOuter();
			break;

	}
}

void Navigate::processEdges(cv::Mat frame)
{
	// Convert image to hue, saturation, value
	Mat frame_hsv;
	cv::cvtColor(frame, frame_hsv, cv::COLOR_BGR2HSV);
	
	// Get course edges (blue)
	int satMin = EDGE_SATURATION_MIN;
	int satMax = 255;
	int hueCenter = EDGE_HUE;
	int hueMin = hueCenter - EDGE_RANGE;
	int hueMax = hueCenter + EDGE_RANGE;
	int valMin = EDGE_VALUE_MIN;
	int valMax = 255;

	// Define minimum and maximums
	Scalar minimum = Scalar(hueMin, satMin, valMin);
	Scalar maximum = Scalar(hueMax, satMax, valMax);

	// Select image range
	Mat edges_range, edges_dilate, edges_label, edges_stats, edges_centroids;
	cv::inRange(frame_hsv, minimum, maximum, edges_range);
	
	// Dilate image slightly
	Size dilation_size = Size(EDGE_DILATION, EDGE_DILATION);
	Mat dilate_structure = cv::getStructuringElement(cv::MORPH_ELLIPSE, 
			dilation_size);
	cv::dilate(edges_range, edges_dilate, dilate_structure);
	edges_frame = edges_dilate;
	
	// Identify edge using connected components
	cv::connectedComponentsWithStats(edges_dilate, edges_label, edges_stats,
			edges_centroids);

	// Find left and right edges from connected components
	vector<int> inner_index, outer_index;
	double centroid_x;
	for(int i = 1; i < edges_centroids.rows; i++) {
		centroid_x = edges_centroids.at<double>(i,0);

		// TODO: Adjust this statement to account for reverse direction
		// This assumes that we are driving in a counter clockwise circle
		// and that the outer edge is on the right side of the image
		if(centroid_x > frame.rows/2)
			outer_index.push_back(i);
		else
			inner_index.push_back(i);
	}

	// Choose best component for inner object
	inner_edge.exists = false; // Reset data strucutre (in case no inner)
	double inner_area = 0;
	for(int index : inner_index) {
		if(inner_area < edges_stats.at<double>(index,cv::CC_STAT_AREA)) {
			inner_edge.exists = true;
			inner_edge.left = edges_stats.at<double>(index,cv::CC_STAT_LEFT);
			inner_edge.top = edges_stats.at<double>(index,cv::CC_STAT_TOP);
			inner_edge.width = edges_stats.at<double>(index,cv::CC_STAT_WIDTH);
			inner_edge.height = edges_stats.at<double>(index,cv::CC_STAT_HEIGHT);
		}
	}

	// Choose best component for outer object
	outer_edge.exists = false; // Reset data structure (in case no outer)
	double outer_area = 0;
	for(int index : outer_index) {
		if(outer_area < edges_stats.at<double>(index,cv::CC_STAT_AREA)) {
			outer_edge.exists = true;
			outer_edge.left = edges_stats.at<double>(index,cv::CC_STAT_LEFT);
			outer_edge.top = edges_stats.at<double>(index,cv::CC_STAT_TOP);
			outer_edge.width = edges_stats.at<double>(index,cv::CC_STAT_WIDTH);
			outer_edge.height = edges_stats.at<double>(index,cv::CC_STAT_HEIGHT);
		}
	}
}

void Navigate::processCones(cv::Mat frame)
{
	// Convert image to hue, saturation, value
	Mat frame_hsv;
	cv::cvtColor(frame, frame_hsv, cv::COLOR_BGR2HSV);

	//get obstacles in image (orange)
	int satMin = CONES_SATURATION_MIN;
	int hueCenter = CONES_HUE; //lower is more orange
	int hueMin = hueCenter - CONES_RANGE;
	int hueMax = hueCenter + CONES_RANGE;
	int valMin = CONES_VALUE_MIN;
	int valMax = 255;

	// Define minimum and maximums
	Scalar minimum = Scalar(hueMin, satMin, valMin);
	Scalar maximum = Scalar(hueMax, 255, valMax);

	// select image range
	cv::inRange(frame_hsv, minimum, maximum, cones_frame);
}

void Navigate::followInner()
{
}

void Navigate::followOuter()
{
}

void Navigate::avoidInner()
{
}

void Navigate::avoidOuter()
{
}

void Navigate::reverseInner()
{
}

void Navigate::reverseOuter()
{
}
