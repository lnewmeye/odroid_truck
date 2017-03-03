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

#define DEBUG

/****************************** Implementation *******************************/

Camera::Camera( void )
{
	p_opened = false;
}

void Camera::open( void )
{
#ifdef CAMERA_USE_FILE
	p_vCap.open(CAMERA_USE_FILE);
#else
	p_vCap.open(0);
#endif

	if( p_vCap.isOpened() ) {
#ifdef CAMERA_USE_FILE
		//fast forward
		for (int i = 0; i < 5; i++) { 
			cv::Mat frame; 
			p_vCap >> frame; 
		}
#else
		//set resolution
		p_vCap.set(CV_CAP_PROP_FRAME_WIDTH, 160 );
		p_vCap.set(CV_CAP_PROP_FRAME_HEIGHT, 90 );
#endif
		p_opened = true;
	} else {
		std::cout << "Error: Unable to open camera.\n";
	}

#ifdef DEBUG
	std::cout << "Tried opening file: " << CAMERA_USE_FILE << std::endl;
#endif
}

cv::Mat Camera::get_frame( void )
{
	cv::Mat frame;

	if( p_opened ) {
		p_vCap >> frame;
	}

	return frame;
}

