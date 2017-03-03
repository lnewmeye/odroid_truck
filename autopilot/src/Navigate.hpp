/******************************************************************************
 * Navigate Class - This can will use subsequent camera frames to determine if
 *                  one should go left, right, stright, or back up.
******************************************************************************/
#pragma once
/*************************** Include Files ***********************************/

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

/*************************** Definitions *************************************/

typedef enum DRIVE_STATE {
	DRIVE_FOLLOW_INNER,
	DRIVE_AVOID_INNER,
	DRIVE_FOLLOW_OUTTER,
	DRIVE_AVOID_OUTTER
} DRIVE_STATE;

class Navigate {
	//variables
public:
	int speed;
	int direction;

	//methods
public:
	Navigate() {}
	void analyze_frame(cv::Mat frame);
	//void analyze_bail(cv::Mat frame);
	cv::Mat getEdges() {return edges_frame;}
	cv::Mat getObstacles() {return obstacles_frame;}

private:
	DRIVE_STATE drive_state = DRIVE_FOLLOW_INNER;
	cv::Mat obstacles_frame;
	cv::Mat edges_frame;
	//NAV_STATE_T p_navState;
	//NAV_BAIL_STATE_T p_bailState;
	//cv::Mat p_debugImg;
	//int p_bailCnt;
	//bool p_bail;
	//bool p_bailToTheRight; //until object on left


	//private methods
private:
	//void analyze_forward( cv::Mat frame );
	//void analyze_bail( cv::Mat frame );
	//int get_min_dist(int y);
	void get_obstacles(cv::Mat frame);
	void get_edges(cv::Mat frame);
};
