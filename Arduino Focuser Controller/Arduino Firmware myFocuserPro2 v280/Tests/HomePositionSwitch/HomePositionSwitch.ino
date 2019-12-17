// TestHomePositionSwitch
//
// (c) Copyright Robert Brown 2014-2016 All Rights Reserved.
// Permission is granted for personal and Academic/Educational use only.

// Test Home Position Switch v1, switch connected to D12

#include <Arduino.h>
#include <Bounce2.h>                // needed to debounce Home Position switch, see https://github.com/thomasfredericks/Bounce2

// define serial port speed - valid values are 9600, 57600
#define SerialPortSpeed 9600

// Home Position switch is on D12
#define HPSWpin 12
bool hpswactivehigh;                    // user must specify high or low to signal a home position close
Bounce hpswbounce = Bounce();           // setup debouncer for hp switch
int hpswstate;

// Setup
void setup()
{
  // initialize serial for ASCOM
  Serial.begin(SerialPortSpeed);

  // define what state we expect on Home Position switch when closed
  // we expect a GND signal as its kept high by the 470K resistor
  // so when we get a GND reading the switch has been activated
  hpswactivehigh = false;

  // set up the Home Position Switch pin as an input
  pinMode( HPSWpin, INPUT_PULLUP);

  // setup defaults for debouncing hp Switch
  hpswbounce.attach( HPSWpin );
  hpswbounce.interval( 5 );        // sets debounce time
}

// Main Loop
void loop()
{
    // check if focuser has hit Mechanical Home switch
    hpswbounce.update();            // we need to call update to read home position switch state, no interrupts are used
    hpswstate = hpswbounce.read();  // reads the home position switch state
    if ( hpswstate == 0 )           // if its closed then logic level will be low, ie GND
    {
      // targetPosition = 0;
      Serial.println("Home switch is activated");
    }
    else              
    {
      Serial.println("Waiting for Home Position Switch to be activated");
    }
}
