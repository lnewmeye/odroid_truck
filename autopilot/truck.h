// Truck class for Odroid Truck Project
// Authors: James Swift, Luke Newmeyer

#pragma once

// For debug purposes
#include <iostream>
#include <fstream>
#include <string>

#define DEBUG

class Truck {
	public:
		int connect_truck(std::string serialName);
		void set_drive(char drive_speed);
		void set_steering(char steering_angle);
	private:
		void write_serial(char location, char value);
		std::fstream truckSerial;
		char drive_speed;
		char steering_angle;
};
