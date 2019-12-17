// myfocuserpro2 L293DMINI TFT 2.2" FULL
// ONLY FOR USE L293DMINI TFT ONLY - supports 1.5A at 12VDC and microstepping
// BIPOLAR STEPPER MOTORS ONLY
// PUSH BUTTONS, BUZZER, LEDS, TEMPERATURE PROBE, TFT 2.2" DISPLAY
// USES NEW LAYOUT TFTV2 SPI
//
// NO HOMEPOSITION SWITCH (not supported)
//
// ONLY TO BE USED WITH TFT 2.2" DISPLAY - ILI9341
// TFT2.2 ILI9341 320x240

// (c) Copyright Robert Brown 2014-2019. All Rights Reserved.

// YOU MAY NOT SELL CONTROLLERS OR PCB'S BASED ON THIS PROJECT
// for commercial gain without express written permission granted from the author.
// Schematics, Code, Firmware, Ideas, Applications, Layout are protected by International Copyright Law.

// Permission is NOT granted to any person to redistribute, market, manufacture or sell for commercial gain the myFocuser
// products, ideas, PCB's, circuits, builds, variations and units described or discussed herein or on this site.
// Permission is granted for personal and Academic/Educational use only.

// THIS MEANS THAT YOU CANNOT RE-DESIGN AND MAKE AVAILABLE KITS OR PCB BASED ON THIS PROJECT AND
// OFFER THOSE KITS FOR SALE TO OTHERS. THAT IS A BREACH OF COPYRIGHT.

// CONTRIBUTIONS
// If you wish to make a small contribution in thanks for this project, please use PayPal and send the amount
// to user rbb1brown@gmail.com (Robert Brown). All contributions are gratefully accepted.

// ----------------------------------------------------------------------------------------------------------
// PCB BOARDS
// Stripboard/Layout
// my_focuser_L293DMINI_OLED_F_schem
// myFocuser_L293DMINI_OLED_Layout_F

// ----------------------------------------------------------------------------------------------------------
// HARDWARE MAPPINGS

// Mapping for PushButtons
// A0

// these are on the header of the ILI9341 tft
// 2.2-SPI 320x240 TFT MODULE
// VCC=5V
// GND=GND
// HEADER FTF                    Arduino
// Pin1  VCC       3.3-5V        5V
// Pin2  GND                     GND
// Pin3  CS        3.3-5V        5
// Pin4  RESET     3.3-5V        4
// Pin5  DC        3.3-5V        6
// Pin6  SDI/MOSI  3.3-5V        11 MOSI
// Pin7  SCK       3.3V-5V       13 SCLK
// Pin8  LED/BL    3.3-5V        7
// Pin9  SDO/MISO  -   -         FOR SD CARD

// ----------------------------------------------------------------------------------------------------------
// DEFINABLE FEATURES
// Caution: Do not enable a feature if you have not added the associated hardware circuits to support that feature
// With this version you can enable or disable specific add-ons and build the controller with the options you want
// using a single firmware file
// By default, this is a MINIMAL Controller firmware

// To enable temperature probe, uncomment next line
//#define TEMPERATUREPROBE 1

// To enable the Push Buttons for manual focusing, uncomment the next line
//#define PUSHBUTTONS 1

// To enable the TFT DISPLAY uncomment the next line
//#define TFTDISPLAY 1

// To enable backlash in this firmware, uncomment the next line
//#define BACKLASH 1

// To enable the buzzer, uncomment the next line
#define BUZZER 1

// To enable the IN-OUT LEDS, uncomment the next line
#define INOUTLEDS 1

// To enable support when usng a Micro instead of a Nano, uncomment the next line
//#define CONTROLLERISAMICRO 1           // provided by IL, enables the serialEventRun function for a Micro

// do not change
//#define DEBUG     1
//#define DEBUGHPSW 1
// ----------------------------------------------------------------------------------------------------------
// FIRMWARE CHANGES

// 2.80
// Fixes

// 2.79
// Controller remembers motorspeed
// Please run the ClearEEPROM.ino file on the controller before programming with v279
// Note that running the ClearEEPROM will erase all focuser settings

// 2.78
// Major update - please use with Windows App 2.4.1.9 or higher

// 2.77
// Fixes for speeds s/m/f
// Fix for delay after move

// 2.76
// Fix bug in tempmode C/F

// 2.75
// Add call to get temperature mode setting, Celsius or Fahrenheight
// Add call to get if there is a temperature probe
// Requires WinApp 2.4.1.1 or higher

// 2.74
// Revert back to move restrictions when in TC mode enabled

// 2.73
// test more changes to Push Button code

// 2.72
// Another change in PB code
// Remove move restrictions when in TC mode enabled

// 2.71
// Small change in push button code

// 2.70
// Combine Move and Home code together, bug fix for backlash issue when Home, and Home followed by Move

// 2.69
// Changes related to compatibility with recent updates to specific client apps

// 2.68
// Fix backlash issue

// v2.67
// Rewrite temp routines to remove wait delay
// Remove backlash maxsteps
// changes to push button code
// eeprom updates consolidated into a method
// remove eeprom writes and jogging speed commands
// changes to jogging, infrared and rotary encoder

// v2.66
// Rewrite using defines to make more configurable
// Add checks to prevent home position switch never ending if not detected
// Reformat display output, add LCD1604, display types now LCD1602, LCD1604 and LCD2004
// Allow focuser moves when temperature compensation is enabled to conform to recent ASCOM standard changes

// For all other previous versions please see the file myFocuserPro2 Firmware Changes Readme.txt

// ----------------------------------------------------------------------------------------------------------
// FIRMWARE CODE START

#include <Arduino.h>
#include <myQueue.h>                //  By Steven de Salas
#include <myHalfStepper.h>          // Author: Tom Biuso <tjbtech@yahoo.com>
#include <myEEPROM.h>               // needed for EEPROM
#include <myeepromanything.h>       // needed for EEPROM
#ifdef TEMPERATUREPROBE
#include <OneWire.h>                // needed for DS18B20 temperature probe, see https://github.com/PaulStoffregen/OneWire
#include <myDallasTemperature.h>    // needed for DS18B20 temperature probe, see https://github.com/milesburton/Arduino-Temperature-Control-Library
#endif
#include <stdint.h>
#ifdef TFTDISPLAY
#include <Wire.h>                         // needed for I2C, installed when installing the Arduino IDE
#include <myTFTv2.h>
#include <SPI.h>
#endif

// ----------------------------------------------------------------------------------------------------------
// GLOBAL DEFINES YOU CAN CHANGE - THESE CAN ONLY BE CONFIGURED AT COMPILE TIME FROM V269 ONWARDS
#define LCDUPDATESTEPCOUNT  15            // the number of steps moved which triggers an lcd update when moving, do not make too small
#define LCDPAGEDISPLAYTIME  25            // time in milliseconds that each lcd page is displayed for (2000-4000)
#define TEMPREFRESHRATE     1000L         // refresh rate between temperature conversions unless an update is requested via serial command
#define SERIALPORTSPEED     9600          // 9600, 14400, 19200, 28800, 38400, 57600

// ----------------------------------------------------------------------------------------------------------
// GLOBAL DEFINES
// DO NOT CHANGE

#define PBSWITCHESPIN       A0            // push button switches
#define INLED               A1            // in and out leds
#define OUTLED              A2
#define BUZZERPIN           A3            // buzzer
#define TEMPPIN             2             // temperature probe on pin 2, use 4.7k pullup

#define EEPROMSIZE          1024          // ATMEGA328P 1024 EEPROM
#define MAXCOMMAND          15            // : + 2 + 10 + # = 14
#define QUEUELENGTH         10            // number of commands that can be saved in the serial queue
#define VALIDDATAFLAG       99            // valid eeprom data flag
#define SLOW                0             // motorspeeds
#define MED                 1
#define FAST                2
#define STEP1               1             // step modes
#define STEP2               2

