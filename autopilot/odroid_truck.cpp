// Odroid Vision Based Truck Autopilot
// Authors: James Swift, Luke Newmeyer

#include <iostream>
#include "truck.h"

using std::cout;
using std::endl;

//#define SERIAL_PORT "/dev/ttyACM0"
#define SERIAL_PORT "test.txt"

int main()
{
	Truck truck;
	
	truck.connect_truck(SERIAL_PORT);
	
	truck.set_steering(100);

	return 0;
}
