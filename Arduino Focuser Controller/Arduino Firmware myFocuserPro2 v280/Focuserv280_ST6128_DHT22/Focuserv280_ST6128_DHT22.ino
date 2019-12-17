// myfocuser example - ST6128 FULL
// ONLY FOR USE ST6128 Driver Module - supports 2A at 12VDC and 1/128 microstepping
// BIPOLAR STEPPER MOTORS ONLY
// SUPPORTS TEMPERATURE PROBE, LCD1602/I2C, PUSH BUTTONS
//
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

/* The driver is an ST6128 from StepperOnLine Motors&Electronics

  http://www.omc-stepperonline.com/bipolar-stepper-motor-driver-max-2a-current-128-high-subdivision-st6128-p-245.html
  $19.65 + Shipping

  or this unit from ebay
  http://www.ebay.com/itm/New-THB6128-Stepper-Motor-Driver-Board-For-24-28-39-42-Motors-2A-/251244889227?hash=item3a7f5cc48b
  $15.84USD

  or
  http://www.aliexpress.com/item/Thb6128-stepper-motor-driver-board-128-2a-motor/32360806327.html?spm=2114.01020208.3.19.4cgrYF&ws_ab_test=201556_1,201527_4_71_72_73_74_75,201560_2
  $15.42USD

  CONNECTIONS
  Arduino   ST6128
  D8    ENA+
  D4    PUL+
  D3    DIR+
  GND   ENA-, PUL-, DIR-, DC-

  12VDC PSU
  +VE   DC+
  -VE   DC-, Arduino GND
*/

// ----------------------------------------------------------------------------------------------------------
// PCB BOARDS
// Stripboard/Layout
// myFocuser_ST6128_Full_Layout
// myFocuser_ST6128_Full_schem

// ----------------------------------------------------------------------------------------------------------
// HARDWARE MAPPINGS
// Mapping for PushButtons
// A0

// Mapping for LCD DISPLAY
// VCC    5V
// GND    GND
// SDA    A4
// SCL    A5
// NOTE 2: IF USING LCD, USE A STRAIGHT 4P-4P HEADER CABLE

// ST6128 CONNECTIONS
// DC+ 12V
// DC- GND
// B- STEPPER MOTOR COIL 2 BLACK
// B+ STEPPER MOTOR COIL 2 GREEN
// A- STEPPER MOTOR COIL 1 BLUE
// A+ STEPPER MOTOR COIL 1 RED
// PUL+ D4
// PUL- GND
// DIR+ D3
// DIR- GND
// ENA+ D8
// ENA- GND

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

// To enable the LCD DISPLAY uncomment the next line (LCD1602, I2C)
//#define LCDDISPLAY 1

// To enable backlash in this firmware, uncomment the next line
#define BACKLASH 1

// To enable the buzzer, uncomment the next line
#define BUZZER 1

// To enable the IN-OUT LEDS, uncomment the next line
#define INOUTLEDS 1

// To enable support when usng a Micro instead of a Nano, uncomment the next line
//#define CONTROLLERISAMICRO 1           // provided by IL, enables the serialEventRun function for a Micro

// ----------------------------------------------------------------------------------------------------------
// FIRMWARE CHANGES

// 280
// Fixes

// 2.79
// Controller remembers motorspeed
// Please run the ClearEEPROM.ino file on the controller before programming with v279
// Note that running the ClearEEPROM will erase all focuser settings

// 2.78
// Major update - please use with Windows App 2.4.1.9 or higher

// 2.77
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
#include <myEEPROM.h>               // needed for EEPROM
#include <myeepromanything.h>       // needed for EEPROM
#ifdef TEMPERATUREPROBE
#include <mydht.h>                 // needed for DHT11/21/22/33
#endif
#ifdef LCDDISPLAY
#include <Wire.h>                   // needed for I2C, installed when installing the Arduino IDE
#include <LCD.h>
#include <LiquidCrystal_I2C.h>      // needed for LCD, see https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
#endif