#define TEMP_PRECISION      10            // Set the default DS18B20 precision to 0.25 of a degree 9=0.5, 10=0.25, 11=0.125, 12=0.0625
#define MOTORPULSETIME      2             // drv8825 requires minimum 2uS pulse to step
#define MOVINGIN            0
#define MOVINGOUT           1
#define FOCUSERUPPERLIMIT   2000000000L   // arbitary focuser limit up to 2000000000
#define FOCUSERLOWERLIMIT   1024L         // lowest value that maxsteps can be
#define DEFAULTSTEPSIZE     50.0          // This is the default setting for the step size in microns
#define MINSTEPSIZE         0.001         // this is the minimum step size in microns
#define EEPROMWRITEINTERVAL 10000L        // interval in milliseconds to wait after a move before writing settings to EEPROM, 10s
#define DISPLAYPAGETIMEMAX  40
#define DISPLAYPAGETIMEMIN  20

struct config_t {
  int validdata;                          // if this is 99 then data is valid
  long fposition;                         // last focuser position
  long maxstep;                           // max steps
  double stepsize;                        // the step size in microns, ie 7.2, minimum value is 0.001 microns
  byte stepmode;                          // stepping mode, full, half, 1/4, 1/8. 1/16. 1/32 [1.2.4.8.16.32]
  byte ReverseDirection;                  // reverse direction
  byte coilPwr;                           // coil pwr
  byte tempmode;                          // temperature display mode, Celcius=1, Fahrenheit=0
  byte pagedisplaytime;                   // refresh rate of display - time each page is displayed for
  byte stepsizeenabled;                   // if 1, controller returns step size
  byte lcdupdateonmove;                   // update position on lcd when moving
  byte ds18b20resolution;                 // resolution of DS18B20 temperature probe
  byte tempcompenabled;                   // indicates if temperature compensation is enabled
  byte tempcoefficient;                   // steps per degree temperature coefficient value
  byte DelayAfterMove;                    // delay after movement is finished
  byte backlashsteps_in;                  // number of backlash steps to apply for IN moves
  byte backlashsteps_out;                 // number of backlash steps to apply for OUT moves
  byte focuserdirection;                  // keeps track of last focuser move direction
  byte backlash_in_enabled;               // enable or disable backlash compensation for IN
  byte backlash_out_enabled;              // enable or disable backlash compensation for OUT
  byte tcdirection;
  byte motorSpeed;
} myfocuser;

// DO NOT CHANGE
// Text Colors for TFT
#define CPositionColor  BRIGHT_RED
#define TPositionColor  RED
#define CoilPwrColor    YELLOW
#define ReverseColor    BLUE
#define StepModeColor   GREEN
#define TempColor       WHITE
#define MotorSpeedColor CYAN
#define MaxStepsColor   GRAY1
#define Portrait1       0
#define Landscape1      1
#define Portrait2       2
#define Landscape2      3
#define rotation        Portrait2
#define TextSize        2

#define L293DMINITFT 1

#ifdef L293DMINITFT
#define STEPSPERREVOLUTION  200           // NEMA17 motor Full steps
// you need to change the above line to reflect your stepper motor, examples below
// #define STEPSPERREVOLUTION 2048        // 24BBYJ-48 motor, if half stepping multiply by 2
// #define STEPSPERREVOLUTION 1036        // NEMA17-PG5 motor, if half stepping multiply by 2
// #define STEPSPERREVOLUTION 200         // NEMA17 motor, if half stepping multiply by 2
// #define STEPSPERREVOLUTION 5370        // NEMA17-PG25 motor, if half stepping multiply by 2
#define IN1                 3  // Stepper Motor stuff, control pins for L293D Mini driver board
#define IN2                 8
#define IN3                 9
#define IN4                 10
#define SLOWDELAY1          40
#define SLOWDELAY2          30
#define MEDDELAY1           20
#define MEDDELAY2           10
#define FASTDELAY1          10             // delay speed fast full steps
#define FASTDELAY2          10
// motor speeds in RPM - you might need to adjust these depending on the stepper motor you select and the step mode
// Only valid for a single step mode
#define motorSpeedSlowRPM   10
#define motorSpeedMedRPM    20
#define motorSpeedFastRPM   60
HalfStepper mystepper(STEPSPERREVOLUTION, IN1, IN2, IN3, IN4);  // initialize the stepper library
int motorSpeedRPM;                        // the motorspeed RPM setting
#endif

char programName[]    = "myFP2.L293DMINI.TFT";
char programVersion[] = "280";

Queue<String> queue(QUEUELENGTH);       // receive serial queue of commands
double ch1tempval;                      // temperature value for probe
double lasttempval;                     // holds previous temperature value - used if ismoving and if temp request < 10s apart
long previousMillis;                    // used as a delay whenever the EEPROM settings need to be updated
long currentPosition;                   // current position
long targetPosition;                    // target position
long newPos;                            // used to calculate and validate target position requests
int  currentaddr;                       // will be address in eeprom of the data stored
int  motorSpeedDelay;
byte writenow;                          // should we update values in eeprom
byte movestarted;
byte savedmotorSpeed;
byte jogging;
byte joggingDirection;                  // defined jogging direction, 0 = IN, 1 = OUT
byte isMoving;                          // is the motor currently moving
char line[MAXCOMMAND];                  // buffer for serial data
byte eoc;                               // end of command
byte idx;                               // index into command string
byte displayenabled;                    // used to enable and disable the display
byte tprobe1;                           // indicate if there is a probe attached to myFocuserPro2
byte motorspeedchange;                  // change motorspeed to slow when nearing target position?
byte tswthreshold;                      // number of steps at which stepper slows down as target position is approached
byte movedirection;                     // holds direction of new planned move
char tempString[12];
String fastreply;
byte enablestate;

#ifdef TEMPERATUREPROBE
OneWire oneWirech1(TEMPPIN);              // setup temperature probe
DallasTemperature sensor1(&oneWirech1);
DeviceAddress tpAddress;                  // holds address of the temperature probe
double starttempval;                      // when using temperature compensation, holds the start temperature value
byte tcstarted;                           // indicates if temperature compensation is enabled
long lasttempconversion;                  // holds time of last conversion
byte requesttempflag;
#endif

#ifdef TFTDISPLAY
byte updatelcdwhilemoving;                // used to trigger updatefposlcd() when focuser is moving
long olddisplaytimestampNotMoving;        // timestamp of last display update
byte updatecount;                         // loop variable used in updating lcd when moving
char verstr[]         = "Version ";
char copystr[]        = "(c) R. BROWN 2017";
char headerstr[]      = "myFocuserPro2";
char coilpwrstr[]     = "CoilPwr   = ";
char ontxt[]          = "ON  ";
char offtxt[]         = "OFF";
char reversestr[]     = "Reverse   = ";
char stepmodestr[]    = "Stepmode  = ";
char step1[]          = "Full ";
char step2[]          = "1/2  ";
char tempstr1[]       = "Temp      = ";
char fstr[] = "F";
char cstr[] = "C";
char mspeedstr[]      = "MotorSpd  = ";
char slowstr[]        = "Slow  ";
char medstr[]         = "Medium";
char faststr[]        = "Fast";
#endif

// ----------------------------------------------------------------------------------------------------------
// CODE START

void software_Reboot()
{
  asm volatile ( "jmp 0");  // jump to the start of the program
}

#ifdef PUSHBUTTONS
// read the push button switches and return state of switches
// 1 = SW1 ON AND SW2 OFF, 2 = SW2 ON AND SW1 OFF, 3 = SW1 ON and SW2 ON, 0 = OFF
int readpbswitches(int pinNum)
{
  // for 1.2K sw1 (681) 650-720, sw2 (338) 310-380, sw1 and sw2 (509) 460-530
  // for 1K sw1 (630) 600-660, sw2 (335) 300-370, sw1 and sw2 (510) 480-540
  int val = 0;                       // variable to store the read value
  digitalWrite(pinNum, HIGH);
  val = analogRead(pinNum);          // read the input pin
  if ( val >= 650 && val <= 720 )
  {
    return 1;                        // toggle sw1 ON and SW2 OFF
  }
  else if ( val >= 460 && val <= 530 )
  {
    return 3;                    // toggle sw1 and sw2 ON
  }
  else if ( val >= 310 && val <= 380 )
  {
    return 2;                    // toggle sw2 ON and SW1 OFF
  }
  else return 0;                   // switches are OFF
}
#endif

