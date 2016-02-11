/******************************************************************************
 * Navigate Class - This can will use subsequent camera frames to determine if
 *                  one should go left, right, stright, or back up.
******************************************************************************/
#pragma once
/*************************** Include Files ***********************************/

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

/*************************** Definitions *************************************/

/** Navigate state machine */
typedef enum NAV_STATE_E {
    NAV_STATE_FORWARD = 0,
    NAV_STATE_BAIL,
    NAV_STATE_NUMS
} NAV_STATE_T;

/** Bail state machine */
typedef enum NAV_BAIL_STATE_E {
	NAV_BAIL_STATE_BACKUP= 0,
	NAV_BAIL_STATE_TURN,
	NAV_BAIL_STATE_NUMS
} NAV_BAIL_STATE_T;

class Navigate {
	//variables
public:
	int speed;
	int direction;
	bool debugMode;
	bool showObjects;
	bool showEdges;
	bool writeVideoVerbose;

	//methods
public:
	Navigate();
	void analyze_frame(cv::Mat frame);
	void start_video( cv::Size videoSize );
	void end_video(void );
	//void analyze_bail(cv::Mat frame);

	//private variables
private:
    NAV_STATE_T p_navState;
    NAV_BAIL_STATE_T p_bailState;
	cv::Mat p_debugImg;
	int p_bailCnt;
	bool p_bail;
	bool p_bailToTheRight; //until object on left
	bool p_writeVideo;
	cv::VideoWriter p_video;
	cv::Size p_videoSize;


	//private methods
private:
	void analyze_forward( cv::Mat frame );
	void analyze_bail( cv::Mat frame );
	int get_min_dist(int y);
	void get_obstacles(cv::Mat hsvImg, cv::Mat *frameObstacles);
	void get_edges(cv::Mat hsvImg, cv::Mat *frameEdges);
};
