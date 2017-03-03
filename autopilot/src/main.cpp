/****************************************************************************
 * Video capture Implementation
 * Authors: James Swift, Luke Newmeyer
 ****************************************************************************/

/****************************** Include Files ******************************/

#include "Truck.hpp"
#include "Camera.hpp"
#include "Navigate.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
//#include <unistd.h>
//#include <termios.h>

/****************************** Definitions ********************************/

/** Keys the user can press to change the state */
#define KEY_AUTOPILOT 'a'
#define KEY_STOP 's'
#define KEY_MANUAL 'm'
#define KEY_CALIBRATE 'c'
#define KEY_HELP 'h'
#define KEY_TEST_FRAME 't'
#define KEY_ESCAPE 27

/** Main state machine */
typedef enum MAIN_STATE_E {
	MAIN_STATE_IDLE = 0,
	MAIN_STATE_MANUAL_DRIVE,
	MAIN_STATE_AUTO_DRIVE,
	MAIN_STATE_CALIBRATE,
	MAIN_STATE_NUMS
} MAIN_STATE_T;

typedef enum AUTO_STATE {
	AUTO_STATE_EDGES,
	AUTO_STATE_OBSTACLES,
	AUTO_STATE_COMPOSITE,
	AUTO_STATE_VIDEO
} AUTO_STATE;

using std::cout;
using std::cin;
using std::cin;
using std::endl;
    
static Camera m_camera;
static Navigate m_nav;
static Truck m_truck;

/****************************** Private Functions **************************/

/** Print usage to screen */
static void main_print_usage( void );
/** Manual drive mode (will enter it's own loop) */
static void main_manual_drive( void );
/** Autopilot mode */
void auto_print_usage();
static void main_auto_drive( void );
/** Calibrate mode */
static void main_calibrate_drive( void );
static void main_report_nav( void );

/****************************** Implementation *****************************/

int main()
{
	MAIN_STATE_T state = MAIN_STATE_IDLE;

	//print usage
	main_print_usage();

	//open camera
	m_camera.open();

	//connect to the truck
	m_truck.connect_truck();

	//open a window (we can't get key presses without a window open)
	cv::namedWindow("main", CV_WINDOW_KEEPRATIO);

	//main loop (close main loop if main window closes, feel free to change)
	main_print_usage();
	while(1) {
		//get key press and change state

		//so apparently this doesnt' work while using ssh. Windows/putty thing?
		//So I'll use the blocking method instead. Bummer.
		char c = cv::waitKey(1);
		//char c;
		//cin >> c;

		//check if key was pressed
		if( c >= 0 ) {
			//check which key was pressed
			switch( c ) {
			case KEY_CALIBRATE:
				cout << "Entering Calibrate state." << endl;
				state = MAIN_STATE_CALIBRATE;
				break;

			case KEY_MANUAL:
				cout << "Entering Manual Drive state." << endl;
				state = MAIN_STATE_MANUAL_DRIVE;
				break;

			case KEY_STOP:
				cout << "Stopping truck" << endl;
				m_truck.set_drive(0);
				m_truck.set_steering(0);
				cout << "Entering Idle state." << endl;
				state = MAIN_STATE_IDLE;
				break;

			case KEY_AUTOPILOT:
				cout << "Entering Auto Drive state." << endl;
				state = MAIN_STATE_AUTO_DRIVE;
				break;

			case KEY_TEST_FRAME:
				cout << "Testing frame." << endl;
				//m_nav.analyze_frame( m_camera.get_frame() );
				cout << "  Speed:" << m_nav.speed << endl;
				cout << "  Direc:" << m_nav.direction << endl;
				break;

			case KEY_HELP:
				main_print_usage();
				break;
			}
		}

		//run state machine
		switch( state ) {
		case MAIN_STATE_IDLE:
			break;

		case MAIN_STATE_MANUAL_DRIVE:
			//enter manual drive mode
			main_manual_drive();
			//when we exit, immediately change state
			state = MAIN_STATE_IDLE;
			break;

		case MAIN_STATE_AUTO_DRIVE:
			//enter auto drive mode
			main_auto_drive();
			//stop truck
			m_truck.set_drive(0);
			m_truck.set_steering(0);
			//when we exit, immediately change state
			state = MAIN_STATE_IDLE;
			break;

		case MAIN_STATE_CALIBRATE:
			//main_calibrate_drive();
			cout << "Calibrate not implemented." << endl;
			cout << "Returning to idle." << endl;
			state = MAIN_STATE_IDLE;
			break;
		}
	}

	return 0;
}

