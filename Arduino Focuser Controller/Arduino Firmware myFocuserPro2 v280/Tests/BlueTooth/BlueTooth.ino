// BLUETOOTH
//
// (c) Copyright Robert Brown 2014-2017. All Rights Reserved.
// The schematic, code and ideas for myFocuser are released into the public domain. Users are free to implement these but
// may NOT sell this project or projects based on this project for commercial gain without express written permission
// granted from the author. Schematics, Code, Firmware, Ideas, Applications, Layout are protected by Copyright Law.
// Permission is NOT granted to any person to redistribute, market, manufacture or sell for commercial gain the myFocuser
// products, ideas, circuits, builds, variations and units described or discussed herein or on this site.
// Permission is granted for personal and Academic/Educational use only.

// BlueTooth HC-05 Module wired as follows
// EN/CMD       // do not connect
// RX to middle of 2K and 1K resistor - other end of 2K resistor to D11 and other end of 1K resistor to ground
// TX to D10
// STATE        // do not connect


#include <Arduino.h>
#include <SoftwareSerial.h>         // needed for bt adapter - this library is already included when you install the Arduino IDE

// define serial port speed - valid values are 9600, 57600
#define SerialPortSpeed 9600
#define BTPortSpeed 9600

#define btRX    11
#define btTX    10
// define Bluetooth Adapter
SoftwareSerial btSerial( btTX, btRX);

char btinChar;
char inChar;

// Setup
void setup()
{
  // initialize serial port
  Serial.begin(SerialPortSpeed);

  // start bt adapter
  btSerial.begin(BTPortSpeed);

  clearSerialPort();
  clearbtPort();
}

// Main Loop
void loop()
{
  // anything recieved on BT will then be sent on Serial Port
  // anything recieved on Serial Port will tranmist on BT

  if (btSerial.available() )
  {
    btinChar = (char) btSerial.read();
    Serial.print( btinChar );
  }

  if( Serial.available() )
  {
    inChar = (char) Serial.read();
    btSerial.print( inChar );
  }
}

void clearSerialPort()
{
  while (Serial.available())
  {
    inChar = Serial.read();
  }
}

void clearbtPort()
{
  while (btSerial.available())
  {
    btinChar = btSerial.read();
  }
}