// ----------------------------------------------------------------------------------------------------------
// GLOBAL DEFINES YOU CAN CHANGE - THESE CAN ONLY BE CONFIGURED AT COMPILE TIME FROM V269 ONWARDS
// only uncomment one of the following LCDxxxx lines depending upon your lcd type
#define LCD1602             1             // 16 character, 2 lines
//#define LCD1604           2             // 16 character, 4 lines
//#define LCD2004           3             // 20 character, 4 lines
#define LCDUPDATESTEPCOUNT  15            // the number of steps moved which triggers an lcd update when moving, do not make too small
#define LCDADDRESS          0x27          // some LCD displays maybe at 0x3F, use I2Cscanner to find the correct address
#define LCDPAGEDISPLAYTIME  25            // time in milliseconds that each lcd page is displayed for (2000-4000)
#define TEMPREFRESHRATE     1000L         // refresh rate between temperature conversions unless an update is requested via serial command
#define SERIALPORTSPEED     9600          // 9600, 14400, 19200, 28800, 38400, 57600

// ----------------------------------------------------------------------------------------------------------
// GLOBAL DEFINES
// DO NOT CHANGE
#define EEPROMSIZE          1024          // ATMEGA328P 1024 EEPROM
#define PBSWITCHESPIN       A0            // push button switches
#define INLED               A1            // in and out leds
#define OUTLED              A2
#define BUZZERPIN           A3            // buzzer

// Stepper Motor stuff, control pins for ST6128 board
#define MYDIR               3             // wire to DIR+
#define MYSTEP              4             // wire to PUL+
#define MYENABLE            8             // wire to ENA+
#define TEMPPIN             2             // temperature probe on pin 2, use 4.7k pullup

#define MAXCOMMAND          15            // : + 2 + 10 + # = 14
#define QUEUELENGTH         10            // number of commands that can be saved in the serial queue
#define VALIDDATAFLAG       99            // valid eeprom data flag
#define SLOW                0             // motorspeeds
#define MED                 1
#define FAST                2

// motorSpeed - time in milliseconds of delay between stepper pulses, you may need to change these
// only change these values if you have the current on the drv8255 set correctly
// if changing these, start with full steps at fast speed, then adjust motorSpeedFast so stepper moves
// if stepper does not move - sortof buzzes - value is too low, increase value
#define motorSpeedSlowDelay 4000
#define motorSpeedMedDelay  2750
#define motorSpeedFastDelay 1800

#define dhtdata             2             // DHT22 sensor connected to pin 2
#define DHT11               1             // defines for each sensor type that is supported
#define DHT21               2
#define DHT22               3
#define DHT33               4
#define MYSENSOR            DHT22         // the sensor I am using is DHT22, if using DH11 change this!!!

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

String programName = "myFP2.ST6128.DHT22";
String programVersion = "280";

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
String fastreply;
byte enablestate;

