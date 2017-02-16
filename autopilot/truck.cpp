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
	truckSerial.open(serialName);
	
	// Check if connection open, error if not
	if(!truckSerial.is_open()) {
		cout << "Error: Failed to open serial connection" << endl;
		return -1;
	}
#ifdef DEBUG
	else {
		cout << "Opened serial port" << endl;
	}
#endif

	// Initialize connection
	//truckSerial << 'i';
	truckSerial.put('i');

#ifdef DEBUG
	cout << "Write 'i'" << endl;
#endif
	
	// Check connection
	char acknowledge;
	acknowledge = truckSerial.get();
	cout << "Acknowledge: " << std::hex << acknowledge << endl;
	
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
	truckSerial << location << value << value;
	
	// Read back acknowedge
	value = truckSerial.get();
	cout << "Acknowledge: " << std::hex << value << " at " << location << endl;
}
