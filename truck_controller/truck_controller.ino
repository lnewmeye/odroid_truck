/******************************************************************************
* James Swift
******************************************************************************/
#include <Servo.h>

/**************************** Definitions ************************************/

/** Velocity servo is connected to this pin number */
#define PIN_DRIVE 3
/** Steering servo is connected to this pin number */
#define PIN_STEERING 5
/** Define baud rate */
#define SERIAL_BAUD_RATE 115200
/** Default servo values */
#define DEFAULT_STEERING 1515
#define DEFAULT_DRIVE 1500

#define SERIAL_ACK 'y'
#define SERIAL_NAK 'n'

/** State machine for receiving data from serial connection */
typedef enum STATE_E {
  STATE_INIT = 0,
  STATE_RESET,
  STATE_IDLE,
  STATE_GET_STEERING1,
  STATE_GET_STEERING2,
  STATE_GET_DRIVE1,
  STATE_GET_DRIVE2,
  STATE_SEND_SPEED_ENCODER1,
  STATE_SEND_SPEED_ENCODER2,
  STATE_NUMS
} STATE_T;

/**************************** Variables **************************************/

/** state machine variable */
static STATE_T m_state;
/** Current set speed */
static int m_setDrive;
/** Current set Steering */
static int m_setSteering;
/** Current Speed */
static int m_currSpeed;

/** Steering servo */
Servo m_steering;
/** Velocity servo */
Servo m_drive;

/** Endoder times */
static unsigned long m_prevtimes[3];

/**************************** Private Function Declaration ******************/
static void get_input( void );

/**************************** Implementation *********************************/

void setup() {
  //initialize servos
  m_steering.attach(PIN_STEERING);
  m_drive.attach(PIN_DRIVE);

  //initialize variables
  m_state = STATE_INIT;
  
   //initialize serial communication
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.write( "Starting..." );
}

void loop() {
  //check for user input
  get_input();
}

/**************************** Private Function Implementation ****************/

static void get_input( void ) {
  //check state
   switch( m_state ) {

    case STATE_INIT:
    if( Serial.available() ) {
      char c = Serial.read();
      if( c == 'i' ) {
        m_state = STATE_RESET;
      } else {
        char nak = SERIAL_NAK;
        ///Serial.write( nak );
        Serial.write( "Not Ready." );
      }
    }
    break;

    case STATE_RESET:
    {
      char ack = SERIAL_ACK;
      m_steering.writeMicroseconds( DEFAULT_STEERING );
      m_drive.writeMicroseconds( DEFAULT_DRIVE );
      //Serial.write( ack );
      Serial.write( "Ready." );
      m_state = STATE_IDLE;
    }
    break;
    
    case STATE_IDLE:
    //check if any data available on serial connection
    if( Serial.available() ) {
      //get the command
      char c = Serial.read();
      //check command
      switch( c ) {
        case 's':
        m_setSteering = 0;
        m_state = STATE_GET_STEERING1;
        break;

        case 'd':
        m_setDrive = 0;
        m_state = STATE_GET_DRIVE1;
        break;

        case 'e':
        m_state = STATE_SEND_SPEED_ENCODER1;
        break;

        case 'r':
        m_state = STATE_RESET;
        break;
      }
    }
    break;

    case STATE_GET_STEERING1:
    //check if any data available on serial connection
    if( Serial.available() ) {
      int i;
      i = Serial.read();
      if( i > 0 ) {
        if( i >= '0' && i <= '9' ) {
          m_setSteering = m_setSteering*10 + (i-'0');
        } else if( i == '\n' ) {
          //Serial.write( "Setting steering to: " );
          //Serial.print( m_setSteering, DEC );
          //Serial.print( '\n' );
          //data must be between 1000 and 2000
          //so offset accordingly (assuming values 0 to 200)
          m_setSteering *= 5;
          m_setSteering += 1015;
          m_steering.writeMicroseconds(m_setSteering);
          Serial.write( 'y' );
          m_state = STATE_IDLE;
        } else {
          Serial.write( 'n' );
          m_state = STATE_IDLE;
        }
      }
    }
    break;

    case STATE_GET_DRIVE1:
    //check if any data available on serial connection
    if( Serial.available() ) {
      int i;
      i = Serial.read();
      if( i > 0 ) {
        if( i >= '0' && i <= '9' ) {
          m_setDrive = m_setDrive*10 + (i-'0');
        } else if( i == '\n' ) {
          //Serial.write( "Setting drive to: " );
          //Serial.print( m_setDrive, DEC );
          //Serial.print( '\n' );
           m_setDrive *= 5;
          m_setDrive += 1000;
          m_drive.writeMicroseconds(m_setDrive);
          Serial.write( 'y' );
          m_state = STATE_IDLE;
        } else {
          Serial.write( 'n' );
          m_state = STATE_IDLE;
        }
      }
    }
    break;

    case STATE_GET_DRIVE2:
    //check if any data available on serial connection
    if( Serial.available() ) {
      //check that we got the same value
      if( m_setDrive == Serial.read() ) {
        char ack = SERIAL_ACK;
        Serial.write(ack);
        //data must be between 1000 and 2000
        //so offset accordingly (assuming values 0 to 200)
        m_setDrive *= 5;
        m_setDrive += 1000;
        //write new value to servo
        m_drive.writeMicroseconds(m_setDrive);
      } else {
        char nak = SERIAL_NAK;
        Serial.write(nak);
      }
      //go back to idle state
      m_state = STATE_IDLE;
    }
    break;

    case STATE_SEND_SPEED_ENCODER1:
    //go back to idle state
    m_state = STATE_IDLE;
    break;

    default:
    m_state = STATE_IDLE;
  }
}






