// Truck class for Odroid Truck Project
// Authors: James Swift, Luke Newmeyer

#include "truck.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::ifstream;
using std::string;

int Truck::connect_truck(string serialName)
{
#ifdef DEBUG
	cout << "Opening Truck" << endl;
#endif
	
	// Open serial connection
	truckSerial = open(serialName.c_str(), O_RDWR | O_NONBLOCK);

	// Check if connection open, error if not
	if(truckSerial < 0) {
		cout << "Error: Failed to open serial connection" << endl;
		return -1;
	}
#ifdef DEBUG
	else {
		cout << "Opened serial port" << endl;
	}
#endif
	
	// Setup struct for serial connection
	memset(&serialTermios, 0, sizeof(serialTermios));
	serialTermios.c_iflag = 0;
	serialTermios.c_oflag = 0;
	serialTermios.c_cflag = 0;
	serialTermios.c_lflag = 0;
	serialTermios.c_cc[VMIN] = 1;
	serialTermios.c_cc[VTIME] = 5;
	cfsetospeed(&serialTermios, B115200);
	cfsetispeed(&serialTermios, B115200);
	
	// Set serial attributes
	tcsetattr(truckSerial, TCSANOW, &serialTermios);

	// Wait for Arduino reset
	usleep(100000);
	
	// Initialize connection
	char init = 'i';
	write(truckSerial, &init, sizeof(char));

#ifdef DEBUG
	cout << "Write 'i'" << endl;
#endif
	
	// Check connection
	char acknowledge;
	read(truckSerial, &acknowledge, sizeof(char));
	printf("Acknowledge: %X\n", acknowledge);
	//cout << "Acknowledge: " << std::hex << acknowledge << endl;
	
	// Exit with success
	return 0;
}

void Truck::set_drive(char drive_speed)
{
	this->drive_speed = drive_speed;
	
}

void Truck::set_steering(char steering_angle)
{
	this->steering_angle = steering_angle;
}


void Truck::write_serial(char location, char value)
{
	// Write location and value to truck
	//truckSerial << location << value << value;
	
	// Read back acknowedge
	//value = truckSerial.get();
	cout << "Acknowledge: " << std::hex << value << " at " << location << endl;
}
