// L298N Basic Stepper Motor Test
// ONLY FOR USE WITH L298N Driver Board - supports 2A at 12VDC - FULL STEPS ONLY
// (c) Copyright Robert Brown 2014-2016. All Rights Reserved.

#include <Arduino.h>
#include <myStepper.h>

// define serial port speed - valid values are 9600, 57600
#define SerialPortSpeed 9600

// stepper motor steps per full revolution using full steps
const int stepsPerRevolution = 200;        // NEMA17 change this to fit the number of steps per revolution

// Stepper Motor stuff, control pins for L298N board
#define IN1     4  // input lines for stepping
#define IN2     5
#define IN3     6
#define IN4     7

// initialize the stepper library on pins 4 through 7:
Stepper mystepper(stepsPerRevolution, IN1, IN2, IN3, IN4);

const int    motorSpeedSlowRPM = 5;
const int    motorSpeedMedRPM  = 15;
const int    motorSpeedFastRPM = 25;
// motorspeeddelay is a fixed value
int motorSpeedDelay = 5;                // the delay in millseconds between steps
int stepontime = 5;

// Move stepper anticlockwise
void anticlockwise()
{
  mystepper.step(1);
}

// Move stepper clockwise
void clockwise()
{
  mystepper.step(-1);
}

// Setup
void setup()
{
  // initialize serial port
  Serial.begin(SerialPortSpeed);

  // set the speed to default 30rpm:
  mystepper.setSpeed(motorSpeedDelay);

  // setup INx step lines as outputs, set low so motor does not move
  pinMode( IN1, OUTPUT );
  pinMode( IN2, OUTPUT );
  pinMode( IN3, OUTPUT );
  pinMode( IN4, OUTPUT );
  digitalWrite( IN1, false);
  digitalWrite( IN2, false);
  digitalWrite( IN3, false);
  digitalWrite( IN4, false);

  // set speed setting
  mystepper.setSpeed(motorSpeedSlowRPM);
}

// Main Loop
void loop()
{
  int stepperpos;
  Serial.println("Test starting now");
  Serial.println("Speed = slow");
  Serial.println("Direction = clockwise");
  Serial.println("Moving 400 steps");
  mystepper.setSpeed(motorSpeedSlowRPM);
  for (stepperpos = 0; stepperpos < 400; stepperpos++ )
    clockwise();

  delay(2000);
  Serial.println("Speed = fast");
  Serial.println("Direction = clockwise");
  Serial.println("Moving 400 steps");
  mystepper.setSpeed(motorSpeedFastRPM);
  for (stepperpos = 0; stepperpos < 400; stepperpos++ )
    clockwise();

  delay(2000);
  Serial.println("Speed = fast");
  Serial.println("Direction = anticlockwise");
  Serial.println("Moving 400 steps");
  mystepper.setSpeed(motorSpeedFastRPM);
  for (stepperpos = 0; stepperpos < 400; stepperpos++ )
    anticlockwise();

  delay(2000);
  Serial.println("Speed = medium");
  Serial.println("Direction = anticlockwise");
  Serial.println("Moving 400 steps");
  mystepper.setSpeed(motorSpeedMedRPM);
  for (stepperpos = 0; stepperpos < 400; stepperpos++ )
    anticlockwise();
  
  delay(2000);

}
