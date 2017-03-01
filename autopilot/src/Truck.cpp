/******************************************************************************
 * Class Implementation
 *
 * Authors: James Swift, Luke Newmeyer
 * Copyright 2017
 *****************************************************************************/
/*************************** Include Files ***********************************/

#include "Truck.hpp"

#include <string>
#include <string.h>
#include <fcntl.h>
//#include <unistd.h>
#include <stdio.h>

/*************************** Definitions *************************************/

#define TRUCK_TIMEOUT 50000000L

using std::cout;
using std::endl;
using std::ofstream;
using std::ifstream;
using std::string;

/*************************** Implementation **********************************/

int Truck::connect_truck( void )
{
#ifdef SERIAL_USE_FILE
	return 0;
#endif

#ifdef DEBUG
	cout << "Opening Truck" << endl;
#endif

	if( p_serial.open() != 0 ) {
		cout << "Error: Failed to open serial connection" << endl;
		return -1;
	}
#ifdef DEBUG
	else {
		cout << "Opened serial port" << endl;
	}
#endif

#ifdef DEBUG
	cout << "Write 'i'" << endl;
#endif

	//wait for initialization flag
	wait_for_resp( 11 );

	//read bytes
	while( p_serial.hitc() ) {
		int c = p_serial.getc();
#ifdef DEBUG
		printf("Reading: %c\n", c );
#endif
	}
	
	// Initialize connection
	char init = 'i';
	p_serial.putc( 'i' );

	//wait for response
	wait_for_resp( 6 );

	//read bytes
	while( p_serial.hitc() ) {
		int c = p_serial.getc();
#ifdef DEBUG
		printf("Reading: %c\n", c );
#endif
	}

#ifdef DEBUG
	//test steering
	set_steering( 100 );
	//sleep(1);
	set_steering( 0 );

	//test drive	
	//set_drive( 1 );
	//sleep(1);
	//set_drive( -1 );
	//sleep(1);
	//set_drive( 0 );
#endif

	// Exit with success
	return 0;
}

void Truck::set_drive(char drive_speed)
{
	int tries;
	
	//for now, limit it to 15
	drive_speed = (char)(((int)drive_speed * 15 ) / 100 );
	
	//truck accepts 0 to 200 with 100 being center
	//this function accepts -100 to 100 (offset)
	drive_speed += 100; 

	for( tries = 0; tries < 3; tries++ ) {
		char driveCmd[5];
		driveCmd[0] = 'd';
		driveCmd[1] = (drive_speed/100) + '0';
		drive_speed = drive_speed % 100;
		driveCmd[2] = (drive_speed/10) + '0';
		drive_speed = drive_speed % 10;
		driveCmd[3] = (drive_speed) + '0';
		driveCmd[4] = '\n';
		p_serial.write( driveCmd, 5 );

		//driveCmd[4] = '\0';
		//cout << "Drive: " << driveCmd << endl;

		//check for ack
		wait_for_resp(1);
#ifndef SERIAL_USE_FILE
		if( p_serial.getc() == SERIAL_ACK ) {
#else
		if( 1 ) {
#endif
			break;
		} else {
			//crap, reset truck
			reset_truck();
		}
		break;
	}

	if( tries == 3 ) {
		cout << "Truck Error: Couldn't set steering!\n";
	}

}

void Truck::set_steering(char steering_angle)
{
	int tries;

	//left is actualy positive, bleh
	steering_angle *= -1;

	//truck accepts 0 to 200 with 100 being center
	//this function accepts -100 to 100 (offset)
	steering_angle += 100; 

	//try 3 times
	for( tries = 0; tries < 3; tries++ ) {
		char cmd[5];
		cmd[0] = 's';
		cmd[1] = (steering_angle/100) + '0';
		steering_angle= steering_angle% 100;
		cmd[2] = (steering_angle/10) + '0';
		steering_angle= steering_angle% 10;
		cmd[3] = (steering_angle) + '0';
		cmd[4] = '\n';
		p_serial.write( cmd, 5 );
		
		//cmd[4] = '\0';
		//cout << "Steering: " << cmd << endl;


		//check for ack
		wait_for_resp(1);
#ifndef SERIAL_USE_FILE
		if( p_serial.getc() == SERIAL_ACK ) {
#else
		if( 1 ) {
#endif
			break;
		} else {
			//crap, reset truck
			reset_truck();
		}
	}

	if( tries == 3 ) {
		cout << "Truck Error: Couldn't set steering!\n";
	}
}

void Truck::wait_for_resp( int minBytes )
{
#ifndef SERIAL_USE_FILE
	long timeout = 0;
	while( p_serial.bytes_available() < minBytes &&
			timeout < TRUCK_TIMEOUT ) {
		timeout++;
	}

	return;
#endif
}

void Truck::reset_truck( void )
{
#ifndef SERIAL_USE_FILE
	//send command to reset a few times to work state machine
	p_serial.putc( 'r' );
	p_serial.putc( 'r' );
	p_serial.putc( 'r' );

	//wait for response
	wait_for_resp( 6 );
	//flush buffer
	while( p_serial.hitc() ) p_serial.getc();
	//ensure no more is comming
	wait_for_resp( 6 );
	//flush anything that came in
	while( p_serial.hitc() ) p_serial.getc();
#endif
}
