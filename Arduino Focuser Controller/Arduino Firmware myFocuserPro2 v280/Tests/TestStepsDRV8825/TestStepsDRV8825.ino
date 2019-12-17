// myfocuser DRV8825 HW203
// ----------------------------------------------------------------------------------------------------------
// FIRMWARE CODE START
#include <Arduino.h>
// ----------------------------------------------------------------------------------------------------------
#define SERIALPORTSPEED     9600          // 9600, 14400, 19200, 28800, 38400, 57600

#define DRV8825DIR          3             // drv8825 control lines
#define DRV8825STEP         4
#define DRV8825ENABLE       8
#define DRV8825M2           5             // for easydriver, do not connect
#define DRV8825M1           6             // for easydriver, connect to MS1
#define DRV8825M0           7             // for easydriver, connect to MS0
#define SLOW                0             // motorspeeds
#define MED                 1
#define FAST                2
#define STEP1               1             // step modes
#define STEP2               2
#define STEP4               4
#define STEP8               8
#define STEP16              16
#define STEP32              32
#define STEP64              64
#define STEP128             128

#define FAST1DELAY          1500          // delay times - values are based on BOTH step mode and motor speed settings
#define FAST2DELAY          600
#define FAST4DELAY          250
#define FAST8DELAY          50
#define FAST16DELAY         20
#define FAST32DELAY         10
#define FAST64DELAY         10
#define FAST128DELAY        10
#define MED1DELAY           3000
#define MED2DELAY           1800
#define MED4DELAY           1000
#define MED8DELAY           400
#define MED16DELAY          200
#define MED32DELAY          100
#define MED64DELAY          50
#define MED128DELAY         10
#define SLOW1DELAY          6000
#define SLOW2DELAY          3600
#define SLOW4DELAY          2000
#define SLOW8DELAY          800
#define SLOW16DELAY         400
#define SLOW32DELAY         200
#define SLOW64DELAY         100
#define SLOW128DELAY        20

#define MOTORPULSETIME      2             // drv8825 requires minimum 2uS pulse to step
#define MOVINGIN            0
#define MOVINGOUT           1

int motorSpeed = FAST;
int stepmode = STEP2;
int motorSpeedDelay;


// disable the stepper motor outputs - coil power off
void disableoutput()
{
  digitalWrite(DRV8825ENABLE, HIGH);
  // control via ENABLE pin, but this turns off indexer inside DRV8825
  // which means the stepper motor will loose power and move position to the nearest full step
  // not an issue if using full steps but major issue if using microstepping as will cause change
  // in focus position
}

// enable the stepper motor outputs - coil power on
void enableoutput()
{
  digitalWrite(DRV8825ENABLE, LOW);
}

// Move stepper anticlockwise
void anticlockwise()
{
  digitalWrite(DRV8825DIR, LOW );
  digitalWrite(DRV8825STEP, 1 );
  delayMicroseconds(MOTORPULSETIME);
  digitalWrite(DRV8825STEP, 0 );
}

// Move stepper clockwise
void clockwise()
{
  digitalWrite(DRV8825DIR, HIGH );
  digitalWrite(DRV8825STEP, 1 );
  delayMicroseconds(MOTORPULSETIME);
  digitalWrite(DRV8825STEP, 0 );
}

