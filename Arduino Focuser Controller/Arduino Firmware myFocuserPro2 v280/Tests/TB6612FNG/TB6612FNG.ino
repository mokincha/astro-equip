// TB6612FNG TEST CODE
// ONLY FOR USE WITH TB6612FNG Driver Board - supports 1.2A at 12VDC - FULL/HALF STEPS ONLY
// BIPOLAR STEPPER MOTORS ONLY
//
// The TB6612FNG is meant to be a replacement for the L298N driver chip

// (c) Copyright Robert Brown 2014-2018. All Rights Reserved.

// ----------------------------------------------------------------------------------------------------------
// PCB BOARDS
// TB6612TNG-M-MT-F-BT
// https://aisler.net/p/XGJMJQGP

// ----------------------------------------------------------------------------------------------------------
// HARDWARE MAPPINGS

// TB16612FNG BOARD
// VMOTOR to 12V
// VCC to 5V
// GND to GND
// AIN2 to D6
// AIN1 to D5
// BIN1 to D7
// BIN2 to D8
// PWMA and PWMB and STANDBY to VCC/5V
// STEPPER MOTOR WIRING
// One coil pair to MOTOR A
// One coil pair to MOTOR B

// ----------------------------------------------------------------------------------------------------------
// FIRMWARE CODE START

#include <Arduino.h>
#include <myHalfStepper.h>          // Author: Tom Biuso <tjbtech@yahoo.com>

// ----------------------------------------------------------------------------------------------------------
// GLOBAL DEFINES
// DO NOT CHANGE
#define SERIALPORTSPEED     9600          // serial port speed
#define SLOW                0             // motorspeeds
#define MED                 1
#define FAST                2
#define STEP1               1             // step modes
#define STEP2               2

// stepper motor steps per full revolution using full steps
#define stepsPerRevolution  200           // NEMA17 change this to fit the number of steps per revolution
// you need to change the above line to reflect your stepper motor, examples below
// #define stepsPerRevolution 1028        // NEMA17-PG5 motor, if half stepping multiply by 2
// #define stepsPerRevolution 200         // NEMA17 motor, if half stepping multiply by 2
// #define stepsPerRevolution 5370        // NEMA17-PG25 motor, if half stepping multiply by 2
#define IN1                 5             // define TB6612FNG inputs
#define IN2                 6
#define IN3                 7
#define IN4                 8
#define motorSpeedSlowRPM   10
#define motorSpeedMedRPM    30
#define motorSpeedFastRPM   50
#define MOTORPULSETIME      5             // requires minimum 5uS pulse to step
#define motorSpeedDelay     5
int motorSpeedRPM;                        // the motorspeed RPM setting
int stepmode;

// initialize the stepper library
HalfStepper mystepper(stepsPerRevolution, IN1, IN2, IN3, IN4);

// disable the stepper motor outputs - coil power off
void clearOutput()
{
    digitalWrite( IN1, false);
    digitalWrite( IN2, false);
    digitalWrite( IN3, false);
    digitalWrite( IN4, false);
}

// Move stepper anticlockwise
void anticlockwise()
{
  mystepper.step(-1);
  delayMicroseconds(MOTORPULSETIME);
}

// Move stepper clockwise
void clockwise()
{
  mystepper.step(1);
  delayMicroseconds(MOTORPULSETIME);
}

// set stepper speed based on stepmode
void setstepperspeed( int sspeed)
{
  if ( stepmode == STEP1 )
  {
    mystepper.setSpeed(sspeed);
  }
  else
  {
    mystepper.setSpeed(sspeed * 3);
  }
}

// set the microstepping mode
void setstepmode( int smode )
{
  if ( smode == STEP1 )
  {
    mystepper.SetSteppingMode(SteppingMode::FULL);
  }
  else if ( smode == STEP2 )
  {
    mystepper.SetSteppingMode(SteppingMode::HALF);
  }
  else
  {
    mystepper.SetSteppingMode(SteppingMode::FULL);
  }
  setstepperspeed(motorSpeedRPM);
}

// Setup
void setup()
{
  Serial.begin(SERIALPORTSPEED);            // initialize serial port

  pinMode( IN1, OUTPUT );  // setup INx step lines as outputs, set low so motor does not move
  pinMode( IN2, OUTPUT );
  pinMode( IN3, OUTPUT );
  pinMode( IN4, OUTPUT );
  digitalWrite( IN1, false);
  digitalWrite( IN2, false);
  digitalWrite( IN3, false);
  digitalWrite( IN4, false);

  stepmode = STEP1;
  motorSpeedRPM = motorSpeedFastRPM;
  setstepperspeed(motorSpeedRPM);
}

// Main Loop
void loop()
{
  int lp;

  Serial.println("500 steps anticlockwise full steps fast");
  stepmode = STEP1;
  motorSpeedRPM = motorSpeedFastRPM;
  setstepmode(stepmode);
  for ( lp = 0; lp < 500; lp++ )
  {
    anticlockwise();
    delay( motorSpeedDelay );  // required else stepper will not move
  }

  delay(2000);

  Serial.println("500 steps clockwise full steps fast");
  stepmode = STEP1;
  motorSpeedRPM = motorSpeedFastRPM;
  setstepmode(stepmode);
  for ( lp = 0; lp < 500; lp++ )
  {
    clockwise();
    delay( motorSpeedDelay );  // required else stepper will not move
  }

  delay(2000);

  Serial.println("500 steps anticlockwise half steps fast");
  stepmode = STEP2;
  motorSpeedRPM = motorSpeedFastRPM;
  setstepmode(stepmode);
  for ( lp = 0; lp < 500; lp++ )
  {
    anticlockwise();
    delay( motorSpeedDelay );  // required else stepper will not move
  }

  clearOutput();
  delay(2000);

  Serial.println("500 steps clockwise half steps fast");
  stepmode = STEP2;
  motorSpeedRPM = motorSpeedFastRPM;
  setstepmode(stepmode);
  for ( lp = 0; lp < 500; lp++ )
  {
    clockwise();
    delay( motorSpeedDelay );  // required else stepper will not move
  }

  clearOutput();
  delay(2000);

  Serial.println("500 steps anticlockwise full steps slow");
  stepmode = STEP1;
  motorSpeedRPM = motorSpeedSlowRPM;
  setstepmode(stepmode);
  for ( lp = 0; lp < 500; lp++ )
  {
    anticlockwise();
    delay( motorSpeedDelay );  // required else stepper will not move
  }

  delay(2000);


  Serial.println("500 steps clockwise full steps slow");
  stepmode = STEP1;
  motorSpeedRPM = motorSpeedSlowRPM;
  setstepmode(stepmode);
  for ( lp = 0; lp < 500; lp++ )
  {
    clockwise();
    delay( motorSpeedDelay );  // required else stepper will not move
  }

  delay(2000);
  clearOutput();
}
