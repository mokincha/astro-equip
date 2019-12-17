// basic test program for DRV8825 and HW203 Build
// rotates motor one full rotation then the reverse direction with 2s delay between moves

#include <Arduino.h>

// Stepper Motor stuff, control pins for DRV8825 board
#define myDir     3
#define myStep    4
#define myM2      5  // microstepping lines
#define myM1      6
#define myM0      7
// m0/m1/m2 sets stepping mode 000 = F, 100 = 1/2, 010 = 1/4, 110 = 1/8, 001 = 1/16, 101 = 1/32
// steps per revolution = 200, 400, 800, 1600, 6400
// must set the current limiting for microstepping to work correctly

// ENABLE - Setting CoilPwr controls the disable or enable state of the EASYDRIVER board outputs
int boardstate;
#define OUTPUTENABLED 1
#define OUTPUTDISABLED 0
#define myEnable  8        // For DRV8825 and Easydriver boards, wire /ENABLE pin to D8 input
// NOTE: If using microstepping, coil power should always be ON and do NOT disable the board

// stepontime - time in microseconds that coil power is ON for one step, board requires 2us pulse
int stepontime = 5;
// motorSpeed - time in milliseconds of delay between stepper pulses
int motorSpeedSlow = 15;
int motorSpeedMed = 8;
int motorSpeedFast = 3;
int motorSpeed = motorSpeedMed;

// enable the stepper motor outputs - coil power on
void enableoutput() {
  digitalWrite(myEnable, LOW);
}

// Move stepper anticlockwise
void anticlockwise() {
    digitalWrite(myDir, LOW );
    digitalWrite(myStep, 1 );
    delayMicroseconds(stepontime);
    digitalWrite(myStep, 0 );
}

// Move stepper clockwise
void clockwise() {
    digitalWrite(myDir, HIGH );
    digitalWrite(myStep, 1 );
    delayMicroseconds(stepontime);
    digitalWrite(myStep, 0 );
}

// set the microstepping mode
void setstepmode( int stepmode ) {
  switch ( stepmode ) {
    case 1:      // full step
      digitalWrite(myM0, 0);
      digitalWrite(myM1, 0);
      digitalWrite(myM2, 0);
      break;
    case 2:      // half step
      digitalWrite(myM0, 1);
      digitalWrite(myM1, 0);
      digitalWrite(myM2, 0);
      break;
    case 4:
      digitalWrite(myM0, 0);
      digitalWrite(myM1, 1);
      digitalWrite(myM2, 0);
      break;
    case 8:
      digitalWrite(myM0, 1);
      digitalWrite(myM1, 1);
      digitalWrite(myM2, 0);
      break;
    case 16:
      digitalWrite(myM0, 0);
      digitalWrite(myM1, 0);
      digitalWrite(myM2, 1);
      break;
    case 32:
      digitalWrite(myM0, 1);
      digitalWrite(myM1, 0);
      digitalWrite(myM2, 1);
      break;
  }
}

// Setup
void setup() {
  // initialize serial for ASCOM
  Serial.begin(9600);

  pinMode(  myDir, OUTPUT );
  pinMode(  myStep, OUTPUT );
  pinMode(  myM0, OUTPUT );
  pinMode(  myM1, OUTPUT );
  pinMode(  myM2, OUTPUT );

  // set direction and step to low
  digitalWrite( myDir, 0 );
  digitalWrite( myStep, 0 );

  boardstate = OUTPUTENABLED;
  pinMode( myEnable, OUTPUT );    // enable the driver board
  enableoutput();
}

// Main Loop
void loop() {
  setstepmode(1);    // enable full steps
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
  setstepmode(2);     // enable half-steps
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


