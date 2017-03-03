/******************************************************************************
 * Navigate Class - This can will use subsequent camera frames to determine if
 *                  one should go left, right, stright, or back up.
******************************************************************************/
#pragma once
/*************************** Include Files ***********************************/

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

/*************************** Definitions *************************************/

// Parameters for edges
#define EDGE_HUE 120
#define EDGE_RANGE 30
#define EDGE_SATURATION_MIN 60
#define EDGE_VALUE_MIN 70
#define EDGE_DILATION 5

// Parameters for obstacles
#define CONES_HUE 8
#define CONES_RANGE 8
#define CONES_SATURATION_MIN 70
#define CONES_VALUE_MIN 70

// Parameters for truck location
#define TRUCK_DEFAULT_RIGHT
#define TRUCK_INNER_DISTANCE
#define TRUCK_OUTER_DISTANCE

// Parameters for navigation
#define OUTER_COUNT_MAX 15

typedef enum NAVIGATE_STATE {
	NAVIGATE_FOLLOW_INNER,
	NAVIGATE_FOLLOW_OUTER,
	NAVIGATE_AVOID_INNER,
	NAVIGATE_AVOID_OUTER,
	NAVIGATE_REVERSE_INNER,
	NAVIGATE_REVERSE_OUTER
} NAVIGATE_STATE;

typedef struct Edge {
	bool exists;
	double left;
	double top;
	double width;
	double height;
} Edge;

class Navigate {

public:
	// Public variables
	int speed;
	int direction;

	// Public functions
	Navigate() {}
	void navigateFrame(cv::Mat frame);
	cv::Mat getEdges() {return edges_frame;}
	cv::Mat getCones() {return cones_frame;}
	cv::Mat getComposite() {return edges_frame+cones_frame;}

private:
	// Private variables
	NAVIGATE_STATE navigate_state = NAVIGATE_FOLLOW_INNER;
	cv::Mat edges_frame;
	cv::Mat cones_frame;
	Edge inner_edge;
	Edge outer_edge;
	bool cone_front = false;
	bool cone_left = false;
	bool cone_right = false;
	int outer_count = 0;

	// Priave functions
	void processEdges(cv::Mat frame);
	void processCones(cv::Mat frame);
	void followInner();
	void followOuter();
	void avoidInner();
	void avoidOuter();
	void reverseInner();
	void reverseOuter();
};
