/*****************************************************************************
 * Class Implementation
 *
 * Authors: James Swift, Luke Newmeyer
 * Copyright 2017
 *****************************************************************************/
/****************************** Include Files ********************************/

#include <iostream>
#include "Camera.hpp"

/****************************** Definitions **********************************/

/****************************** Implementation *******************************/

Camera::Camera( void )
{
	p_opened = false;
}

void Camera::open( void )
{
#ifdef CAMERA_USE_FILE
	//cv::String filename(CAMERA_USE_FILE);
	p_vCap.open(CAMERA_USE_FILE);
	//fast forward
	for (int i = 0; i < 5; i++) { 
		cv::Mat frame; 
		p_vCap >> frame; 
	}
#else
	p_vCap.open(0);
#endif

	if( p_vCap.isOpened() ) {

#ifndef CAMERA_USE_FILE
		//set resolution
		p_vCap.set(CV_CAP_PROP_FRAME_WIDTH, 160 );
		p_vCap.set(CV_CAP_PROP_FRAME_HEIGHT, 90 );
#endif
		p_opened = true;
	} else {
		std::cout << "Error: Unable to open camera.\n";
	}
}

cv::Mat Camera::get_frame( void )
{
	cv::Mat frame;

	if( p_opened ) {
		p_vCap >> frame;
	}

	return frame;
}