void clearOutput()
{
  // check to see what is selected, keep or release
  if ( myfocuser.coilPwr == 0)
  {
    digitalWrite( IN1, 0);
    digitalWrite( IN2, 0);
    digitalWrite( IN3, 0);
    digitalWrite( IN4, 0);
    enablestate = 0;
  }
}

void anticlockwise()
{
#ifdef INOUTLEDS
  (!myfocuser.ReverseDirection) ? digitalWrite( OUTLED, 1) : digitalWrite( INLED, 1);
#endif
  (!myfocuser.ReverseDirection) ? mystepper.step(1) : mystepper.step(-1);
  delayMicroseconds(MOTORPULSETIME);
#ifdef INOUTLEDS
  (!myfocuser.ReverseDirection) ? digitalWrite( OUTLED, 0 ) : digitalWrite( INLED, 0);
#endif
}

// Move stepper clockwise
void clockwise()
{
#ifdef INOUTLEDS
  (!myfocuser.ReverseDirection) ? digitalWrite( INLED, 1) : digitalWrite( OUTLED, 1);
#endif
  (!myfocuser.ReverseDirection) ? mystepper.step(-1) : mystepper.step(1);
  delayMicroseconds(MOTORPULSETIME);
#ifdef INOUTLEDS
  (!myfocuser.ReverseDirection) ? digitalWrite( INLED, 0 ) : digitalWrite( OUTLED, 0);
#endif
}

// set the microstepping mode
void setstepmode( byte stepmode)
{
  if ( stepmode == STEP1 )
  {
    mystepper.SetSteppingMode(SteppingMode::FULL);
    myfocuser.stepmode = STEP1;
  }
  else if ( stepmode == STEP2 )
  {
    mystepper.SetSteppingMode(SteppingMode::HALF);
    myfocuser.stepmode = STEP2;
  }
  else
  {
    mystepper.SetSteppingMode(SteppingMode::FULL);
    myfocuser.stepmode = STEP1;
  }
}

void updatemotorSpeedDelay()
{
  switch ( myfocuser.motorSpeed )                   // calculate delay based on stepmode and motorspeed
  {
    case SLOW: // slow
      switch (myfocuser.stepmode)
      {
        case STEP1: // full steps
          motorSpeedDelay = SLOWDELAY1;
          motorSpeedRPM = motorSpeedSlowRPM;
          break;
        case STEP2: // half steps
          motorSpeedDelay = SLOWDELAY2;
          motorSpeedRPM = motorSpeedSlowRPM * 4;
          break;
      }
      break;
    case MED: // medium
      switch (myfocuser.stepmode)
      {
        case STEP1: // full steps
          motorSpeedDelay = MEDDELAY1;
          motorSpeedRPM = motorSpeedMedRPM;
          break;
        case STEP2: // half steps
          motorSpeedDelay = MEDDELAY2;
          motorSpeedRPM = motorSpeedMedRPM * 3;
          break;
      }
      break;
    case FAST: // fast
      switch (myfocuser.stepmode)
      {
        case STEP1: // full steps
          motorSpeedDelay = FASTDELAY1;
          motorSpeedRPM = motorSpeedFastRPM;
          break;
        case STEP2: // half steps
          motorSpeedDelay = FASTDELAY2;
          motorSpeedRPM = motorSpeedFastRPM + (motorSpeedFastRPM / 2);
          break;
      }
      break;
  }
  mystepper.setSpeed(motorSpeedRPM);      // update the motor speed
}

#ifdef TEMPERATUREPROBE
void requesttemp()
{
  sensor1.requestTemperatures();
}

void readtemp()
{
  double result = sensor1.getTempCByIndex(0);             // get channel 1 temperature, always in celsius
  // sometimes sensor returns -127, not sure why, so check and use last reading if this is the case
  // range of sensor is -55c to +125c
  // limit to values of -40c to +80c (-40f to 176f)
  int tempval = (int) result;
  if ( tempval < -40 )
    ch1tempval = lasttempval;
  else if ( tempval > 80 )
    ch1tempval = lasttempval;
  else
  {
    ch1tempval = result;
    lasttempval = ch1tempval;             // remember last reading
  }
}
#endif

// convert string to long
long decstr2long(String line)
{
  char ch_arr[12];
  line.toCharArray(ch_arr, sizeof(ch_arr));
  return atol(ch_arr);
}

// convert string to int
int decstr2int(String line)
{
  int ret = 0;
  ret = line.toInt();
  return ret;
}

void sendresponsestr(String str)
{
  if (Serial)
  {
    Serial.print(str);
  }
}

void sendresponsenl()
{
  if (Serial)
  {
    Serial.println();
  }
}

void flageepromupdate()
{
  writenow = 1;                      // updating of EEPROM ON
  previousMillis = millis();            // start 30s time interval
}

