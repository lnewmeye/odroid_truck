/******************************************************************************
 * Navigate Class - This can will use subsequent camera frames to determine if
 *                  one should go left, right, stright, or back up.
******************************************************************************/
#pragma once
/*************************** Include Files ***********************************/

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

/*************************** Definitions *************************************/

#define NAVIGATE_NUM_AREAS 4
//#define NAVIGATE_USE_LUKE

class Navigate {
	//variables
public:
	int speed;
	int direction;

	//methods
public:
	Navigate();
	void analyze_frame(cv::Mat frame);

	//private variables
private:

	//private methods
private:
	void analyze_frame_james( cv::Mat frame );
	void analyze_frame_luke( cv::Mat frame );
};
