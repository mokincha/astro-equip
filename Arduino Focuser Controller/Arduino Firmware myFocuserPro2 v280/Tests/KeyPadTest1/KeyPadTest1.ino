// TEST1 4x4 MEMBRANE KEYPAD PCF8274 I2C BOARD

#include <Arduino.h>
#include <Wire.h>                   // needed for I2C
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <myi2ckeypad.h>

#define ROWS 4
#define COLS 4
#define PCF8574_ADDR 0x20     // With A0, A1 and A2 of PCF8574 to ground I2C address is 0x20
i2ckeypad kpd = i2ckeypad(PCF8574_ADDR, ROWS, COLS);

// define serial port speed - valid values are 9600, 57600
#define SerialPortSpeed 9600

LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Setup
void setup()
{
  // initialize serial for ASCOM
  Serial.begin(SerialPortSpeed);

  // initialize keypad
  Wire.begin();
  kpd.init();

  Serial.print("Test1\n");
  lcd.begin(16, 2);
  lcd.setBacklight(HIGH);
  lcd.clear();
  lcd.print("Test1-PCF8574");
} // end of setup()

// Main Loop
void loop()
{
  // read keypad
  char key = kpd.get_key();

  if(key != '\0') 
  {
    Serial.print(key);
    lcd.setCursor(0, 1);
    lcd.print("  ");
    lcd.setCursor(0, 1);
    lcd.print(key);    
  }
  delay(100);
}

