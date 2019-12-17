// basic test program for ULN2003
// rotates motor one full rotation then the reverse direction with 2s delay between moves

#include <Arduino.h>
#include <myStepper.h>                  // needed for stepper motor

// initialize the stepper library on pins 4 (IN1), 5 (IN2), 6 (IN3), 7 (IN4)
Stepper mystepper(2048, 4, 6, 7, 5);

// Setup
void setup()
{
  Serial.begin(9600);
  mystepper.setSpeed(2); // medium speed
  Serial.println("Starting test");
}

// Main Loop
void loop() 
{
  Serial.println("Stepping motor one full direction");
  mystepper.step(2048);        // step the motor one step anticlockwise
  Serial.println("Motor stopped");
  delay(2000);
  Serial.println("Stepping motor one full direction in opposite direction");
  mystepper.step(-2048);        // step the motor one step anticlockwise
  Serial.println("Motor stopped");
  delay(2000);
}