#ifdef LCDDISPLAY
LiquidCrystal_I2C lcd(LCDADDRESS, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
long olddisplaytimestampNotMoving;        // timestamp of last display update
byte updatecount;                          // loop variable used in updating lcd when moving
byte LCDScreen;                            // used to decide which LCD screen to display
#endif

#ifdef TEMPERATUREPROBE
dht mydht;                     // setup dhtxx sensor
double hval;                   // humidity value - NOT USED
double tval;                   // temperature value
byte dhtchk;                    // variable to hold result of reading sensor
byte dhterrorflag;             // state of DHTxx sensor read, 1 = Error, 0 = OK
double starttempval;           // when using temperature compensation, holds the start temperature value
byte tcstarted;                // indicates if temperature compensation is enabled
long lasttempconversion;       // holds time of last conversion
#endif

// ----------------------------------------------------------------------------------------------------------
// DO NOT CHANGE ANY OF THE FOLLOWING LINES

// do not change
#ifdef LCDDISPLAY
#ifdef LCD1602
#ifdef LCD1604
#halt //Error - you can only have one LCD type defined, one of LCD1602, LCD1604 or LCD2004.
#endif
#endif
#endif

// do not change
#ifdef LCDDISPLAY
#ifdef LCD1602
#ifdef LCD2004
#halt //Error - you can only have one LCD type defined, one of LCD1602, LCD1604 or LCD2004.
#endif
#endif
#endif

// do not change
#ifdef LCDDISPLAY
#ifdef LCD1604
#ifdef LCD2004
#halt //Error - you can only have one LCD type defined, one of LCD1602, LCD1604 or LCD2004.
#endif
#endif
#endif

// do not change
#ifdef LCDDISPLAY
#ifdef LCD1602
#ifdef LCD2004
#ifdef LCD1604
#halt //Error - you can only have one LCD type defined, one of LCD1602, LCD1604 or LCD2004.
#endif
#endif
#endif
#endif

// do not change
#ifdef LCDDISPLAY
#ifndef LCD2004
#ifndef LCD1604
#ifndef LCD1602
#halt //Error - you can must define either LCD1602 or LCD2004 or LCD1604.
#endif
#endif
#endif
#endif

// do not change
//#define DEBUG     1
//#define DEBUGHPSW 1

// ----------------------------------------------------------------------------------------------------------
// CODE START

void updatemotorSpeedDelay()
{
  switch ( myfocuser.motorSpeed )
  {
    case 0: motorSpeedDelay = motorSpeedSlowDelay; break;
    case 1: motorSpeedDelay = motorSpeedMedDelay; break;
    case 2: motorSpeedDelay = motorSpeedFastDelay; break;
    default: motorSpeedDelay = motorSpeedSlowDelay; break;
  }
}

// reboot the Arduino
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

// disable the stepper motor outputs - coil power off
void disableoutput()
{
  digitalWrite(MYENABLE, HIGH);
  // control via ENABLE pin, but this turns off indexer inside DRV8825
  // which means the stepper motor will loose power and move position to the nearest full step
  // not an issue if using full steps but major issue if using microstepping as will cause change
  // in focus position
  enablestate = 0;
}

// enable the stepper motor outputs - coil power on
void enableoutput()
{
  digitalWrite(MYENABLE, LOW);
  enablestate = 1;
}

// Move stepper anticlockwise
void anticlockwise()
{
#ifdef INOUTLEDS
  (!myfocuser.ReverseDirection) ? digitalWrite( OUTLED, 1) : digitalWrite( INLED, 1);
#endif
  (!myfocuser.ReverseDirection) ? digitalWrite(MYDIR, LOW ) : digitalWrite(MYDIR, HIGH );
  digitalWrite(MYSTEP, 1 );
  delayMicroseconds(MOTORPULSETIME);
  digitalWrite(MYSTEP, 0 );
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
  (!myfocuser.ReverseDirection) ? digitalWrite(MYDIR, HIGH ) : digitalWrite(MYDIR, LOW );
  digitalWrite(MYSTEP, 1 );
  delayMicroseconds(MOTORPULSETIME);
  digitalWrite(MYSTEP, 0 );
#ifdef INOUTLEDS
  (!myfocuser.ReverseDirection) ? digitalWrite( INLED, 0 ) : digitalWrite( OUTLED, 0);
#endif
}

// set the microstepping mode
void setstepmode( byte stepmode)
{
  // ignore - this does nothing really as the step mode is controlled by DIP switches on the driver board
}

#ifdef TEMPERATUREPROBE
void updatedhtsensor()
{
  // Read the humidity and temperature from DHTxx sensor
  dhterrorflag = 0;
  if ( MYSENSOR == DHT11 )
  {
    dhtchk = mydht.read11( dhtdata );
  }
  else if ( MYSENSOR == DHT21 )
  {
    dhtchk = mydht.read21( dhtdata );
  }
  else if ( MYSENSOR == DHT22 )
  {
    dhtchk = mydht.read22( dhtdata );
  }
  else if ( MYSENSOR == DHT33 )
  {
    dhtchk = mydht.read33( dhtdata );
  }
  else dhtchk = -3; // error - undefined sensor type

  if ( dhtchk == DHTLIB_OK )
  {
    // sensor is ok, so this is the main program now
    hval = mydht.humidity;        // read the humidity
    // Read the ambient temperature and add any calibration bias offset
    tval = mydht.temperature;
  }
  else
  {
    // do nothing as cannot read sensor
    dhterrorflag = 1;
  }
}

void gettemp()
{
  if ( dhterrorflag == 1 )         // error, so use last vaue
    tval = lasttempval;
  if ( tval < -40 )
    ch1tempval = lasttempval;
  else if ( tval > 80 )
    ch1tempval = lasttempval;
  else
  {
    ch1tempval = tval;
    lasttempval = ch1tempval;         // remember last reading
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
    case 21: // :21#    Qxx#    get temperature probe resolution setting (9, 10, 11 or 12)
      // DHT22 does not support this
      myfocuser.ds18b20resolution = TEMP_PRECISION;
      sendresponsestr("Q");
      sendresponsestr(String(myfocuser.ds18b20resolution));
      sendresponsestr("#");
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
    case 26:  // :26#   Bxxx#   get Temperature Coefficient (in steps per degree)
      replystr = "B" + String(myfocuser.tempcoefficient) + "#";
      sendresponsestr(replystr);
      break;
    case 29:    // :29#     Sxx#    get stepmode, returns XX#
      replystr = "S" + String(myfocuser.stepmode) + "#";
      sendresponsestr(replystr);
      break;
    case 32:  // :32#   Ux#       Get if stepsize is enabled in controller (1 or 0, 0/1)
      replystr = "U" + String(myfocuser.stepsizeenabled) + "#";
      sendresponsestr(replystr);
      break;
    case 33:    // :33#   Txxxxx#   get step size in microns (if enabled by controller)
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
      // check if focuser has hit Mechanical Home switch
      sendresponsestr("H00#");
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
#ifdef LCDDISPLAY
        updatepositionlcd();
#endif
        flageepromupdate();
      }
      break;
    case 7: // :07xxxxxx# None    set MaxStep
      pos = decstr2long(param);
      if ( pos > FOCUSERUPPERLIMIT )            // range check the new value for maxSteps
        pos = FOCUSERUPPERLIMIT;
      if ( pos < FOCUSERLOWERLIMIT )            // avoid setting maxSteps too low
        pos = FOCUSERLOWERLIMIT;
      myfocuser.maxstep = pos;
      flageepromupdate();
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
      // DHT22 does not support this
      myfocuser.ds18b20resolution = TEMP_PRECISION;
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
    case 30:    // :30xx#	  	None		set stepmode (1=Full, 2=Half, 4=1/4, 8=1/8, 16=1/16, 32=1/32, 64=1/64, 128=1/128);
      myfocuser.stepmode = 1;
      // ignore - set to full steps
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
      // :360#    None    Disable Display
      // :361#    None    Enable Display
      paramval = decstr2int(param);
      displayenabled = (byte) paramval & 0x01;
#ifdef LCDDISPLAY
      if ( displayenabled == 0 )
      {
        // disable the screen by setting clearing the screen and then the text color to black
        // clear the screen
        lcd.noDisplay();
        lcd.noBacklight();
      }
      else
      {
        // set the text color to white
        lcd.display();
        lcd.backlight();
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
  EEPROM_writeAnything(currentaddr, myfocuser);       // update values in EEPROM
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

#ifdef LCDDISPLAY
#ifdef LCD2004
void  displaylcd2004pg1()
{
  char tempString[8];
  lcd.print("POS=");
  lcd.print(currentPosition);
  lcd.setCursor( 0, 1);
  lcd.print("TAR=");
  lcd.print(targetPosition);
  lcd.setCursor( 0, 2 );
  lcd.print("CPwr=");
  if ( myfocuser.coilPwr ) lcd.print("Y"); else lcd.print("N");
  lcd.setCursor( 10, 2 );
  lcd.print("RDir=");
  if ( myfocuser.ReverseDirection ) lcd.print("Y"); else lcd.print("N");
  lcd.setCursor(0, 3);
  lcd.print("Temp=");
  if ( tprobe1 == 0 )
  {
    lcd.print("--");
  }
  else
  {
    if ( myfocuser.tempmode == 1 )
    {
      /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
      dtostrf(ch1tempval, 4, 2, tempString);
      lcd.print(tempString);
    }
    else
    {
      float tempvalf;
      tempvalf = (ch1tempval * 1.8) + 32;
      dtostrf(tempvalf, 4, 2, tempString);
      lcd.print(tempString);
    }
  }
  LCDScreen = 2;
}

void  displaylcd2004pg2()
{
  lcd.setCursor( 0, 0);
  lcd.print("MSpd =");
  switch (myfocuser.motorSpeed )
  {
    case SLOW: lcd.print("S"); break;
    case MED:  lcd.print("M"); break;
    case FAST: lcd.print("F"); break;
  }

  lcd.setCursor( 10, 0 );
  lcd.print("SM=Usr");
  lcd.setCursor( 0, 1);
  lcd.print("MStep=");
  lcd.print(myfocuser.maxstep);

  lcd.setCursor(0, 2);
  lcd.print("TComp=");
  if ( myfocuser.tempcompenabled == 1 )
  {
    lcd.print("Y");
  }
  else
  {
    lcd.print("N");
  }
  lcd.setCursor( 0, 3);
  if ( myfocuser.tempmode == 1 )
  {
    lcd.print("Celsius");
  }
  else
  {
    lcd.print("Fahren");
  }
  LCDScreen = 1;
}
#endif
#endif

#ifdef LCDDISPLAY
#ifdef LCD1604
void  displaylcd1604pg1()
{
  lcd.print("POS=");
  lcd.print(currentPosition);
  lcd.setCursor( 0, 1);
  lcd.print("TAR=");
  lcd.print(targetPosition);
  lcd.setCursor( 0, 2 );
  lcd.print("CPwr=");
  if ( myfocuser.coilPwr ) lcd.print("Y"); else lcd.print("N");
  lcd.setCursor( 7, 2 );
  lcd.print("RDir=");
  if ( myfocuser.ReverseDirection ) lcd.print("Y"); else lcd.print("N");
  lcd.setCursor( 0, 3 );
  lcd.print("MSpd=");
  switch (myfocuser.motorSpeed )
  {
    case SLOW: lcd.print("S"); break;
    case MED:  lcd.print("M"); break;
    case FAST: lcd.print("F"); break;
  }

  lcd.setCursor( 7, 3 );
  lcd.print("StepM=U");
  LCDScreen = 2;
}

void  displaylcd1604pg2()
{
  char tempString[8];
  lcd.setCursor( 0, 0);
  lcd.print("Temp    =");
  if ( tprobe1 == 0 )
  {
    lcd.print("--");
  }
  else
  {
    if ( myfocuser.tempmode == 1 )
    {
      /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
      dtostrf(ch1tempval, 4, 3, tempString);
      lcd.print(tempString);
    }
    else
    {
      float tempvalf;
      tempvalf = (ch1tempval * 1.8) + 32;
      dtostrf(tempvalf, 4, 3, tempString);
      lcd.print(tempString);
    }
  }
  lcd.setCursor( 0, 1);
  lcd.print("MaxStep =");
  lcd.print(myfocuser.maxstep);
  lcd.setCursor( 0, 2);
  lcd.print("TempComp=");
  if ( myfocuser.tempcompenabled == 1 )
  {
    lcd.print("Y");
  }
  else
  {
    lcd.print("N");
  }
  lcd.setCursor( 0, 3);
  if ( myfocuser.tempmode == 1 )
  {
    lcd.print("Celsius");
  }
  else
  {
    lcd.print("Fahren");
  }
  LCDScreen = 1;
}
#endif
#endif

#ifdef LCDDISPLAY
#ifdef LCD1602
void displaylcd1602pg1()
{
  lcd.print("POS=");
  lcd.print(currentPosition);
  lcd.setCursor( 0, 1);
  lcd.print("TAR=");
  lcd.print(targetPosition);
  LCDScreen = 2;
}

void displaylcd1602pg2()
{
  lcd.setCursor( 0, 0 );
  lcd.print("CoilPwr=");
  if ( myfocuser.coilPwr )
    lcd.print("Y");
  else
    lcd.print("N");
  lcd.setCursor( 0, 1 );
  lcd.print("RevDir =");
  if ( myfocuser.ReverseDirection )
    lcd.print("Y");
  else
    lcd.print("N");
  LCDScreen = 3;
}

void displaylcd1602pg3()
{
  char tempString[8];
  // display values on LCD1602
  lcd.print("Temp   =");
  if ( tprobe1 == 0 )
  {
    lcd.print("--");
  }
  else
  {
    if ( myfocuser.tempmode == 1 )
    {
      /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
      dtostrf(ch1tempval, 4, 2, tempString);
      lcd.print(tempString);
    }
    else
    {
      float tempvalf;
      tempvalf = (ch1tempval * 1.8) + 32;
      dtostrf(tempvalf, 4, 2, tempString);
      lcd.print(tempString);
    }
  }
  lcd.setCursor( 0, 1);
  lcd.print("MaxStep=");
  lcd.print(myfocuser.maxstep);
  LCDScreen = 4;
}

void displaylcd1602pg4()
{
  lcd.print("Speed   =");
  switch (myfocuser.motorSpeed )
  {
    case SLOW: lcd.print("S"); break;
    case MED:  lcd.print("M"); break;
    case FAST: lcd.print("F"); break;
  }
  lcd.setCursor( 0, 1);
  lcd.print("StepMode=Usr");
  LCDScreen = 5;
}

void displaylcd1602pg5()
{
  lcd.setCursor( 0, 0);
  lcd.print("TempComp=");
  if ( myfocuser.tempcompenabled == 1 )
  {
    lcd.print("Y");
  }
  else
  {
    lcd.print("N");
  }
  lcd.setCursor( 0, 1);
  if ( myfocuser.tempmode == 1 )
  {
    lcd.print("Celsius");
  }
  else
  {
    lcd.print("Fahren");
  }
  LCDScreen = 1;
}
#endif

void updatepositionlcd()
{
  LCDScreen = 1;
  lcd.setCursor(0, 0);
  lcd.print("               ");
  lcd.setCursor(0, 0);
  lcd.print("POS=");
  lcd.print(currentPosition);
  lcd.setCursor(0, 1);
  lcd.print("               ");
  lcd.setCursor(0, 1);
  lcd.print("TAR=");
  lcd.print(targetPosition);
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

#ifdef LCDDISPLAY
  displayenabled = 1;                    // display is enabled
  LCDScreen = 1;                            // set for page 1
  updatecount = 0;
  olddisplaytimestampNotMoving = previousMillis;
#ifdef LCD1602
  lcd.begin(16, 2);
#endif
#ifdef LCD1604
  lcd.begin(16, 4);
#endif
#ifdef LCD2004
  lcd.begin(20, 4);
#endif
  lcd.setBacklight(HIGH);
  lcd.clear();                        // print the title of program and copyright message to lcd
  lcd.print(programName);
  lcd.setCursor(0, 1);
  lcd.print(programVersion);
#endif

  pinMode(  MYDIR, OUTPUT );
  pinMode(  MYSTEP, OUTPUT );
  digitalWrite( MYDIR, 0 );         // set direction and step to low
  digitalWrite( MYSTEP, 0 );
  pinMode( MYENABLE, OUTPUT );    // enable the driver board

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
  dhterrorflag = 0;
  updatedhtsensor();                  // trigger humidity and ambient and calc dew_point
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

  if ( myfocuser.coilPwr == 1 )
    enableoutput();
  else
    disableoutput();

  savedmotorSpeed = myfocuser.motorSpeed;
  setstepmode(myfocuser.stepmode);
  updatemotorSpeedDelay();

  writenow = 1;                          // ensure validated values are saved

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
#ifdef LCDDISPLAY
            updatepositionlcd();
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
#ifdef LCDDISPLAY
            updatepositionlcd();
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

  if ( jogging == 1 )
  {
    if ( myfocuser.tempcompenabled == 0 )
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
#ifdef LCDDISPLAY
        updatepositionlcd();
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
#ifdef LCDDISPLAY
        updatepositionlcd();
#endif
      }
    }
  }

  // Move the position by a single step if target <> current position
  if (targetPosition != currentPosition)
  {
    if ( enablestate == 0 )
    {
      enableoutput();
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
        updatemotorSpeedDelay();            // update the motordelay time
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

#ifdef LCDDISPLAY
    // check if lcd needs updating during move
    if ( myfocuser.lcdupdateonmove == 1 )
    {
      updatecount++;
      if ( updatecount > LCDUPDATESTEPCOUNT )
      {
        updatepositionlcd();
        updatecount = 0;
      }
    }
#endif
    updatemotorSpeedDelay();                        // ensure delay is set correctly
    delayMicroseconds(motorSpeedDelay);
  }
  else
  {
    // focuser has reached target, focuser is NOT moving now, move is completed
    isMoving = 0;

    // reset motorSpeed
    myfocuser.motorSpeed = savedmotorSpeed;         // restore original motorSpeed
    updatemotorSpeedDelay();                        // set the correct delay based on motorSpeed

#ifdef LCDDISPLAY
    // see if the display needs updating
    long currentMillis = millis();
    if ( ((currentMillis - olddisplaytimestampNotMoving) > (myfocuser.pagedisplaytime * 100)) || (currentMillis < olddisplaytimestampNotMoving) )
    {
      olddisplaytimestampNotMoving = currentMillis;    // update the timestamp
      lcd.clear();
#ifdef LCD1602
      switch ( LCDScreen)
      {
        case 1: displaylcd1602pg1(); break;
        case 2: displaylcd1602pg2(); break;
        case 3: displaylcd1602pg3(); break;
        case 4: displaylcd1602pg4(); break;
        case 5: displaylcd1602pg5(); break;
      }
#endif
#ifdef LCD1604
      switch ( LCDScreen)
      {
        case 1: displaylcd1604pg1(); break;
        case 2: displaylcd1604pg2(); break;
      }
#endif
#ifdef LCD2004
      switch ( LCDScreen)
      {
        case 1: displaylcd2004pg1(); break;
        case 2: displaylcd2004pg2(); break;
      }
#endif
    }
#endif

#ifdef TEMPERATUREPROBE
    // if there is a temperature probe
    if ( tprobe1 == 1)
    {
      long tempnow = millis();
      // see if the temperature needs updating - done automatically every 5s
      if ( ((tempnow - lasttempconversion) > TEMPREFRESHRATE) || (tempnow < lasttempconversion) )
      {
        lasttempconversion = millis();      // update
        gettemp();
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
    if ( myfocuser.coilPwr == 1 )
      enableoutput();
    else
      disableoutput();
  }
}

void clearSerialPort()
{
  while (Serial.available())
  {
    Serial.read();
  }
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

