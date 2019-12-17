// TEST_L9110S
// (c) R Brown 2017
// All rights reserved

// Test code for myFocuserPro with L9110S driver board
// Full steps only

#include <Arduino.h>
#include <myStepper.h>

// define L9110S inputs
#define A1A   3
#define A1B   4
#define B1A   5
#define B1B   6

// stepper motor steps per full revolution using full steps
// const int stepsPerRevolution = 200;        // NEMA17 change this to fit the number of steps per revolution
// you need to change the above line to reflect your stepper motor, examples below
// const int stepsPerRevolution = 1028;       // NEMA17-PG5 motor
// const int stepsPerRevolution = 200;        // NEMA17 motor
const int stepsPerRevolution = 5370;          // NEMA17-PG27 motor

// initialize the stepper library
Stepper mystepper(stepsPerRevolution, A1A, A1B, B1A, B1B);

const int    motorSpeedSlowRPM = 10;
const int    motorSpeedMedRPM  = 30;
const int    motorSpeedFastRPM = 50;
// motorspeeddelay is a fixed value
int motorSpeedDelay = 5;                      // the delay in millseconds between steps
int motorSpeedRPM = motorSpeedSlowRPM;        // the motorspeed RPM setting
int motorSpeed = 0;                           // 0=slow, 1=medium, 2=fast, default=slow on startup only
int savedmotorSpeed = motorSpeed;             // used to save original speed if slowing down when nearing target position
int stepontime = 5;                           // minimum step pulse time

// define serial port speed - valid values are 9600, 57600
#define SerialPortSpeed 9600

boolean ReverseDirection;

// disable the stepper motor outputs - coil power off
void clearOutput()
{
    digitalWrite( A1A, false);
    digitalWrite( A1B, false);
    digitalWrite( B1A, false);
    digitalWrite( B1B, false);
}

// Move stepper anticlockwise
void anticlockwise()
{
  if ( !ReverseDirection )
  {
    mystepper.step(1);
    delayMicroseconds(stepontime);
  }
  else
  {
    mystepper.step(-1);
    delayMicroseconds(stepontime);
  }
}

// Move stepper clockwise
void clockwise()
{
  if ( !ReverseDirection )
  {
    mystepper.step(-1);
    delayMicroseconds(stepontime);
  }
  else
  {
    mystepper.step(1);
    delayMicroseconds(stepontime);
  }
}

void setup()
{
  Serial.begin(SerialPortSpeed);

  // setup pins to control LS9110S
  pinMode( A1A, OUTPUT);
  pinMode( A1B, OUTPUT);
  pinMode( B1A, OUTPUT);
  pinMode( B1B, OUTPUT);
  // set low so motor does not move
  digitalWrite( A1A, false);
  digitalWrite( A1B, false);
  digitalWrite( B1A, false);
  digitalWrite( B1B, false);
  
  ReverseDirection = false;

  mystepper.setSpeed(motorSpeedRPM);
  clearOutput();
}

void loop()
{
  Serial.print("Stepper should be stationary");
  delay(1000);                  // wait 1s

  Serial.print("200 steps clockwise FAST");
  ReverseDirection = false;
  mystepper.setSpeed(motorSpeedFastRPM);
  for( int i = 0; i < 200; i++ )
  {
    clockwise();
    delay( motorSpeedDelay );   // required else stepper will not move
  }
  clearOutput();
  Serial.print("Move has finished. Motor stationary");
  delay(1000);                  // wait 1s
  
  Serial.print("200 steps anticlockwise FAST");
  ReverseDirection = false;
  mystepper.setSpeed(motorSpeedFastRPM);
  for( int i = 0; i < 200; i++ )
  {
    anticlockwise();
    delay( motorSpeedDelay );   // required else stepper will not move
  }
  clearOutput();
  Serial.print("Move has finished. Motor stationary");
  delay(1000);


  Serial.print("200 steps clockwise SLOW");
  ReverseDirection = false;
  mystepper.setSpeed(motorSpeedSlowRPM);
  for( int i = 0; i < 200; i++ )
  {
    clockwise();
    delay( motorSpeedDelay );   // required else stepper will not move
  }
  clearOutput();
  Serial.print("Move has finished. Motor stationary");
  delay(1000);                  // wait 1s
  
  Serial.print("200 steps anticlockwise SLOW");
  ReverseDirection = false;
  mystepper.setSpeed(motorSpeedSlowRPM);
  for( int i = 0; i < 200; i++ )
  {
    anticlockwise();
    delay( motorSpeedDelay );  // required else stepper will not move
  }
  clearOutput();
  Serial.print("Move has finished. Motor stationary");
  delay(1000);

  // Now lets test reverse direction flag
  Serial.print("100 steps clockwise SLOW with Reverse Direction False");
  ReverseDirection = false;
  mystepper.setSpeed(motorSpeedSlowRPM);
  for( int i = 0; i < 100; i++ )
  {
    clockwise();
    delay( motorSpeedDelay );   // required else stepper will not move
  }
  clearOutput();
  Serial.print("Move has finished. Motor stationary");
  delay(1000);                  // wait 1s
  
  Serial.print("100 steps clockwise SLOW with Reverse Direction True");
  ReverseDirection = true;
  mystepper.setSpeed(motorSpeedSlowRPM);
  for( int i = 0; i < 100; i++ )
  {
    anticlockwise();
    delay( motorSpeedDelay );   // required else stepper will not move
  }
  clearOutput();
  Serial.print("Move has finished. Motor stationary");
  delay(1000);
}


