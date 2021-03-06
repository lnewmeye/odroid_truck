/******************************************************************************
 * Serial Class - This may source directly from a serial port or from a file
 * 		  depending if the SERIAL_USE_FILE definition is set.
 *
 * Authors: James Swift, Luke Newmeyer
 * Copyright 2017
 *****************************************************************************/
#pragma once

/****************************** Include Files ********************************/

//#define SERIAL_USE_FILE "serial.txt"

#define SERIAL_ACK 'y'
#define SERIAL_NAK 'n'

class Serial {
	public:
		Serial();
		bool open();
		bool hitc();
		int bytes_available();
		char getc();
		int read( char *data, int size);
		void putc( char c);
		void write( char *data, int size);
	private:
		int p_fd;
		int set_interface_attribs(int fd, int speed, int parity);

};
