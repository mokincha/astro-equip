// myfocuser OLED  Test Program
//
// (c) Copyright Robert Brown 2014-2017. All Rights Reserved.
// The schematic, code and ideas for myFocuser are released into the public domain. Users are free to implement these but
// may NOT sell this project or projects based on this project for commercial gain without express written permission
// granted from the author. Schematics, Code, Firmware, Ideas, Applications, Layout are protected by Copyright Law.
// Permission is NOT granted to any person to redistribute, market, manufacture or sell for commercial gain the myFocuser
// products, ideas, circuits, builds, variations and units described or discussed herein or on this site.
// Permission is granted for personal and Academic/Educational use only.

#include <Arduino.h>
#include <Wire.h>                   // needed for I2C, installed when installing the Arduino IDE
#include <mySSD1306Ascii.h>         // oled
#include <mySSD1306AsciiWire.h>     // oled

#define OLED_SDA A4                 // connected to SDA pin on OLED
#define OLED_SCL A5                 // connected to SCL pin on OLED, and connect OLED VCC pin to 5V and OLED GND pin to GND
// Connect OLED VCC pin to 5V and OLED GND pin to GND
#define I2C_ADDRESS 0x3C            // 0X3C+SA0 - 0x3C or 0x3D
SSD1306AsciiWire myoled;

// Setup
void setup()
{
  Serial.begin(96000);              // initialize serial port

  Wire.begin();                     // Setup the OLED
  myoled.begin(&Adafruit128x64, I2C_ADDRESS);
  myoled.set400kHz();
  myoled.setFont(Adafruit5x7);
  myoled.clear();                   // clrscr OLED
  myoled.Display_Normal();          // black on white
  myoled.Display_On();              // display ON
  myoled.Display_Rotate(0);         // portrait, not rotated
  myoled.Display_Bright();
}

// Main Loop
void loop()
{
  // The screen size is 128 x 64, so using characters at 6x8 this gives 21chars across and 8 lines down
  myoled.set1X();
  myoled.Display_Rotate(0);         // normal display
  myoled.println("Hello and Welcome");
  myoled.InverseCharOn();
  myoled.println("This is inversed text");
  myoled.InverseCharOff();
  myoled.setCursor(0, 4);
  myoled.set2X();
  myoled.print("Big Text");
  delay(2000);
  myoled.clear();
  myoled.Display_Rotate(1);         // rotate display
  myoled.setCursor(0, 0);
  myoled.print("Rotated");
  delay(2000);
  myoled.clear();
}


