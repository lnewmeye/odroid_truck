/******************************************************************************
 * Camera Class - This may source directly from a camera or from a file
 * 		  depending if the CAMERA_USE_FILE definition is set.
 *
 * Authors: James Swift, Luke Newmeyer
 * Copyright 2017
 *****************************************************************************/
#pragma once

/****************************** Include Files ********************************/

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define CAMERA_USE_FILE "data/truck_competition_course.avi"

class Camera {
	public:
		Camera();
		void open();
		cv::Mat get_frame();

	private:
		bool p_opened;
		cv::VideoCapture p_vCap;
};
