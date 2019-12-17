// TEST2 4x4 MEMBRANE KEYPAD PCF8274 I2C BOARD

#include <Arduino.h>
#include <Wire.h>                   // needed for I2C
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <myi2ckeypad.h>       // courtesy by Angelo Santo

#define ROWS 4
#define COLS 4
#define PCF8574_ADDR 0x20     // With A0, A1 and A2 of PCF8574 to ground I2C address is 0x20
i2ckeypad kpd = i2ckeypad(PCF8574_ADDR, ROWS, COLS);
#define lenkeypadstr 6
char  keypadstr[lenkeypadstr];
const char NO_KEY = '\0';
char  keyval;

// define serial port speed - valid values are 9600, 57600
#define SerialPortSpeed 9600

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void getkeypadstr()
{
  keyval = NO_KEY;
  memset( keypadstr, 0, lenkeypadstr );
  int index = 0;

  while ( keyval != '#' )
  {
    keyval = kpd.get_key();
    if (keyval != NO_KEY)
    {
      switch (keyval)
      {
        case '*':     // clear string
          index = 0;
          memset( keypadstr, 0, lenkeypadstr );
          lcd.clear();
          lcd.print("Enter Numeric Value");
          Serial.print("Enter Numeric Value");
          break;
        case '#':     // end of data input
          break;
        case 'A' :    // ignore
        case 'B' :
        case 'C' :
        case 'D' :
          break;
        default:
          // add number to string
          keypadstr[index] = keyval;
          // print on second line of LCD
          lcd.setCursor(0, 1);
          lcd.print( keypadstr );
          Serial.println( keypadstr );
          // ensure that buffer is not exceeded
          if (index == lenkeypadstr - 1)
          {
            index = lenkeypadstr - 1;
          }
          else
            index++;
      } // end of switch
      delay(100);
    }   // end of if
  }     // end of while
}

// convert string to long
long decstr2long(char *line)
{
  long ret = 0;

  ret = strtol(line, NULL, 10);
  return (ret);
} // end of decstr2long()

// Setup
void setup()
{
  // initialize serial
  Serial.begin(SerialPortSpeed);

  // initialize keypad
  Wire.begin();
  kpd.init();

  Serial.print("Test2\n");
  lcd.begin(16, 2);
  lcd.setBacklight(HIGH);
  lcd.clear();
  lcd.print("Test2-PCF8574");
} // end of setup()

// Main Loop
void loop()
{
  lcd.clear();
  lcd.print("Enter number (end #)");
  Serial.print("Enter number (end #)");  
  getkeypadstr();
  lcd.clear();
  lcd.print("typed string was");
  lcd.println(keypadstr);
  Serial.print("typed string was ");
  Serial.println(keypadstr);
  delay(3000);
  long number;
  number = decstr2long( keypadstr );
  lcd.clear();
  lcd.print("Number is ");
  lcd.println(number);
  Serial.print("Number is ");
  Serial.println(number);
  delay(3000);
}

