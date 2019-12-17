#include <Arduino.h>
#include <myQuadrature.h>             // needed for rotary encoder

// define Rotary Encoder Pins
Quadrature myrotaryencoder(9, 10);  //  Rotary encoder is connected to pins 9 and 10
#define encoderSWPin 11             // encoder push sw on pin 11

long pos = 0L;

// Setup
void setup() {
  // initialize serial for ASCOM
  Serial.begin(9600);

  // setup defaults for rotary encoder
  myrotaryencoder.minimum(-1000);
  myrotaryencoder.maximum(1000); 
  pinMode( encoderSWPin, INPUT_PULLUP );
}

// Main Loop
void loop() {
  // check rotary encoder
  // do not update if focuser is already moving - just wait
    int lp;
    lp = myrotaryencoder.getposition();
    if( lp != 0 ) {
      // adjust the target position
      Serial.print("Encoder position = ");
      Serial.println(lp);
      
      pos = pos + lp;
      Serial.print("Position = ");
      Serial.println(pos);
      myrotaryencoder.setposition(0);    // reset to avoid 
    }
  
  // check rotary encoder push switch
  // if pushed then halt and set to 0
  if( !digitalRead(encoderSWPin) ){
    delay(1);     // software debounce to prevent false triggers
    if( !digitalRead(encoderSWPin) ) {
      // still pressed
      Serial.println("SW Pressed");
    
      // wait for PB to be released
      while( !digitalRead(encoderSWPin) )
        ;
      
      Serial.print("Released");   // debug message only, do not uncomment
    }
  }
}
