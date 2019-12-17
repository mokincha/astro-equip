// TestPB
//
// (c) Copyright Robert Brown 2014-2016 All Rights Reserved.
// Permission is granted for personal and Academic/Educational use only.

// Test Push Buttons

#include <Arduino.h>

// define serial port speed - valid values are 9600, 57600
#define SerialPortSpeed 9600

//#define DEBUG 1

// define Push Buttons, use voltage divider network for two push button switches using A0, use software debouncing
#define PBswitchesPin  A0   // push button switches wired to A0 via resistor divider network
int PBVal = 0;              // holds state of pushbutton switches

// read the push button switches and return state of switches
// 1 = SW1 ON AND SW2 OFF, 2 = SW2 ON AND SW1 OFF, 3 = SW1 ON and SW2 ON, 0 = OFF
int readpbswitches(int pinNum)
{
  // for 1.2K sw1 (681) 650-720, sw2 (338) 310-380, sw1 and sw2 (509) 460-530
  // for 1K sw1 (630) 600-660, sw2 (335) 300-370, sw1 and sw2 (510) 480-540
  int retval = 0;
  int val = 0;                            // variable to store the read value
  digitalWrite(pinNum, HIGH);
  val = analogRead(pinNum);               // read the input pin
  // push button pin A0 is held low by 1M ohm resistor.
  // Internal pull-ups disabled
  // quick return if no keys are pressed
  Serial.print("Raw button value = "); 
  Serial.println(val);
  if ( val < 310 )
    retval = 0;
  else if (val > 720 )
    retval = 0;
  else if ( val >= 650 && val <= 720 )
  {
    retval = 1;                             // toggle sw1 ON and SW2 OFF
  }
  else if ( val >= 460 && val <= 530 )
  {
    retval = 3;                             // toggle sw1 and sw2 ON
  }
  else if ( val >= 310 && val <= 380 )
  {
    retval = 2;                             // toggle sw2 ON and SW1 OFF
  }
  else retval = 0;                          // switches are OFF
  //Serial.print("Retval = ");
  //Serial.println(retval);
  return retval;
}

// Setup
void setup()
{
  Serial.begin(SerialPortSpeed);
  Serial.println("Starting pushbutton test");
}

// Main Loop
void loop()
{
  // SPECIAL NOTE START
  // check pushbutton switches
  // if you are not using the Push Button switches this part of code MUST BE DELETED
#ifdef DEBUG
  Serial.println();
  Serial.println("-----------------------------------------");
  Serial.print("PBswitchesPin=");
  Serial.println(PBswitchesPin);
  Serial.println("-----------------------------------------");
#endif  
  PBVal = readpbswitches(PBswitchesPin);
  if ( PBVal != 0)
  {
    // if its a genuine button press
    delay(20);
    PBVal = readpbswitches(PBswitchesPin);
    Serial.print("PBVal = ");
    Serial.println(PBVal);
    // now check the pbval using a switch for 1 2 and 3
    switch ( PBVal )
    {
      case 1:                        // toggle sw1 is ON and 2 is off
        Serial.println("Button1 is pressed");
        break;
      case 2:                        // toggle sw2 is ON and SW1 is OFF
        Serial.println("Button2 is pressed");
        break;
      case 3:                        // toggle sw1 and sw2 are ON
        Serial.println("Button1 AND Button2 is pressed");
        break;
      default:
        Serial.println("No button is pressed");
        break;
    } // end of switch
  } // end of pb test
  // SPECIAL NOTE END
  delay(100);
}