// Serial Commands
void processCommand()
{
  byte len;
  byte cmdval;
  long pos;
  int paramval;
  String replystr;
  String mycmd;
  String param;

  replystr = queue.pop();
  len = (byte) replystr.length();
  if ( len == 2 )
  {
    mycmd = replystr;                         // a valid command with no parameters, ie, :01#
  }
  else if ( len > 2 )
  {
    mycmd = replystr.substring(0, 2);         // this command has parameters
    param = replystr.substring(2, len);
  }
  else return;

  cmdval = (byte) decstr2int(mycmd);

#ifdef DEBUG
  Serial.print("- replystr = "); Serial.print(replystr); Serial.print("#");
  Serial.print("- len = "); Serial.print(len); Serial.print("#");
  Serial.print("- mycmd = "); Serial.print(mycmd); Serial.print("#");
  Serial.print("- param = "); Serial.print(param); Serial.print("#");
  Serial.print("- cmdval = "); Serial.print(cmdval); Serial.print("#");
#endif
  switch ( cmdval )
  {
    case 0: // :00#     Pxxxx#    get current focuser position
      replystr = "P" + String(currentPosition) + "#";
      sendresponsestr(replystr);
      break;
    case 1: // :01#     Ixx#      get motor moving status - 01 if moving, 00 otherwise
      if (isMoving == 1)
        sendresponsestr("I01#");
      else
      {
        if ( movestarted )
        {
          delay(myfocuser.DelayAfterMove);
          movestarted = 0;
        }
        sendresponsestr("I00#");
      }
      flageepromupdate();
      break;
    case 2: // :02#     EOK#      get motor controller status - Controller Response to "Are we connected"- also see Wifi
      sendresponsestr("EOK#");
      break;
    case 3: // :03#     Fxxx#     get firmware version string
      replystr = "F" + String(programVersion) + "#";
      sendresponsestr(replystr);
      break;
    case 4: // :04#     FString#  get firmware version string (Fprogram name, version, #)
      sendresponsestr("F");
      sendresponsestr(String(programName));
      sendresponsenl();
      sendresponsestr(String(programVersion) + "#");
      break;
    case 6: // :06#     Zxxxxxx#  get temperature as a double XXXX
      // Send the command to get temperatures from DS18B20 probe
      // is there a probe?
      // there is not a probe so just return 20
      replystr = "Z" + String(ch1tempval, 3) + "#";
      sendresponsestr(replystr);
      break;
    case 8: // :08#     Mxxxxxx#  Get MaxStep, returns XXXXXX
      replystr = "M" + String(myfocuser.maxstep) + "#";
      sendresponsestr(replystr);
      break;
    case 10: // :10#    Yxxxxxx#  Get MaxIncrement, returns xxxxxx
      replystr = "Y" + String(myfocuser.maxstep) + "#";
      sendresponsestr(replystr);
      break;
    case 11:  // :11#   Oxx#      Get coil pwr setting (00 = coils released after move, 01 = coil pwr on after move)
      replystr = "O" + String(myfocuser.coilPwr) + "#";
      sendresponsestr(replystr);
      break;
    case 13:  // :13#   Rxx#      Get reverse direction setting, 00 off, 01 on
      replystr = "R" + String(myfocuser.ReverseDirection) + "#";
      sendresponsestr(replystr);
      break;
    case 21: // :21#    Qxx#      get temperature probe resolution setting (9, 10, 11 or 12)
#ifdef TEMPERATUREPROBE
      if ( tprobe1 == 1 )
      {
        myfocuser.ds18b20resolution = sensor1.getResolution(tpAddress);
        flageepromupdate();
      }
#endif
      replystr = "Q" + String(myfocuser.ds18b20resolution) + "#";
      sendresponsestr(replystr);
      break;
    case 24:  // :24#   1x#       Get state of Temperature Compensation, 0=disabled, 1=enabled
      replystr = "1" + String(myfocuser.tempcompenabled) + "#";
      sendresponsestr(replystr);
      break;
    case 25:  // :25#   Ax#       Get if Temperature Compensation available 0=No, 1=Yes
      replystr = "A0#";
#ifdef TEMPERATUREPROBE
      replystr = "A1#";                               // this focuser supports temperature compensation
#endif
      sendresponsestr(replystr);
      break;
    case 26:  // :26#   Bxxx#     Get Temperature Coefficient (in steps per degree)
      replystr = "B" + String(myfocuser.tempcoefficient) + "#";
      sendresponsestr(replystr);
      break;
    case 29:  // :29#   Sxx#      Get stepmode, returns XX#
      replystr = "S" + String(myfocuser.stepmode) + "#";
      sendresponsestr(replystr);
      break;
    case 32:  // :32#   Ux#       Get if stepsize is enabled in controller (1 or 0, 0/1)
      replystr = "U" + String(myfocuser.stepsizeenabled) + "#";
      sendresponsestr(replystr);
      break;
    case 33:  // :33#   Txxxxx#   Get step size in microns (if enabled by controller)
      replystr = "T" + String(myfocuser.stepsize) + "#";
      sendresponsestr(replystr);
      break;
    case 34:  // :34#   Xxxxxx#   get the time that an LCD screen is displayed for (in milliseconds, eg 2500 = 2.5seconds
      paramval = myfocuser.pagedisplaytime * 100;
      replystr = "X" + String(paramval) + "#";
      sendresponsestr(replystr);
      break;
    case 37:  // :37#   Dxx#      Get Display status 0=disabled, 1=enabled
      replystr = "D" + String(displayenabled) + "#";
      sendresponsestr(replystr);
      break;
    case 38:  // :38#   Dxx#      Get Temperature mode 1=Celsius, 0=Fahrenheight
      replystr = "b" + String(myfocuser.tempmode) + "#";
      sendresponsestr(replystr);
      break;
    case 39:  // :39#   Nxxxxxx#  Get the new motor position (target) XXXXXX (not used yet)
      replystr = "N" + String(targetPosition) + "#";
      sendresponsestr(replystr);
      break;
    case 41:  // :41#   String#   Troubleshooting only
      sendresponsestr("- #");
      break;
    case 43:    // :43#            Cx#          Get motorspeed (0-3)
      replystr = "C" + String(myfocuser.motorSpeed) + "#";
      sendresponsestr(replystr);
      break;
    case 45:  // :45#   Gxxx#     Get tswthreshold - value for which stepper slows down at end of its move
      replystr = "G" + String(tswthreshold) + "#";
      sendresponsestr(replystr);
      break;
    case 47:  // 47#    Jxxx#     Get if motorspeedchange enabled/disabled
      replystr = "J" + String(motorspeedchange) + "#";
      sendresponsestr(replystr);
      break;
    case 49:    // :49#         aXXXXX
      sendresponsestr("ab552efd25e454b36b35795029f3a9ba7#");
      break;
    case 62:  // :62#   Lxx#      Get update of position on lcd when moving (00=disable, 01=enable)
      replystr = "L" + String(myfocuser.lcdupdateonmove) + "#";
      sendresponsestr(replystr);
      break;
    case 63:  // :63#   Hxx#      get status of home position switch (0=open, 1=closed)
      replystr = "H00#";
      sendresponsestr(replystr);
      break;
    case 66:  // :66#   Kxx#      Get jogging state enabled/disabled
      replystr = "K" + String(jogging) + "#";
      sendresponsestr(replystr);
      break;
    case 68:  // :68#   Vxx#      Return jogging direction 0=IN, 1=OUT
      replystr = "V" + String(joggingDirection) + "#";
      sendresponsestr(replystr);
      break;
    case 72: // :72#            3xxx#   Gets DelayAfterMove
      replystr = "3" + String(myfocuser.DelayAfterMove) + "#";
      sendresponsestr(replystr);
      break;
    case 74:  // get backlash in enabled status
      replystr = "4" + String(myfocuser.backlash_in_enabled) + "#";
      sendresponsestr(replystr);
      break;
    case 76:  // get backlash OUT enabled status
      replystr = "5" + String(myfocuser.backlash_out_enabled) + "#";
      sendresponsestr(replystr);
      break;
    case 78: // get number of backlash steps IN
      replystr = "6" + String(myfocuser.backlashsteps_in) + "#";
      sendresponsestr(replystr);
      break;
    case 80: // get number of backlash steps OUT
      replystr = "7" + String(myfocuser.backlashsteps_out) + "#";
      sendresponsestr(replystr);
      break;
    case 83: // get if there is a temperature probe
      replystr = "c" + String(tprobe1) + "#";
      sendresponsestr(replystr);
      break;
    case 87:  // :87#    kx#     Get temp comp direction 1=IN
      replystr = "k" + String(myfocuser.tcdirection) + "#";
      sendresponsestr(replystr);
      break;
    case 89: // :89# 9x#          Get stepper power
      replystr = "91#";
      sendresponsestr(replystr);
      break;
    case 5:     // :05xxxxxx# None    Set new target position to xxxxxx (and focuser initiates immediate move to xxxxxx)
    case 28:    // :28#       None    home the motor to position 0
      if ( myfocuser.tempcompenabled == 0 )
      {
        if ( cmdval == 28 )
        {
          newPos = 0;                   // if this is a home then set target to 0
        }
        else
        {
          pos = decstr2long(param);     // else set target to a move command
          newPos = pos;
        }
        isMoving = 1;
#ifdef BACKLASH
        if ( newPos < currentPosition )
          movedirection = MOVINGIN;
        else
          movedirection = MOVINGOUT;
#ifdef DEBUG
        Serial.print("- Current Pos="); Serial.print(currentPosition); Serial.print("#");
        Serial.print("- New Pos="); Serial.print(newPos); Serial.print("#");
        Serial.print("- Previous direction="); Serial.print(myfocuser.focuserdirection); Serial.print("#");
        Serial.print("- New Direction="); Serial.print(movedirection); Serial.print("#");
#endif
        // determine if a change in direction has taken place
        if ( movedirection != myfocuser.focuserdirection )
        {
#ifdef DEBUG
          Serial.print("- Applying backlash#");
#endif
          long tmppos = newPos;
          // apply backlash because moving in opposite direction
          if ( movedirection == MOVINGIN )
          {
#ifdef DEBUG
            Serial.print("- Backlash Steps IN="); Serial.print(myfocuser.backlashsteps_in); Serial.print("#");
#endif
            // apply IN backlash steps
            for ( int steps = 0; steps < myfocuser.backlashsteps_in; steps++ )
            {
              anticlockwise();
              delayMicroseconds(motorSpeedDelay);
              tmppos--;
              if ( tmppos <= 0 )
              {
                newPos = 0;
                break;
              }
            }
          }
          else
          {
#ifdef DEBUG
            Serial.print("- Backlash steps OUT="); Serial.print(myfocuser.backlashsteps_out); Serial.print("#");
#endif
            // apply OUT backlash steps
            for ( int steps = 0; steps < myfocuser.backlashsteps_out; steps++ )
            {
              clockwise();
              delayMicroseconds(motorSpeedDelay);
              tmppos++;
              if ( tmppos >= myfocuser.maxstep )
              {
                newPos = myfocuser.maxstep;
                break;
              }
            }
          }
          myfocuser.focuserdirection = movedirection;
        }
#endif
#ifdef DEBUG
        Serial.print("- 2s delay before move#");
        delay(2000);
#endif
        // rangecheck target
        if ( newPos < 0 )
          newPos = 0;
        if ( newPos > myfocuser.maxstep)
          newPos = myfocuser.maxstep;
        movestarted = 1;
#ifdef DEBUG
        Serial.print("- Move to position "); Serial.print(newPos); Serial.print("#");
#endif
        targetPosition = newPos;
#ifdef TFTDISPLAY
        updatepositiontft();
#endif
        flageepromupdate();
        break;
      case 7: // :07xxxxxx# None    set MaxStep
        pos = decstr2long(param);
        if ( pos > FOCUSERUPPERLIMIT )            // range check the new value for maxSteps
          pos = FOCUSERUPPERLIMIT;
        if ( pos < FOCUSERLOWERLIMIT )            // avoid setting maxSteps too low
          pos = FOCUSERLOWERLIMIT;
        myfocuser.maxstep = pos;
        flageepromupdate();
      }
      break;
    case 12:   // :12xx#      None    set coil pwr 0=release pwr after move, 1=keep power on after move
      paramval = decstr2int(param);
      myfocuser.coilPwr = (byte) paramval & 0x01;
      flageepromupdate();
      break;
    case 14:    // :14xx#     None    set reverse direction setting 0=normal, 1=reverse
      paramval = decstr2int(param);
      myfocuser.ReverseDirection = (byte) paramval & 0x01;
      flageepromupdate();
      break;
    case 15:  // :15XX#   None    Set MotorSpeed, 00 = Slow, 01 = Med, 02 = Fast
      paramval = decstr2int(param);
      myfocuser.motorSpeed = savedmotorSpeed = (byte) paramval & 0x03;
      updatemotorSpeedDelay();
      flageepromupdate();
      break;
    case 16:  // :16#   None      Display in Celsius (LCD or TFT)
      myfocuser.tempmode = 1;
      flageepromupdate();
      break;
    case 17:  // :17#   None      Display in Fahrenheit (LCD or TFT)
      myfocuser.tempmode = 0;
      flageepromupdate();
      break;
    case 18:
      // :180#    None    set the return of user specified stepsize to be OFF - default
      // :181#    None    set the return of user specified stepsize to be ON - reports what user specified as stepsize
      paramval = decstr2int(param);
      myfocuser.stepsizeenabled = (byte) paramval & 0x01;
      flageepromupdate();
      break;
    case 19:  // :19xxxx#  None   set the step size value - double type, eg 2.1
      {
        double tempstepsize = (double) param.toFloat();
        if ( tempstepsize < MINSTEPSIZE )
          tempstepsize = DEFAULTSTEPSIZE;       // set default maximum stepsize
        myfocuser.stepsize = tempstepsize;
        flageepromupdate();
      }
      break;
    case 20:   // :20xx#    None    set the temperature resolution setting for the DS18B20 temperature probe
      // no probe, set to default
      myfocuser.ds18b20resolution = TEMP_PRECISION;
      paramval = decstr2int(param);             // read precision
      if ( tprobe1 == 1 )
      {
#ifdef TEMPERATUREPROBE
        sensor1.setResolution( tpAddress, paramval );
#endif
        myfocuser.ds18b20resolution = (byte) paramval; // save setting in EEPROM
        flageepromupdate();
      }
      break;
    case 22:  // :22xxx#    None    set the temperature compensation value to xxx
      paramval = decstr2int(param);
      if ( paramval < 0 )
        paramval = 0;
      else if ( paramval > 200 )
        paramval = 200;
      myfocuser.tempcoefficient = (byte) paramval;     // save setting in EEPROM
      flageepromupdate();
      break;
    case 23:   // :23x#   None    Set the temperature compensation ON (1) or OFF (0)
#ifdef TEMPERATUREPROBE
      paramval = decstr2int(param);
      myfocuser.tempcompenabled = (byte) paramval & 0x01;
      flageepromupdate();
#endif
      break;
    case 27:  // :27#   None      stop a move - like a Halt
      isMoving = 0;
      jogging = 0;
      targetPosition = currentPosition;
      flageepromupdate();
      break;
    case 30:  // :30xx#   None    set stepmode (1=Full, 2=Half, 4=1/4, 8=1/8, 16=1/16, 32=1/32, 64=1/64, 128=1/128)
      paramval = decstr2int(param);
      myfocuser.stepmode = (byte) paramval & 0x03;
      setstepmode(myfocuser.stepmode);
      updatemotorSpeedDelay();
      flageepromupdate();
      break;
    case 31:  // :31xxxxxx# None  set current motor position to xxxxxx (does not move, updates currentpos and targetpos to xxxxxx)
      pos = decstr2long(param);
      newPos = pos;
      // rangecheck target
      if ( newPos < 0 )
        newPos = 0;
      if ( newPos > myfocuser.maxstep)
        newPos = myfocuser.maxstep;
      isMoving = 0;
      currentPosition = targetPosition = newPos;
      flageepromupdate();
      break;
    case 35:  // :35xxxx# None    Set length of time an LCD page is displayed for in milliseconds
      pos = decstr2long(param) / 100;
      if ( pos < DISPLAYPAGETIMEMIN )           // bounds check to 2000-4000 2s-4s
        pos = DISPLAYPAGETIMEMIN;
      if ( pos > DISPLAYPAGETIMEMAX )
        pos = DISPLAYPAGETIMEMAX;
      myfocuser.pagedisplaytime = (byte) pos;
      flageepromupdate();
      break;
    case 36:
      // :360#		None		Disable Display
      // :361#		None		Enable Display
      // disable or enable the display setting
#ifdef TFTDISPLAY
      paramval = decstr2int(param);
      displayenabled = (byte) paramval & 0x01;
      if ( displayenabled == 0 )
      {
        // disable the screen by setting clearing the scree
        Tft.fillScreen();               // Clear screen
        TFT_BL_OFF;                     // turn off the background light
        displayenabled = 0;
      }
      else
      {
        TFT_BL_ON;                      // turn on the background light
        Tft.fillScreen();               // Clear screen
        displayenabled = 1;
      }
#endif
      break;
    case 40:    // :40#      None   Reset Arduino myFocuserPro2 controller
      software_Reboot();
      break;
    case 44:  // :44xxx#  None    Set motorspeed threshold when moving - switches to slowspeed when nearing destination
      paramval = decstr2int(param);
      if ( paramval < 50 )                  // range check
        paramval = 50;
      else if ( paramval > 200 )
        paramval = 200;
      tswthreshold = (byte) paramval;
      break;
    case 46:  // :46x#    None    Enable/Disable motorspeed change when moving
      paramval = decstr2int(param);
      motorspeedchange = (byte) paramval & 0x01;
      break;
    case 48:  // :48#     None    Save settings to EEPROM
      // copy current settings and write the data to EEPROM
      myfocuser.validdata = VALIDDATAFLAG;
      myfocuser.fposition = currentPosition;
      writeEEPROMNow();
      writenow = 0;
      break;
    case 61:  // :61xx#   None    set update of position on lcd when moving (00=disable, 01=enable)
      paramval = decstr2int(param);
      myfocuser.lcdupdateonmove = (byte) paramval & 0x01;
      flageepromupdate();
      break;
    case 64:  // :64xxx#  None    Move a specified number of steps
      paramval = decstr2int(param);
      isMoving = 1;
      movestarted = 1;
      newPos = currentPosition + paramval;
      // rangecheck target
      if ( newPos < 0 )
        newPos = 0;
      if ( newPos > myfocuser.maxstep)
        newPos = myfocuser.maxstep;
      targetPosition = newPos;
      flageepromupdate();
      break;
    case 65:  // :65xx#  None    Set jogging state enable/disable
      paramval = decstr2int(param);
      jogging = (byte) paramval & 0x01;
      break;
    case 67:  // :67#     None    Set jogging direction, 0=IN, 1=OUT
      paramval = decstr2int(param);
      joggingDirection = (byte) paramval & 0x01;
      break;
    case 42:    // :42#    None    Reset focuser defaults
      currentaddr = 0;
      setfocuserdefaults();
      // Set focuser defaults.
      currentPosition = myfocuser.fposition;
      targetPosition = myfocuser.fposition;
      break;
    case 71:  // :71xxx#  None      Set DelayAfterMove in milliseconds
      paramval = decstr2int(param);
      // bounds check to 0-250
      if ( paramval < 0 )
        paramval = 0;
      if ( paramval > 250 )
        paramval = 250;
      myfocuser.DelayAfterMove = (byte) paramval;
      flageepromupdate();
      break;
    case 73: // Disable/enable backlash IN (going to lower focuser position)
#ifdef BACKLASH
      paramval = decstr2int(param);
      myfocuser.backlash_in_enabled = (byte) paramval & 0x01;
      flageepromupdate();
#endif
      break;
    case 75: // Disable/enable backlash OUT (going to lower focuser position)
#ifdef BACKLASH
      paramval = decstr2int(param);
      myfocuser.backlash_out_enabled = (byte) paramval & 0x01;
      flageepromupdate();
#endif
      break;
    case 77: // set backlash in steps
#ifdef BACKLASH
      paramval = decstr2int(param);
      if ( paramval < 0 )                   // range check
        paramval = 0;
      myfocuser.backlashsteps_in = (byte) paramval & 0xff;
      flageepromupdate();
#endif
      break;
    case 79: // set backlash OUT steps
#ifdef BACKLASH
      paramval = decstr2int(param);
      if ( paramval < 0 )                   // range check
        paramval = 0;
      myfocuser.backlashsteps_out = (byte) paramval & 0xff;
      flageepromupdate();
#endif
      break;
    case 88:  //:88#          Set temp comp direction 1=IN
      paramval = decstr2int(param);
      myfocuser.tcdirection = (byte) paramval & 0x01;
      flageepromupdate();
      break;
  } // end of switch
}  // end of processcmd()

