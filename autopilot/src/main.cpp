// Odroid Vision Based Truck Autopilot
// Authors: James Swift, Luke Newmeyer

#include "Truck.hpp"

#include <iostream>
#include <unistd.h>
#include <termios.h>

#define KB_ESCAPE 27

using std::cout;
using std::cin;
using std::endl;

int main()
{
	Truck truck;
	
	truck.connect_truck();

	/*
	char kbCode = 0;
	char speed = 0;
	char steer = 0;
	while( kbCode != KB_ESCAPE ) {
		//kbCode = getchar();
		cin >> kbCode;
		cout << "pressed: " << std::hex << kbCode << endl;

		switch( kbCode ) {
			case 'l':
				steer -= 10;
				break;

			case 'r':
				steer += 10;
				break;

			case 'd':
				speed -= 10;
				break;

			case 'u':
				speed += 10;
				break;

		}

		truck.set_steering( steer );
		truck.set_drive( speed );
	}
	*/

	return 0;
}
