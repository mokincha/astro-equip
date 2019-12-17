// myfocuser Clear EEPROM
// This program wipes the entire EEPROM of the controller
// Use this program to clear all the EEPROM if you encounter problems with values in EEPROM not setting properly
// Avoid running this program on a regular basis - only designed to be used sparingly

#include <Arduino.h>
#include <EEPROM.h>                 // needed for EEPROM

// define serial port speed - valid values are 9600, 57600
#define SerialPortSpeed 9600

// Setup
void setup()
{
  // initialize serial for ASCOM
  Serial.begin(SerialPortSpeed);
  
  Serial.println("This program clears all EEPROM positions by writing 0 to each location");
  unsigned int lp;
  int val = 0;
  Serial.print("The size of the EEPROM in bytes is ");
  Serial.println( EEPROM.length() );
  Serial.println("Starting now: Please wait....");
  for( lp = 0; lp < EEPROM.length(); lp++ )
  {
    EEPROM.write(lp, val);    // update values in EEPROM
  }
  Serial.println("DONE");
  Serial.println();
  Serial.println("Now reprogram the controller with the latest firmware");
}

// Main Loop
void loop()
{


}
