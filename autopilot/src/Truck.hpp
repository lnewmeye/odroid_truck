/****************************************************************************
 * Truck Class - This class defines a truck navigated through a serial 
 *				 connection. 
 *
 * Authors: James Swift, LukeNewmeyer
 * Copyright 2017
 ****************************************************************************/
#pragma once

/****************************** Include Files ********************************/

#include "Serial.hpp"
#include <iostream>

//#define DEBUG
#define SERIAL_USE_FILE

class Truck {
	//methods
public:
	int connect_truck(void);
	void set_drive(char drive_speed);
	void set_steering(char steering_angle);

	//private variables
private:
	Serial p_serial;

	//private methods
private:
	void wait_for_resp(int minBytes);
	void reset_truck(void);
};
