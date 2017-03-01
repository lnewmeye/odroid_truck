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

typedef enum NAV_STATE_E {
    NAV_STATE_FOLLOW_EDGE = 0,
    NAV_STATE_AVOID_OBSTACLE,
    NAV_STATE_NUMS
} NAV_STATE_T;

class Navigate {
	//variables
public:
	int speed;
	int direction;
	int confidenceRow;
	int bailCnt;
	bool bailDirectionSet;
	bool bailDirectionRight;

	//methods
public:
	Navigate();
	void analyze_frame(cv::Mat frame);
	void analyze_bail(cv::Mat frame);

	//private variables
private:
	cv::Mat p_debugImg;
    NAV_STATE_T p_navState;
    bool p_backwards;
    int p_hardLeft;
    int p_softLeft;
    int p_hardRight;
    int p_softRight;
	float p_objDir;
	float p_edgeDir;
	float p_objSpeed;
	float p_edgeSpeed;

	//private methods
private:
	void analyze_frame_james( cv::Mat frame );
	void analyze_frame_luke( cv::Mat frame );
	
    /** Get minimum distance to an edge/obstacle for a given y value */
	int get_min_dist(int y);
	bool build_path(cv::Mat *img, cv::Rect *imgBounds, int dir, cv::Point currPoint, std::vector<cv::Point> *path);
    void populate_areas( int x, int y );
};