void writeEEPROMNow()
{
  EEPROM_writeAnything(currentaddr, myfocuser);    // update values in EEPROM
}

void setfocuserdefaults()
{
  myfocuser.validdata = VALIDDATAFLAG;
  myfocuser.maxstep = 10000L;
  myfocuser.fposition = 5000L;
  myfocuser.coilPwr = 1;
  myfocuser.ReverseDirection = 0;
  myfocuser.stepmode = 1;                             // full stepping
  myfocuser.pagedisplaytime = DISPLAYPAGETIMEMIN;
  myfocuser.stepsizeenabled = 0;                  // default state is step size OFF
  myfocuser.stepsize = DEFAULTSTEPSIZE;
  myfocuser.tempmode = 1;                          // default is celsius
  myfocuser.ds18b20resolution = TEMP_PRECISION;
  myfocuser.tempcompenabled = 0;
  myfocuser.tempcoefficient = 0;
  myfocuser.tcdirection = 1;
  myfocuser.lcdupdateonmove = 0;
  myfocuser.DelayAfterMove = 0;
  myfocuser.backlashsteps_in = 0;
  myfocuser.backlashsteps_out = 0;
  myfocuser.focuserdirection = MOVINGIN;
  myfocuser.backlash_in_enabled = 0;
  myfocuser.backlash_out_enabled = 0;
  myfocuser.motorSpeed = FAST;
  writeEEPROMNow();                                   // update values in EEPROM
  isMoving = 0;
  movestarted = 0;
}

