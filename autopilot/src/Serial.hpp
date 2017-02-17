/****************************************************************************
 * Serial Class - This may source directly from a serial port or from a file
 * 		  depending if the SERIAL_USE_FILE definition is set.
 ****************************************************************************/
#pragma once

/****************************** Include Files ******************************/

//#define SERIAL_USE_FILE "serial.txt"

class Serial {
	public:
		Serial();
		bool open();
		bool hitc();
		char getc();
		int read( char *data, int size);
		void putc( char c);
		void write( char c);
		void write( char *data, int size);
	private:
		int p_fd;
		int set_interface_attribs(int fd, int speed, int parity);

};