// set the microstepping mode for DRV8825
void setstepmode( int stepmode)
{
  switch ( stepmode )
  {
    case 1:                               // full step
      digitalWrite(DRV8825M0, 0);
      digitalWrite(DRV8825M1, 0);
      digitalWrite(DRV8825M2, 0);
      break;
    case 2:                               // half step
      digitalWrite(DRV8825M0, 1);
      digitalWrite(DRV8825M1, 0);
      digitalWrite(DRV8825M2, 0);
      break;
    case 4:                               // quarter step
      digitalWrite(DRV8825M0, 0);
      digitalWrite(DRV8825M1, 1);
      digitalWrite(DRV8825M2, 0);
      break;
    case 8:                               // eighth step
      digitalWrite(DRV8825M0, 1);
      digitalWrite(DRV8825M1, 1);
      digitalWrite(DRV8825M2, 0);
      break;
    case 16:                              // sixteenth step
      digitalWrite(DRV8825M0, 0);
      digitalWrite(DRV8825M1, 0);
      digitalWrite(DRV8825M2, 1);
      break;
    case 32:                              // thirty-two step
      digitalWrite(DRV8825M0, 1);
      digitalWrite(DRV8825M1, 0);
      digitalWrite(DRV8825M2, 1);
      break;
    default:                              // half step
      digitalWrite(DRV8825M0, 1);
      digitalWrite(DRV8825M1, 0);
      digitalWrite(DRV8825M2, 0);
      stepmode = 2;
      break;
  }
}

void updatemotorSpeedDelay()
{
  switch ( motorSpeed )                   // calculate delay based on stepmode and motorspeed
  {
    case SLOW:
      switch ( stepmode )
      {
        case STEP1:
          motorSpeedDelay = SLOW1DELAY;
          break;
        case STEP2:
          motorSpeedDelay = SLOW2DELAY;
          break;
        case STEP4:
          motorSpeedDelay = SLOW4DELAY;
          break;
        case STEP8:
          motorSpeedDelay = SLOW8DELAY;
          break;
        case STEP16:
          motorSpeedDelay = SLOW16DELAY;
          break;
        case STEP32:
          motorSpeedDelay = SLOW32DELAY;
          break;
      }
      break;
    case MED:
      switch ( stepmode )
      {
        case STEP1:
          motorSpeedDelay = MED1DELAY;
          break;
        case STEP2:
          motorSpeedDelay = MED2DELAY;
          break;
        case STEP4:
          motorSpeedDelay = MED4DELAY;
          break;
        case STEP8:
          motorSpeedDelay = MED8DELAY;
          break;
        case STEP16:
          motorSpeedDelay = MED16DELAY;
          break;
        case STEP32:
          motorSpeedDelay = MED32DELAY;
          break;
      }
      break;
    case FAST:
      switch ( stepmode )
      {
        case STEP1:
          motorSpeedDelay = FAST1DELAY;
          break;
        case STEP2:
          motorSpeedDelay = FAST2DELAY;
          break;
        case STEP4:
          motorSpeedDelay = FAST4DELAY;
          break;
        case STEP8:
          motorSpeedDelay = FAST8DELAY;
          break;
        case STEP16:
          motorSpeedDelay = FAST16DELAY;
          break;
        case STEP32:
          motorSpeedDelay = FAST32DELAY;
          break;
      }
      break;
  }
}

// Setup
void setup()
{
  Serial.begin(SERIALPORTSPEED);            // initialize serial port

  pinMode(  DRV8825DIR, OUTPUT );
  pinMode(  DRV8825STEP, OUTPUT );
  pinMode(  DRV8825M0, OUTPUT );
  pinMode(  DRV8825M1, OUTPUT );
  pinMode(  DRV8825M2, OUTPUT );
  digitalWrite( DRV8825DIR, 0 );                 // set direction and step to low
  digitalWrite( DRV8825STEP, 0 );
  pinMode( DRV8825ENABLE, OUTPUT );              // enable the driver board

  enableoutput();
  motorSpeed = FAST;
  setstepmode(stepmode);
  updatemotorSpeedDelay();

}

// Main Loop
void loop()
{
  delay(2000);

  // move stepper clockwise 1000
  for ( int lp = 0; lp < 1000; lp++ )
  {
    clockwise();
    delayMicroseconds(motorSpeedDelay);
  }

  delay(2000);

  // move stepper anticlockwise 1000
  for ( int lp = 0; lp < 1000; lp++ )
  {
    anticlockwise();
    delayMicroseconds(motorSpeedDelay);
  }

  delay(2000);

}

