
/*
 Stepper Motor Control - one step at a time

 The motor is attached to digital pins 5 - 8 of the Arduino.

 The motor will step one step at a time, very slowly.  You can use this to
 test that you've got the four wires of your stepper wired to the correct
 pins. If wired correctly, all steps should be in the same direction.

 Use this also to count the number of steps per revolution of your motor,
 if you don't know it.  Then plug that number into the oneRevolution
 example to see if you got it right.

 Created 30 Nov. 2009
 by Tom Igoe
 Modified by RB Brown May 2015
 */

#include <myStepper.h>

const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution
// for your motor

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 5, 6, 7, 8);

int stepCount = 0;         // number of steps the motor has taken

#define anticlockwise 1
#define clockwise 0
int direction = clockwise;

void setup() {
  // initialize the serial port:
  Serial.begin(9600);
  direction = clockwise;
}

void loop() {

  if( direction == clockwise )
  {  
    // step one step:
    myStepper.step(1);
  }
  else
  {
    // step one step:
    myStepper.step(-1);
  }
  Serial.print("steps:" );
  Serial.println(stepCount);
  stepCount++;
  delay(500);
  // go 200 steps then reverse
  if ( stepCount > 200 )
  {
    stepCount = 0;
    if ( direction == clockwise )
      direction = anticlockwise;
    else
      direction = clockwise;
    }
}