#ifdef TFTDISPLAY
// display first screen
void displaytft()
{
  char cp[20];
  char tp[20];
  char ms[20];

  String cpstr = "CurrentPos= " + String(currentPosition);
  String tpstr = "TargetPos = " + String(targetPosition);
  String msstr = "MaxSteps  = " + String(myfocuser.maxstep);

  // display values on tft
  Tft.fillScreen();
  Tft.Rotation( rotation );

  // print header string size 3
  Tft.drawString(headerstr, 0, 0, TextSize + 1, WHITE );

  memset( cp, 0, 20 );
  memset( tp, 0, 20 );
  memset( ms, 0, 20 );
  cpstr.toCharArray(cp, 20);
  tpstr.toCharArray(tp, 20);
  msstr.toCharArray(ms, 20);
  Tft.drawString(cp, 0, 40, TextSize, CPositionColor);
  Tft.drawString(tp, 0, 60, TextSize, TPositionColor );

  Tft.drawString(coilpwrstr, 0, 80, TextSize, CoilPwrColor );
  if ( myfocuser.coilPwr )
  {
    Tft.drawString(ontxt, 140, 80, TextSize, CoilPwrColor );
  }
  else
  {
    Tft.drawString(offtxt, 140, 80, TextSize, CoilPwrColor );
  }

  Tft.drawString(reversestr, 0, 100, TextSize, ReverseColor );
  if ( myfocuser.ReverseDirection )
  {
    Tft.drawString(ontxt, 140, 100, TextSize, ReverseColor );
  }
  else
  {
    Tft.drawString(offtxt, 140, 100, TextSize, ReverseColor );
  }

  // stepmode setting
  Tft.drawString(stepmodestr, 0, 120, TextSize, StepModeColor );
  if ( myfocuser.stepmode == 1 )
  {
    Tft.drawString(step1, 140, 120, TextSize, StepModeColor  );
  }
  else if ( myfocuser.stepmode == 2 )
  {
    Tft.drawString(step2, 140, 120, TextSize, StepModeColor  );
  }

  Tft.drawString(tempstr1, 0, 140, TextSize, TempColor );
  char tempstr[8];
  memset( tempstr, 0, 8);
  if ( myfocuser.tempmode == 1 )
  {
    // no need to convert, already in C
    dtostrf(ch1tempval, 4, 3, tempstr);
    int lp = 0;
    int num = strlen(tempstr);
    while ( lp < num )
    {
      if ( tempstr[lp] == '.' )
        tempstr[lp] = '>';
      lp++;
    }
    Tft.drawFloat(ch1tempval, 140, 140, TextSize, TempColor );
    Tft.drawString(cstr, 230, 140, TextSize, TempColor );
  }
  else
  {
    // convert to F for display
    double tempval = ((ch1tempval * 9) / 5) + 32;
    dtostrf(tempval, 4, 3, tempstr);
    int lp = 0;
    int num = strlen(tempstr);
    while ( lp < num )
    {
      if ( tempstr[lp] == '.' )
        tempstr[lp] = '>';
      lp++;
    }
    Tft.drawFloat(tempval, 140, 140, TextSize, TempColor );
    Tft.drawString(fstr, 230, 140, TextSize, TempColor );
  }

  Tft.drawString(mspeedstr, 0, 160, TextSize, MotorSpeedColor );
  if ( myfocuser.motorSpeed == 0 )
  {
    Tft.drawString(slowstr, 140, 160, TextSize, MotorSpeedColor );
  }
  else if (myfocuser.motorSpeed == 1 )
  {
    Tft.drawString(medstr, 140, 160, TextSize, MotorSpeedColor );
  }
  else if ( myfocuser.motorSpeed == 2 )
  {
    Tft.drawString(faststr, 140, 160, TextSize, MotorSpeedColor );
  }
  Tft.drawString(ms, 0, 180, TextSize, MaxStepsColor );
}

void updatepositiontft()
{
  // every 15 times
  char cp[20];
  char tp[20];
  String cpstr = "CurrentPos= " + String(currentPosition);
  String tpstr = "TargetPos = " + String(targetPosition);
  memset( cp, 0, 20 );
  memset( tp, 0, 20 );
  cpstr.toCharArray(cp, 20);
  tpstr.toCharArray(tp, 20);
  Tft.fillScreen(0, 230, 40, 80, BLACK);      // black out previous value
  Tft.drawString(cp, 0, 40, TextSize, CPositionColor);
  Tft.drawString(tp, 0, 60, TextSize, TPositionColor );
}
#endif

#ifdef TEMPERATUREPROBE
// find the address of the DS18B20 sensor probe
void findds18b20address()
{
  // look for probes
  // Search the wire for address
  if (sensor1.getAddress(tpAddress, 0))
  {
    tprobe1 = 1;  // there is a probe1
  }
  // device address is now saved in tpAddress
}
#endif

