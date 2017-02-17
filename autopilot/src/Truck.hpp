// Truck class for Odroid Truck Project
// Authors: James Swift, Luke Newmeyer

#pragma once

#include "Serial.hpp"

#include <iostream>
#include <termios.h>

#define DEBUG

class Truck {
	public:
		int connect_truck( void );
		void set_drive(char drive_speed);
		void set_steering(char steering_angle);

	private:
		int truckSerial;
		char drive_speed;
		char steering_angle;
		struct termios serialTermios;
		Serial p_serial;
		
		void wait_for_resp( int minBytes );
		void reset_truck( void );
};
