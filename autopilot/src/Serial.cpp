/******************************************************************************
 * Class Implementation
 *
 * Authors: James Swift, Luke Newmeyer
 * Copyright 2017
 *****************************************************************************/
/****************************** Include Files ********************************/

#include <iostream>
#include "Serial.hpp"

#include <string.h>

#ifndef SERIAL_USE_FILE
#include <errno.h>
#include <fcntl.h> 
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

/****************************** Definitions **********************************/

//#define SERIAL_PORT "/dev/ttyUSB0"
#define SERIAL_PORT "/dev/ttyACM0"
#define MICROSEC_PER_BYTE 87


using std::cout;
using std::string;

/****************************** Implementation *******************************/

Serial::Serial( void )
{
}

bool Serial::open( void )
{
#ifdef SERIAL_USE_FILE
#else
	p_fd = ::open( SERIAL_PORT, O_RDWR | O_NOCTTY | O_SYNC );
	if( p_fd < 0 ) {
		cout <<  "Error: " << errno << "opening " << SERIAL_PORT << " " << strerror (errno);
		return -1;
	}

	if( set_interface_attribs( p_fd, B115200, 0) != 0 ) {
		cout <<  "Serial Error: Couldn't set attributes\n";
		return -1;
	}
#endif
	
	return 0;
}

bool Serial::hitc( void ) 
{
	int bytesAvail;
#ifdef SERIAL_USE_FILE
	bytesAvail = 1;
#else
	ioctl( p_fd, FIONREAD, &bytesAvail );
#endif
	return ( bytesAvail > 0 );
}

int Serial::bytes_available( void ) 
{
	int bytesAvail;
#ifdef SERIAL_USE_FILE
	bytesAvail = 1;
#else
	ioctl( p_fd, FIONREAD, &bytesAvail );
#endif
	return bytesAvail;
}

char Serial::getc( void ) 
{
	char c;
#ifdef SERIAL_USE_FILE
	c = 0;
#else
	//don't use read from this class (::)
	::read( p_fd, &c, 1);
#endif
	return c;
}

int Serial::read( char *data, int size)
{
#ifdef SERIAL_USE_FILE
	return 0;
#else
	//don't use read from this class (::)
	return ::read( p_fd, data, size );
#endif
}

void Serial::putc( char c )
{
#ifdef SERIAL_USE_FILE
#else
	//don't use write from this class (::)
	::write( p_fd, &c, 1);
	//allow time to write
	usleep( MICROSEC_PER_BYTE );
#endif
}

void Serial::write( char *data, int size)
{
#ifdef SERIAL_USE_FILE
#else
	//don't use write from this class (::)
	::write( p_fd, data, size);
	//allow time to write
	usleep( MICROSEC_PER_BYTE*size );
#endif
}

#ifndef SERIAL_USE_FILE
int Serial::set_interface_attribs (int fd, int speed, int parity)
{
	struct termios tty;

	//reset structure
	memset (&tty, 0, sizeof tty);

	//get structure
	if (tcgetattr (fd, &tty) != 0)
	{
		cout << "Serial Error: " << errno << " from atcgetattr.\n";
		return -1;
	}

	//set speeds
	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	//local fields (
	tty.c_lflag = 0;         // no signaling chars, no echo,
	//tty.c_lflag |= NOFLSH;  //no flush after interrupt

	// disable IGNBRK for mismatched speed tests; 
	// otherwise receive break as \000 chars
	tty.c_iflag &= ~IGNBRK;  // disable break processing
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
	tty.c_iflag &= ~(INLCR); //don't change ln to cr

	// no canonical processing
	tty.c_oflag = 0;         // no remapping, no delays

	// enable reading
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	//tty.c_cflag &= ~HUPCL;              // don't reset on close
	tty.c_cflag |= (CLOCAL | CREAD);    // ignore modem controls,
	tty.c_cflag &= ~(PARENB | PARODD);  // shut off parity
	tty.c_cflag |= parity;              // parity enable
	tty.c_cflag &= ~CSTOPB;             // don't send 2 stop bits
	tty.c_cflag &= ~CRTSCTS;            // CRT/CTS

	//control codes
	tty.c_cc[VMIN]  = 0;     // min bytes for read: read doesn't block
	tty.c_cc[VTIME] = 5;     // timeout: 0.5 seconds read timeout

	//set attributes
	if (tcsetattr (fd, TCSANOW, &tty) != 0)
	{
		cout << "Serial error: " << errno << " from tcsetattr.\n";
		return -1;
	}

	return 0;
}
#endif