// Setup
void setup()
{
  int datasize;      // will hold size of the struct myfocuser - 6 bytes
  int nlocations;    // number of storage locations available in EEPROM
  byte found;

  Serial.begin(SERIALPORTSPEED);            // initialize serial port
  clearSerialPort();                        // clear any garbage from serial buffer

#ifdef BUZZER
  pinMode(BUZZERPIN, OUTPUT);               // turn ON the BUZZER - provide power ON beep
  digitalWrite( BUZZERPIN, 1);
#endif

#ifdef INOUTLEDS
  pinMode( INLED, OUTPUT);                 // turn ON both LEDS as power on cycle indicator
  pinMode( OUTLED, OUTPUT);
  digitalWrite( INLED, 1 );
  digitalWrite( OUTLED, 1 );
#endif

  eoc = 0;
  idx = 0;
  isMoving = 0;
  writenow = 0;
  jogging = 0;
  joggingDirection = 0;
  displayenabled = 0;
  previousMillis = millis();
  memset(line, 0, MAXCOMMAND);
  ch1tempval  = 20.0;                       // start temperature sensor DS18B20
  lasttempval = 20.0;
  tprobe1 = 0;                              // set probe indicator NOT FOUND
  movestarted = 0;
  motorspeedchange = 0;                 // disable motorspeed change when nearing target position - will be updated on connect()
  tswthreshold = 200;                       // number of steps at which stepper slows down as target position is approached - will be updated on connect()

#ifdef TEMPERATUREPROBE
  tcstarted = 0;                        // temperature compensation is off
  lasttempconversion = previousMillis;
#endif

#ifdef TFTDISPLAY
  displayenabled = 1;                // display is enabled
  updatecount = 0;
  olddisplaytimestampNotMoving = previousMillis;
  TFT_BL_ON;                  // turn on the background light
  Tft.TFTinit();              // init TFT library
  Tft.Rotation( rotation );
  Tft.fillScreen();

  // print startup screen
  Tft.drawString(headerstr, 0, 0, TextSize + 1, WHITE );         // print header string size 3
  Tft.drawString(programName, 0, 40, TextSize, BLUE);
  Tft.drawString(verstr, 0, 60, TextSize, BLUE );
  Tft.drawString(programVersion, 86, 60, TextSize, BLUE);
  Tft.drawString(copystr, 0, 80, TextSize, BLUE);
#endif

  pinMode( IN1, OUTPUT );  // setup INx step lines as outputs, set low so motor does not move
  pinMode( IN2, OUTPUT );
  pinMode( IN3, OUTPUT );
  pinMode( IN4, OUTPUT );
  digitalWrite( IN1, 0);
  digitalWrite( IN2, 0);
  digitalWrite( IN3, 0);
  digitalWrite( IN4, 0);

  currentaddr = 0;                          // start at 0 if not found later
  found = 0;
  datasize = sizeof( myfocuser );
  nlocations = EEPROMSIZE / datasize;
  for (int lp1 = 0; lp1 < nlocations; lp1++ )
  {
    int addr = lp1 * datasize;
    EEPROM_readAnything( addr, myfocuser );
    if ( myfocuser.validdata == VALIDDATAFLAG ) // check to see if the data is valid
    {
      currentaddr = addr;                       // data was erased so write some default values
      found = 1;
    }
  }
  if ( found == 1 )
  {
    // set the focuser back to the previous settings
    // done after this in one hit
    // mark current eeprom address as invalid and use next one
    // each time focuser starts it will read current storage, set it to invalid, goto next location and
    // write values to there and set it to valid - so it doesnt always try to use same locations over and
    // over and destroy the eeprom
    // using it like an array of [0-nlocations], ie 100 storage locations for 1k EEPROM
    EEPROM_readAnything( currentaddr, myfocuser );
    myfocuser.validdata = 0;
    writeEEPROMNow();                       // update values in EEPROM
    currentaddr += datasize;                // goto next free address and write data
    // bound check the eeprom storage and if greater than last index [0-EEPROMSIZE-1] then set to 0
    if ( currentaddr >= (nlocations * datasize) )
      currentaddr = 0;
    myfocuser.validdata = VALIDDATAFLAG;
    writeEEPROMNow();                       // update values in EEPROM
  }
  else
  {
    // set defaults because not found
    setfocuserdefaults();
  }

#ifdef TEMPERATUREPROBE
  sensor1.begin();                          // start the temperature sensor1
  sensor1.getDeviceCount();                 // should return 1 if probe connected
  findds18b20address();
  if ( tprobe1 == 1 )
  {
    sensor1.setResolution( tpAddress, myfocuser.ds18b20resolution );   // set probe resolution
    requesttemp();
    delay(600 / (1 << (12 - myfocuser.ds18b20resolution))); // should enough time to wait
    readtemp();
  }
  requesttempflag = 0;
#endif

  myfocuser.tempcompenabled = 0;        // disable temperature compensation on startup else focuser will auto adjust whilst focusing!

  // range check focuser variables
  if ( myfocuser.pagedisplaytime < DISPLAYPAGETIMEMIN )
    myfocuser.pagedisplaytime = DISPLAYPAGETIMEMIN;
  else if ( myfocuser.pagedisplaytime > DISPLAYPAGETIMEMAX )
    myfocuser.pagedisplaytime = DISPLAYPAGETIMEMAX;
  if ( myfocuser.maxstep < FOCUSERLOWERLIMIT )
    myfocuser.maxstep = FOCUSERLOWERLIMIT;
  if ( myfocuser.fposition < 0 )
    myfocuser.fposition = 0;
  else if ( myfocuser.fposition > myfocuser.maxstep )
    myfocuser.fposition = myfocuser.maxstep;
  if ( myfocuser.stepsize < 0 )
    myfocuser.stepsize = 0;
  else if ( myfocuser.stepsize > DEFAULTSTEPSIZE )
    myfocuser.stepsize = DEFAULTSTEPSIZE;
  if ( myfocuser.DelayAfterMove > 250 )
    myfocuser.DelayAfterMove = 250;
#ifdef BACKLASH
  myfocuser.focuserdirection = myfocuser.focuserdirection & 0x01;
  movedirection = myfocuser.focuserdirection;
#endif
  if (myfocuser.tempcoefficient > 200)
    myfocuser.tempcoefficient = 200;
  myfocuser.motorSpeed = myfocuser.motorSpeed & 0x03;

#ifdef DEBUG
  Serial.print("myfocuser.fposition = ");
  Serial.println(myfocuser.fposition);
#endif
  currentPosition = myfocuser.fposition;
  targetPosition  = myfocuser.fposition;

  if ( myfocuser.coilPwr == 0)
    clearOutput();
  // restore stepmode setting and range check on restore
  if ( myfocuser.stepmode < 1 )
    myfocuser.stepmode = STEP1;
  else if ( myfocuser.stepmode > 2 )
    myfocuser.stepmode = STEP2;
  setstepmode(myfocuser.stepmode);
  savedmotorSpeed = myfocuser.motorSpeed;
  setstepmode(myfocuser.stepmode);
  updatemotorSpeedDelay();

  writenow = 1;

#ifdef INOUTLEDS
  digitalWrite( INLED, 0 );                // turn off the IN/OUT LEDS and BUZZER
  digitalWrite( OUTLED, 0 );
#endif

#ifdef BUZZER
  digitalWrite( BUZZERPIN, 0);
#endif
}