static void main_print_usage( void )
{
	printf( "Welcome to our truck. choose a key to continue:\n" );
	printf( "  %c - Calibrate\n", KEY_CALIBRATE );
	printf( "  %c - Manual Drive Mode\n", KEY_MANUAL );
	printf( "  %c - Autopilot mode\n", KEY_AUTOPILOT );
	printf( "  %c - Test Frame\n", KEY_TEST_FRAME );
	printf( "  %c - Help (this message)\n", KEY_HELP );
	printf( "  %c - Stop\n", KEY_STOP);

}

static void main_manual_drive( void )
{
	cout << "Entering manual drive mode." << endl;

	m_nav.speed = 0;
	m_nav.direction = 0;

	//we're in manual mode, enter while loop until user exits
	char c = 0;
	int speed = 0;
	int direction = 0;
	while( c != KEY_ESCAPE ) {
		//get key
		c = cv::waitKey(1);

		//do something here (modify this code!!!)
		switch( c ) {
			case 'l':
				//go left
				m_nav.direction -= 10;
				m_truck.set_steering( m_nav.direction );
				cout << "Left." << endl;
				main_report_nav();
				break;

			case 'r':
				//go right
				m_nav.direction += 10;
				m_truck.set_steering( m_nav.direction );
				cout << "Right." << endl;
				main_report_nav();
				break;

			case 'f':
				//go forward
				m_nav.speed += 10;
				m_truck.set_drive( m_nav.speed );
				cout << "Forward." << endl;
				main_report_nav();
				break;

			case 'b':
				//go backwards
				m_nav.speed -= 10;
				m_truck.set_drive( m_nav.speed );
				cout << "Backwards." << endl;
				main_report_nav();
				break;
		}
	}
}

static void main_auto_drive( void )
{
	// Print auto drive usage
	auto_print_usage();

	// Open display window
	cv::namedWindow("Auto Drive", CV_WINDOW_KEEPRATIO);

	// Analyze camera frame
	cv::Mat frame = m_camera.get_frame();
	cv::Mat display;

	char keypress;
	AUTO_STATE auto_state = AUTO_STATE_VIDEO;

	//wait for user to press escape
	while(keypress != 'q' && frame.data) {
		//analyze frame
		m_nav.analyze_frame(frame);

		//update truck
		m_truck.set_drive( m_nav.speed );
		m_truck.set_steering( m_nav.direction );

		cout << "Ready for keypress" << endl;

		// Wait for key press
		keypress = cv::waitKey(0);

		// Display output based on state machine
		switch(auto_state) {
			case AUTO_STATE_EDGES:
				display = m_nav.getEdges();
				break;

			case AUTO_STATE_OBSTACLES:
				display = m_nav.getObstacles();
				break;

			case AUTO_STATE_COMPOSITE:
				display = frame;
				break;
				
			case AUTO_STATE_VIDEO:
				display = frame;
				break;

			default:
				display = frame;
				break;
		}

		// Update statemachine based on keypress (for display output)
		switch(keypress) {
			case 'e':
				auto_state = AUTO_STATE_EDGES;
				break;

			case 'o':
				auto_state = AUTO_STATE_OBSTACLES;
				break;

			case 'c':
				auto_state = AUTO_STATE_COMPOSITE;
				break;

			case 'v':
				auto_state = AUTO_STATE_VIDEO;
				break;

			default:
				break;
		}

		// Display frame
		cv::imshow("Auto Drive", display);

		cout << "Ready to get next frame" << endl;

		//get next frame
		frame = m_camera.get_frame();
	}
}

void auto_print_usage()
{
	cout << "Auto drive mode:" << endl;
	cout << "\t'q' to quit" << endl;
	cout << "\t'e' view edges" << endl;
	cout << "\t'o' view obstacles" << endl;
}

static void main_calibrate_drive( void )
{
}

static void main_report_nav(void)
{
	cout << "speed    : " << m_nav.speed << endl;
	cout << "direction: " << m_nav.direction << endl;
}
