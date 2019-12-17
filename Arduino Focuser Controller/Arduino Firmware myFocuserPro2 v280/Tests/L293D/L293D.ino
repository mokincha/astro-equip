// basic test program for L293D MotorShield + Uno
// rotates motor one full rotation then the reverse direction with 2s delay between moves

#include <Arduino.h>
#include <myAFMotor.h>                // needed for stepper motor and L293D shield

// Stepper Motor stuff - YOU NEED TO USE THE CORRECT ONES FOR YOUR STEPPER MOTOR
// Motor port on the L293D shield to use
#define Motor_Port 2      // use M3 and M4 as its easier to connect
// you need to change the above line to reflect which port you are using on the L293D shield
// it is either 1 (M2/M1) or 2 (M3/M4)

const int stepsPerRevolution = 1036;       // NEMA17-PG5 motor
// you need to change the above line to reflect your stepper motor, examples below
// const int stepsPerRevolution = 2048;    // 24BBYJ-48 motor, if half stepping multiply by 2
// const int stepsPerRevolution = 1036;    // NEMA17-PG5 motor, if half stepping multiply by 2
// const int stepsPerRevolution = 200;     // NEMA17 motor, if half stepping multiply by 2
// const int stepsPerRevolution = 5370;    // NEMA17-PG25 motor, if half stepping multiply by 2

// motor speeds in RPM - you need to adjust these depending on the stepper motor you select
int motorSpeedSlow = 1;
int motorSpeedMed = 10;
int motorSpeedFast = 30;
int motorSpeed = motorSpeedMed;
int savedmotorSpeed;    // used with home position switch

AF_Stepper mystepper(stepsPerRevolution, Motor_Port);

// disable the stepper motor outputs - coil power off
void clearOutput()
{
  // check to see what is selected, keep or release
    mystepper.release();
}

// Move stepper anticlockwise
void anticlockwise()
{
  {
    // (!ReverseDirection)? analogWrite( gledOUT, 255) : analogWrite( bledIN, 255);
    // mystepper.step(1, (!ReverseDirection)? BACKWARD : FORWARD, (halfstep) ? INTERLEAVE : DOUBLE );
    // (!ReverseDirection)? analogWrite( gledOUT, 0 ): analogWrite( bledIN, 0);
      mystepper.step(1, BACKWARD, INTERLEAVE); // one half step anticlockwise
  }
}

// Move stepper clockwise
void clockwise()
{
  // (!ReverseDirection)? analogWrite( bledIN, 255) : analogWrite( gledOUT, 255);
  // mystepper.step(1, (!ReverseDirection)? BACKWARD : FORWARD, (halfstep) ? INTERLEAVE : DOUBLE );
  // (!ReverseDirection)? analogWrite( bledIN, 0 ) : analogWrite( gledOUT, 0);
  {
      mystepper.step(1, FORWARD, INTERLEAVE);   // one half step clockwise
  }
}

// Setup
void setup() {
  // initialize serial for ASCOM
  Serial.begin(9600);

  mystepper.setSpeed(motorSpeed);

    mystepper.release();
}

// Main Loop
void loop() {
  Serial.println("Clockwise 200 steps - Full steps");
  for ( int lp = 0; lp < 200; lp++ )
  {
    clockwise();
    delay(10);
  }
  Serial.println("Wait 2s");
  delay(2000);
  Serial.println("Anti-Clockwise 200 steps - Full steps");
  for ( int lp = 0; lp < 200; lp++ )
  {
    anticlockwise();
    delay(10);
  }
  Serial.println("Wait 2s");
  delay(2000);
  Serial.println("Clockwise 200 steps - Half steps");
  for ( int lp = 0; lp < 200; lp++ )
  {
    clockwise();
    delay(10);
  }
  Serial.println("Wait 2s");
  delay(2000);
  Serial.println("Anti-Clockwise 200 steps - Half steps");
  for ( int lp = 0; lp < 200; lp++ )
  {
    anticlockwise();
    delay(10);
  }
  Serial.println("Wait 2s");
  delay(2000);
}