// Main Loop
void loop()
{
  if ( queue.count() >= 1 )                 // check for serial command
  {
    processCommand();
  }

#ifdef PUSHBUTTONS
  if ( myfocuser.tempcompenabled == 0 )
  {
    int PBVal = readpbswitches(PBSWITCHESPIN);
    if ( PBVal != 0)
    {
      delay(50);                             // wait small delay in case user is holding pb down
      PBVal = readpbswitches(PBSWITCHESPIN);
      if ( PBVal != 0 )
      {
        switch ( PBVal )                      // now check the pbval using a switch for 1 2 and 3
        {
          case 1:                             // toggle sw1 is ON and 2 is off
            // move IN
            movedirection = MOVINGIN;
            myfocuser.focuserdirection = movedirection;
            movestarted = 1;
            isMoving = 1;
            targetPosition = targetPosition - 1;
            if ( targetPosition < 0 )
              targetPosition = 0;
#ifdef TFTDISPLAY
            updatepositiontft();
#endif
            break;
          case 2:                             // toggle sw2 is ON and SW1 is OFF
            // move OUT
            movedirection = MOVINGOUT;
            myfocuser.focuserdirection = movedirection;
            movestarted = 1;
            isMoving = 1;
            targetPosition = targetPosition + 1;
            if ( targetPosition > myfocuser.maxstep )
              targetPosition = myfocuser.maxstep;
#ifdef TFTDISPLAY
            updatepositiontft();
#endif
            break;
          case 3:                             // toggle sw1 and sw2 are ON
#ifdef BUZZER
            digitalWrite(BUZZERPIN, 1);       // turn on buzzer
#endif
            while ( readpbswitches(PBSWITCHESPIN) == 3 )  // wait for pb to be released
              ;
            currentPosition = 0;
            targetPosition = 0;
            isMoving = 0;
#ifdef BUZZER
            digitalWrite(BUZZERPIN, 0);       // turn off buzzer
#endif
            break;
          default:
            // do nothing
            break;
        } // end of switch
      }
    }
  }
#endif

  if ( myfocuser.tempcompenabled == 0 )
  {
    if ( jogging == 1 )
    {
      movestarted = 1;
      isMoving = 1;
      if ( joggingDirection == 0 )
      {
        // move IN
        movedirection = MOVINGIN;
        myfocuser.focuserdirection = movedirection;
        movestarted = 1;
        targetPosition = targetPosition - 1;
        if ( targetPosition < 0 )
          targetPosition = 0;
#ifdef TFTDISPLAY
        updatepositiontft();
#endif
      }
      else
      {
        // move OUT
        movedirection = MOVINGOUT;
        myfocuser.focuserdirection = movedirection;
        movestarted = 1;
        targetPosition = targetPosition + 1;
        if ( targetPosition > myfocuser.maxstep )
          targetPosition = myfocuser.maxstep;
#ifdef TFTDISPLAY
        updatepositiontft();
#endif
      }
    }
  }

  // Move the position by a single step if target <> current position
  if (targetPosition != currentPosition)
  {
    if ( enablestate == 0 )
    {
      enablestate = 1;
      flageepromupdate();
    }
    isMoving = 1;                        // focuser is moving
    if ( motorspeedchange == 1 )
    {
      // Slow down if approaching home position
      long nearinghomepos = currentPosition - targetPosition;
      nearinghomepos = abs(nearinghomepos);
      if ( nearinghomepos < tswthreshold )
      {
        myfocuser.motorSpeed = SLOW;                  // slow
        updatemotorSpeedDelay();
      }
    }

    // Going Anticlockwise to lower position
    if (targetPosition < currentPosition)
    {
      anticlockwise();
      currentPosition--;
    }

    // Going Clockwise to higher position
    if (targetPosition > currentPosition)
    {
      // do not need to check if > maximumPosition as its done when a target command is receieved
      clockwise();
      currentPosition++;
    }

#ifdef TFTDISPLAY
    // check if lcd needs updating during move
    if ( myfocuser.lcdupdateonmove == 1 )
    {
      updatecount++;
      if ( updatecount > LCDUPDATESTEPCOUNT )
      {
        updatepositiontft();
        updatecount = 0;
      }
    }
#endif
    updatemotorSpeedDelay();
    delay( motorSpeedDelay );  // required else stepper will not move
  }
  else
  {
    // focuser has reached target, focuser is NOT moving now, move is completed
    isMoving = 0;

    // reset motorSpeed
    myfocuser.motorSpeed = savedmotorSpeed;         // restore original motorSpeed
    updatemotorSpeedDelay();                        // set the correct delay based on motorSpeed

#ifdef TFTDISPLAY   // see if the display needs updating
    long newdisplaytimestampNotMoving = millis();
    if ( ((newdisplaytimestampNotMoving - olddisplaytimestampNotMoving) > (myfocuser.pagedisplaytime * 100)) || (newdisplaytimestampNotMoving < olddisplaytimestampNotMoving) )
    {
      olddisplaytimestampNotMoving = newdisplaytimestampNotMoving;     // update the timestamp
      displaytft();                                     // update ALL the display values
    }
#endif

#ifdef TEMPERATUREPROBE                     // if there is a temperature probe
    if ( tprobe1 == 1)
    {
      long tempnow = millis();
      // see if the temperature needs updating - done automatically every 5s
      if ( ((tempnow - lasttempconversion) > TEMPREFRESHRATE) || (tempnow < lasttempconversion) )
      {
        lasttempconversion = millis();      // update
        if ( requesttempflag == 0 )
        {
          readtemp();
          requesttempflag = 1;
        }
        else
        {
          requesttemp();
          requesttempflag = 0;
        }
      } // end of check to see if it is time to get new temperature reading
    } // end of check for temperature probe present

    // check for temperature compensation;
    if ( myfocuser.tempcompenabled == 1 )
    {
      if ( tcstarted == 0 )
      {
        tcstarted = 1;
        starttempval = ch1tempval;
      }

      // if temperature has changed by 1 degree
      double tempchange = starttempval - ch1tempval;

      if ( tempchange >= 1 )
      {
        // move the focuser by the required amount
        // this should move focuser inwards
        newPos = targetPosition - myfocuser.tempcoefficient;
        // rangecheck target
        if ( newPos < 0 )
          newPos = 0;
        if ( newPos > myfocuser.maxstep)
          newPos = myfocuser.maxstep;
        movestarted = 1;
        targetPosition = newPos;
        tcstarted = 0;                  // indicate that temp compensation was done
      } // end of check for tempchange >=1
    } // end of check for tempcompenabled == 1)
#endif

    // is it time to update EEPROM settings?
    if ( writenow == 1 )
    {
      // decide if we have waited 10s (value of myinterval) after the last myfocuser key variable update, if so, update the EEPROM
      long currentMillis = millis();
      if ( ((currentMillis - previousMillis) > EEPROMWRITEINTERVAL) || (currentMillis < previousMillis ) )
      {
        myfocuser.validdata = VALIDDATAFLAG;
        myfocuser.fposition = currentPosition;
        writeEEPROMNow();    // update values in EEPROM
        writenow = 0;
        previousMillis = currentMillis;    // update the timestamp
      }
    }
    if ( myfocuser.coilPwr == 0 )
      clearOutput(); // release the stepper coils to save power
  }
}

void clearSerialPort()
{
  while ( Serial.available() )
    Serial.read();
}

// (IL: Needed on Pro Micro Clone otherwise serial input is never processed) -------------------------------------------------------------------------------
// On some boards serialEvent() is never called as serialEventRun is not defined
// properly by the IDE. If so this function provides the missing definition.
// Typically you will get a timeout error when you try to connect to the focuser
// using the ASCOM driver or focuser app then you need to uncomment FORCE_SERIALEVENT
// Checks to see if there is data in the Hardware Serial and if so calls serialEvent()
// to process it. This is called automatically between each loop() run and calls
// serialEvent() if there is data in the Hardware Serial that needs processing.
//-------------------------------------------------------------------------------
#ifdef CONTROLLERISAMICRO
void serialEventRun()
{
  if (Serial.available())
    serialEvent();
}
#endif
// (IL: End of extra Pro Micro function) -------------------------------------------------------------------------------

// SerialEvent occurs whenever new data comes in the serial RX.
void serialEvent()
{
  // : starts the command, # ends the command, do not store these in the command buffer
  // read the command until the terminating # character
  while (Serial.available() && !eoc)
  {
    char inChar = Serial.read();
    if (inChar != '#' && inChar != ':')
    {
      line[idx++] = inChar;
      if (idx >= MAXCOMMAND)
        idx = MAXCOMMAND - 1;
    }
    else
    {
      if (inChar == '#')
      {
        eoc = 1;
        idx = 0;
        if ( strcmp(line, ":00#") == 0 )   // get current focuser position
        {
          fastreply = "P" + String(currentPosition) + "#";
          sendresponsestr(fastreply);
        }
        else if ( strcmp( line, ":01#") == 0) // get motor moving status - 01 if moving, 00 otherwise
        {
          if (isMoving == 1)
            sendresponsestr("I01#");
          else
          {
            if ( movestarted )
            {
              delay(myfocuser.DelayAfterMove);
              movestarted = 0;
            }
            sendresponsestr("I00#");
          }
        }
        else if ( strcmp( line, ":27#") == 0) // Halt
        {
          // :27#   None      stop a move - like a Halt
          isMoving = false;
          jogging = false;
          targetPosition = currentPosition;
          flageepromupdate();
        }
        else
        {
          queue.push(String(line));
        }
        eoc = 0;
        memset( line, 0, MAXCOMMAND);
      }
    }
  }
}

