// Truck class for Odroid Truck Project
// Authors: James Swift, Luke Newmeyer

/****************************************************************************
 * Camera Class - This may source directly from a camera or from a file
 * 		  depending if the CAMERA_USE_FILE definition is set.
 ****************************************************************************/
#pragma once

/****************************** Include Files ******************************/

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define CAMERA_USE_FILE "video_output.avi"

class Camera {
	public:
		Camera();
		void open();
		cv::Mat get_frame();
	private:
		bool p_opened;
		cv::VideoCapture p_vCap;

};
