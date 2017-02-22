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
#define DEFAULT_STEERING 1500
#define DEFAULT_DRIVE 1500

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
        char nak = 0x15;
        ///Serial.write( nak );
        Serial.write( "Not Ready." );
      }
    }
    break;

    case STATE_RESET:
    {
      char ack = 0x06;
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
        m_state = STATE_GET_STEERING1;
        break;

        case 'd':
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
      //get the value
      m_setSteering = Serial.read();
      //change state
      m_state = STATE_GET_STEERING2;
    }
    break;

    case STATE_GET_STEERING2:
    //check if any data available on serial connection
    if( Serial.available() ) {
      //check that we got the same value
      if( m_setSteering == Serial.read() ) {
        char ack = 0x06;
        Serial.write(ack);
        //data must be between 1000 and 2000
        //so offset accordingly (assuming values -100 to 100)
        m_setSteering *= 5;
        m_setSteering += 1500;
        //write new value to servo
        m_steering.writeMicroseconds(m_setSteering);
      } else {
        char nak = 0x15;
        Serial.write(nak);
      }
      //go back to idle state
      m_state = STATE_IDLE;
    }
    break;

    case STATE_GET_DRIVE1:
    //check if any data available on serial connection
    if( Serial.available() ) {
      //get the value
      m_setDrive = Serial.read();
      //change state
      m_state = STATE_GET_DRIVE2;
    }
    break;

    case STATE_GET_DRIVE2:
    //check if any data available on serial connection
    if( Serial.available() ) {
      //check that we got the same value
      if( m_setDrive == Serial.read() ) {
        char ack = 0x06;
        Serial.write(ack);
        //data must be between 1000 and 2000
        //so offset accordingly (assuming values -100 to 100)
        m_setDrive *= 5;
        m_setDrive += 1500;
        //write new value to servo
        m_drive.writeMicroseconds(m_setDrive);
      } else {
        char nak = 0x15;
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






