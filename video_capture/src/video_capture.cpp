/****************************************************************************
 * Video capture
 ****************************************************************************/

/****************************** Include Files ******************************/
// Standard includes
#include <iostream>

// OpenCV includes
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

/****************************** Definitions ********************************/

#define VIDEO_TITLE "video_output.avi"
#define VIDEO_RATE 30

using std::cout;
using std::endl;
using cv::VideoCapture;
using cv::VideoWriter;
using cv::Mat;
using cv::Size;
using cv::waitKey;

/****************************** Implementation *****************************/

int main( int argc, char **argv)
{
	// Open camera and error and exit if failure to open
	cout << "Opening camera..." << endl;
	VideoCapture camera(0);
	if (!camera.isOpened()) {
		cout << "Error: camera not open" << endl;
		return -1;
	}
	//set resolution
	camera.set(CV_CAP_PROP_FRAME_WIDTH, 160 );
	camera.set(CV_CAP_PROP_FRAME_HEIGHT, 90 );

	//camera.set(CV_CAP_PROP_FRAME_WIDTH, 320 );
	//camera.set(CV_CAP_PROP_FRAME_HEIGHT, 240 );

	//camera.set(CV_CAP_PROP_FRAME_WIDTH, 640 );
	//camera.set(CV_CAP_PROP_FRAME_HEIGHT, 480 );

	//camera.set(CV_CAP_PROP_FRAME_WIDTH, 1920 );
	//camera.set(CV_CAP_PROP_FRAME_HEIGHT, 1080 );

	//create video writer
	cout << "Opening video file..." << endl;
	VideoWriter video;
	Size image_size = Size((int)camera.get(CV_CAP_PROP_FRAME_WIDTH),
			(int)camera.get(CV_CAP_PROP_FRAME_HEIGHT));
	video.open(	VIDEO_TITLE, 
			VideoWriter::fourcc('M', 'J', 'P', 'G'), 
			VIDEO_RATE, 
			image_size, 
			true);
	if (!video.isOpened())
	{
		cout << "Could not open video output" << endl;
		return -1;
	}

	//capture video
	cout << "Capturing video. Pres space to stop." << endl;
	Mat frame;
	while( waitKey(1) != ' ' ) {
		//capture a frame
		camera >> frame;
		//place frame in video
		video << frame;
	}

	//done
	cout << "Done!\n";
}
