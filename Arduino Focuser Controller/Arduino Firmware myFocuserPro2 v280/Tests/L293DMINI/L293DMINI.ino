// TEST_L293DMINI
// (c) R Brown 2017
// All rights reserved

// Test code for myFocuserPro with L293D Mini driver board

#include <Arduino.h>
#include <myStepper.h>

// Stepper Motor stuff, control pins for L293D Mini driver board
#define IN1     3  // input lines for stepping
#define IN2     4
#define IN3     5
#define IN4     6

const int stepsPerRevolution = 200;     // NEMA17 motor


// initialize the stepper library
Stepper mystepper(stepsPerRevolution, IN1, IN2, IN3, IN4);

const int    motorSpeedSlowRPM = 10;
const int    motorSpeedMedRPM  = 30;
const int    motorSpeedFastRPM = 50;
// motorspeeddelay is a fixed value
int motorSpeedDelay = 5;                // the delay in millseconds between steps
int motorSpeedRPM = motorSpeedSlowRPM;  // the motorspeed RPM setting
int motorSpeed = 0;                     // 0=slow, 1=medium, 2=fast, default=slow on startup only
int savedmotorSpeed = motorSpeed;       // used to save original speed if slowing down when nearing target position
int stepontime = 5;                     // minimum step pulse time

// define serial port speed - valid values are 9600, 57600
#define SerialPortSpeed 9600

boolean ReverseDirection;

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
  pinMode( IN1, OUTPUT);
  pinMode( IN2, OUTPUT);
  pinMode( IN3, OUTPUT);
  pinMode( IN4, OUTPUT);
  // set low so motor does not move
  digitalWrite( IN1, false);
  digitalWrite( IN2, false);
  digitalWrite( IN3, false);
  digitalWrite( IN4, false);
  
  ReverseDirection = false;

  mystepper.setSpeed(motorSpeedRPM);
  clearOutput();
}

void loop()
{
  Serial.println("Stepper should be stationary");
  delay(5000);      // wait 5s

  Serial.println("200 steps clockwise FAST");
  ReverseDirection = false;
  mystepper.setSpeed(motorSpeedFastRPM);
  for( int i = 0; i < 200; i++ )
  {
    clockwise();
    delay( motorSpeedDelay );  // required else stepper will not move
  }
  clearOutput();
  Serial.println("Move has finished. Motor stationary");
  delay(5000);      // wait 5s
  
  Serial.println("200 steps anticlockwise FAST");
  ReverseDirection = false;
  mystepper.setSpeed(motorSpeedFastRPM);
  for( int i = 0; i < 200; i++ )
  {
    anticlockwise();
    delay( motorSpeedDelay );  // required else stepper will not move
  }
  clearOutput();
  Serial.println("Move has finished. Motor stationary");
  delay(5000);


  Serial.println("200 steps clockwise SLOW");
  ReverseDirection = false;
  mystepper.setSpeed(motorSpeedSlowRPM);
  for( int i = 0; i < 200; i++ )
  {
    clockwise();
    delay( motorSpeedDelay );  // required else stepper will not move
  }
  clearOutput();
  Serial.println("Move has finished. Motor stationary");
  delay(5000);      // wait 5s
  
  Serial.println("200 steps anticlockwise SLOW");
  ReverseDirection = false;
  mystepper.setSpeed(motorSpeedSlowRPM);
  for( int i = 0; i < 200; i++ )
  {
    anticlockwise();
    delay( motorSpeedDelay );  // required else stepper will not move
  }
  clearOutput();
  Serial.println("Move has finished. Motor stationary");
  delay(5000);

  // Now lets test reverse direction flag
  Serial.println("100 steps clockwise SLOW with Reverse Direction False");
  ReverseDirection = false;
  mystepper.setSpeed(motorSpeedSlowRPM);
  for( int i = 0; i < 100; i++ )
  {
    clockwise();
    delay( motorSpeedDelay );  // required else stepper will not move
  }
  clearOutput();
  Serial.println("Move has finished. Motor stationary");
  delay(5000);      // wait 5s
  
  Serial.println("100 steps clockwise SLOW with Reverse Direction True");
  ReverseDirection = true;
  mystepper.setSpeed(motorSpeedSlowRPM);
  for( int i = 0; i < 100; i++ )
  {
    anticlockwise();
    delay( motorSpeedDelay );  // required else stepper will not move
  }
  clearOutput();
  Serial.println("Move has finished. Motor stationary");
  delay(5000);
}


