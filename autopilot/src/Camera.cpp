/****************************************************************************
 * Video capture Implementation
 ****************************************************************************/
/****************************** Include Files ******************************/

#include <iostream>
#include "Camera.hpp"

/****************************** Definitions ********************************/

/****************************** Implementation *****************************/

Camera::Camera( void )
{
	p_opened = false;
}

void Camera::open( void )
{
#ifdef CAMERA_USE_FILE
	//cv::String filename(CAMERA_USE_FILE);
	p_vCap.open(CAMERA_USE_FILE);
#else
	p_vCap(0);
#endif
	if( p_vCap.isOpened() ) {
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

