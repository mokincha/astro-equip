// Test IR (Infra-Red Remote) PROGRAM
// Progam uses serial monitor to display program output - IR Codes
// For use with HX1828 NEC REMOTE CONTROLLER

// The files IRemote1.cpp, IRemote.h and IRemoteInt1.h must be in the same folder as the firmware ino file
// Do not replace the library files for myIRRemote wuth those from the IRemote library - these files have been modified!!!!!!!
/
// (c) Copyright Robert Brown 2014-2016. All Rights Reserved.
// The schematic, code and ideas for myFocuser are released into the public domain. Users are free to implement these but
// may NOT sell this project or projects based on this project for commercial gain without express written permission
// granted from the author. Schematics, Code, Firmware, Ideas, Applications, Layout are protected by Copyright Law.
// Permission is NOT granted to any person to redistribute, market, manufacture or sell for commercial gain the myFocuser
// products, ideas, circuits, builds, variations and units described or discussed herein or on this site.
// Permission is granted for personal and Academic/Educational use only.

// v1.01 24102016

#include <Arduino.h>
#include <myIRremote1.h>

// define serial port speed
#define SerialPortSpeed 9600

// define IR Pin
int RECV_PIN = 9;      // must be connected to pin 9
IRrecv irrecv(RECV_PIN);
decode_results results;

// Setup
void setup()
{
  // initialize serial port
  Serial.begin(SerialPortSpeed);

  // setup defaults for IR
  irrecv.enableIRIn(); // Start the receiver
}

// Main Loop
void loop()
{
  // check IR
  long ircode ;

  if (irrecv.decode(&results))
  {
    static long lastcode;
    Serial.print("Receive code=");
    Serial.println(lastcode);
    if ( results.value == 4294967295 )
    {
      // repeat last code
      results.value = lastcode;
    }
    else
    {
      lastcode = results.value;
    }
    irrecv.resume(); // Receive the next value
  }
}


