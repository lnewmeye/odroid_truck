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

// Parameters for cones
#define CONES_HUE 10
#define CONES_RANGE 10
#define CONES_SATURATION_MIN 220
#define CONES_VALUE_MIN 170
#define CONE_MIN_AREA 20

// Parameters for left, front, and right regions
#define FRAME_WIDTH 160
#define FRAME_HEIGHT 80
#define FRONT_LEFT_X1 FRAME_WIDTH / 3
#define FRONT_LEFT_Y1 FRAME_HEIGHT
#define FRONT_LEFT_X2 FRAME_WIDTH / 2
#define FRONT_LEFT_Y2 0
#define FRONT_RIGHT_X1 2*FRAME_WIDTH / 3
#define FRONT_RIGHT_Y1 FRAME_HEIGHT
#define FRONT_RIGHT_X2 FRONT_LEFT_X2
#define FRONT_RIGHT_Y2 FRONT_RIGHT_Y2
#define FRONT_DISTANCE FRAME_HEIGHT / 3

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
	int left;
	int top;
	int width;
	int height;
} Edge;

typedef struct Cone {
	int centroid_x;
	int centroid_y;
	int box_left;
	int box_top;
	int box_width;
	int box_height;
} Cone;

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
	void getNavigation(cv::Mat input, cv::Mat& output);

private:
	// Private variables
	NAVIGATE_STATE navigate_state = NAVIGATE_FOLLOW_OUTER;
	cv::Mat edges_frame;
	cv::Mat cones_frame;
	Edge inner_edge;
	Edge outer_edge;
	std::vector<Cone> left_cones;
	std::vector<Cone> front_cones;
	std::vector<Cone> right_cones;
	std::vector<Cone> other_cones;
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
