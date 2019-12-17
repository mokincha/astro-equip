// TestLCDI2C
//
// (c) Copyright Robert Brown 2014-2017 All Rights Reserved.
// Permission is granted for personal and Academic/Educational use only.

// Test LCD Display

#include <Arduino.h>
#include <Wire.h>                   // needed for I2C
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Setup
void setup()
{
  lcd.begin(16, 2);
  lcd.setBacklight(HIGH);
}

// Main Loop
void loop()
{
  lcd.clear();
  lcd.print("Hello");
  delay(2000);
  lcd.setCursor(0,0);
  lcd.print("0123456789ABCDEF");
  lcd.setCursor(0,1);
  lcd.print("GHIJKLMNOPQRSTUV");
  delay(2000);
  int pos = 0;
  while( pos < 16 )
  {
     lcd.scrollDisplayLeft();
     delay(500);
     pos = pos + 1; 
  }
  delay(1000);
}
