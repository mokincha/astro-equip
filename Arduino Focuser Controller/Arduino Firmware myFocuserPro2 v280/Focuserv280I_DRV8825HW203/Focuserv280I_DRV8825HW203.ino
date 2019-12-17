// INTERRUPT DRIVEN CODE
// myfocuser DRV8825 HW203
// ONLY FOR USE DRV8825 AND EASYDRIVER Driver Board - supports 1.5A at 12VDC and microstepping
// BIPOLAR STEPPER MOTORS ONLY
// HW V203 OF DRV8825 BOARD LAYOUT (M0/M2 swapped)
// SUPPORTS TEMPERATUE PROBE
// SUPPORTS PUSH BUTTOMS

// NO DISPLAY
// NO HOME POSITION SWITCH

// (c) Copyright Robert Brown 2014-2019. All Rights Reserved.

// YOU MAY NOT SELL CONTROLLERS OR PCB'S BASED ON THIS PROJECT
// without express written permission granted from the author.
// Schematics, Code, Firmware, Ideas, Applications, Layout are protected by International Copyright Law.

// Permission is NOT granted to any person to redistribute, market, manufacture or sell for commercial gain the myFocuser
// products, ideas, PCB's, circuits, builds, variations and units described or discussed herein or on this site.
// Permission is granted for personal and Academic/Educational use only.

// THIS MEANS THAT YOU CANNOT RE-DESIGN AND MAKE AVAILABLE KITS OR PCB BASED ON THIS PROJECT AND
// OFFER THOSE KITS FOR SALE TO OTHERS. THAT IS A BREACH OF COPYRIGHT.

// ----------------------------------------------------------------------------------------------------------
// PCB to use
// DRV8825HW203-M-MT-F-BT    https://aisler.net/p/DWEURMAC
// DRV8825HW203_FIRBT        https://aisler.net/p/UGHLNFJC      for infra red controller
// DRV8825HW203_FRE          https://aisler.net/p/KMHRZAON      for rotary encoder

// ----------------------------------------------------------------------------------------------------------
// HARDWARE MAPPINGS
// Mapping for PushButtons
// A0

// ----------------------------------------------------------------------------------------------------------
// DEFINABLE FEATURES
// Caution: Do not enable a feature if you have not added the associated hardware circuits to support that feature
// With this version you can enable or disable specific add-ons and build the controller with the options you want
// using a single firmware file
// By default, this is a MINIMAL Controller firmware

// To enable the 12V power detect to the stepper motor, uncomment the next line
//#define STEPPERPWRDETECT 1
// This prevents the stepper motor moving when 12V to the stepper is OFF - needs special circuitry or has no effect

// To enable temperature probe, uncomment next line
//#define TEMPERATUREPROBE 1

// To enable the Push Buttons for manual focusing, uncomment the next line
//#define PUSHBUTTONS 1

// To enable backlash in this firmware, uncomment the next line
//#define BACKLASH 1

// To enable the buzzer, uncomment the next line
#define BUZZER 1

// To enable the IN-OUT LEDS, uncomment the next line
#define INOUTLEDS 1

// ----------------------------------------------------------------------------------------------------------
// FIRMWARE CHANGES

// 2.80I
// Add 12V stepper power detect circuit, if 12V power not present stepper will not move when asked to step
// Requires additional circuitry. All Aisler boards changed to add this support dated from 10-Jan-2019
// Fix for serial port error when HPSW enabled

// 2.79I
// Controller remembers motorspeed
// Please run the ClearEEPROM.ino file on the controller before programming with v279
// Note that running the ClearEEPROM will erase all focuser settings

// v278I
// Major update - please use with Windows App 2.4.1.9 or higher
// Fix for delay after move
// Fix bug in tempmode C/F

// v2.75I
// Use interrupts to drive stepper motor

// 2.75
// Add call to get temperature mode setting, Celsius or Fahrenheight
// Add call to get if there is a temperature probe
// Requires WinApp 2.4.1.1 or higher

// 2.74
// Revert back to move restrictions when in TC mode enabled

// 2.73
// Test more changes to Push Button code
// Added NOKIA 5110 LCD option to L293DMini, ULN2003 and L9110S

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
// FIRMWARE START
#include <Arduino.h>
#include <myQueue.h>                //  By Steven de Salas
#include <myEEPROM.h>               // needed for EEPROM
#include <myeepromanything.h>       // needed for EEPROM
#ifdef TEMPERATUREPROBE
#include <OneWire.h>                // needed for DS18B20 temperature probe, see https://github.com/PaulStoffregen/OneWire
#include <myDallasTemperature.h>    // needed for DS18B20 temperature probe, see https://github.com/milesburton/Arduino-Temperature-Control-Library
#endif

// ----------------------------------------------------------------------------------------------------------
// GLOBAL DEFINES YOU CAN CHANGE - THESE CAN ONLY BE CONFIGURED AT COMPILE TIME FROM V269 ONWARDS
#define LCDPAGEDISPLAYTIME  2500          // time in milliseconds that each lcd page is displayed for (2000-4000)
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
#define DRV8825DIR          3             // drv8825 control lines
#define DRV8825STEP         4
#define DRV8825ENABLE       8
#define DRV8825M2           5             // for easydriver, do not connect
#define DRV8825M1           6             // for easydriver, connect to MS1
#define DRV8825M0           7             // for easydriver, connect to MS0
#define TEMPPIN             2             // temperature probe on pin 2, use 4.7k pullup
#define STEPPERDETECTPIN    A7

#define MAXCOMMAND          15            // : + 2 + 10 + # = 14
#define QUEUELENGTH         10            // number of commands that can be saved in the serial queue
#define VALIDDATAFLAG       99            // valid eeprom data flag
#define SLOW                0             // motorspeed
#define MED                 1
#define FAST                2
#define STEP1               1             // step modes
#define STEP2               2
#define STEP4               4
#define STEP8               8
#define STEP16              16
#define STEP32              32

#define FAST1DELAY          6
#define FAST2DELAY          3
#define FAST4DELAY          1
#define FAST8DELAY          1
#define FAST16DELAY         1
#define FAST32DELAY         1
#define MED1DELAY           14
#define MED2DELAY           10
#define MED4DELAY           6
#define MED8DELAY           3
#define MED16DELAY          2
#define MED32DELAY          1
#define SLOW1DELAY          30
#define SLOW2DELAY          18
#define SLOW4DELAY          10
#define SLOW8DELAY          5
#define SLOW16DELAY         2
#define SLOW32DELAY         1

#define TEMP_PRECISION      10            // Set the default DS18B20 precision to 0.25 of a degree 9=0.5, 10=0.25, 11=0.125, 12=0.0625
#define MOVINGIGNORE        0
#define MOVINGIN            0
#define MOVINGOUT           1
#define FOCUSERUPPERLIMIT   2000000000L   // arbitary focuser limit up to 2000000000
#define FOCUSERLOWERLIMIT   1024L         // lowest value that maxsteps can be
#define DEFAULTSTEPSIZE     50.0          // This is the default setting for the step size in microns
#define MINSTEPSIZE         0.001         // this is the minimum step size in microns
#define DISPLAYPAGETIMEMAX  40
#define DISPLAYPAGETIMEMIN  20
#define EEPROMWRITEINTERVAL 10000L        // interval in milliseconds to wait after a move before writing settings to EEPROM, 10s
#define HPSWOPEN            HIGH
#define HPSWCLOSED          LOW

#define MAXBACKLASHSTEPS    50
#define DIRECTIONIN         1
#define DIRECTIONOUT        0
#define PUSHBUTTONTIME      500L          // time between push button checks, 500mS, do not make too small
int stepontime;
int motorskip;

struct config_t {
  int validdata;                          // if this is 99 then data is valid
  long fposition;                         // last focuser position
  long maxstep;                           // max steps
  double stepsize;                        // the step size in microns, ie 7.2, minimum value is 0.001 microns
  byte stepmode;                           // stepping mode, full, half, 1/4, 1/8. 1/16. 1/32 [1.2.4.8.16.32]
  byte ReverseDirection;                  // reverse direction
  byte coilPwr;                           // coil pwr
  byte tempmode;                          // temperature display mode, Celcius=1, Fahrenheit=0
  byte pagedisplaytime;                   // refresh rate of display - time each page is displayed for
  byte stepsizeenabled;                   // if 1, controller returns step size
  byte lcdupdateonmove;                   // update position on lcd when moving
  byte ds18b20resolution;                  // resolution of DS18B20 temperature probe
  byte tempcompenabled;                   // indicates if temperature compensation is enabled
  byte tempcoefficient;                    // steps per degree temperature coefficient value
  byte DelayAfterMove;                     // delay after movement is finished
  byte backlashsteps_in;                   // number of backlash steps to apply for IN moves
  byte backlashsteps_out;                  // number of backlash steps to apply for OUT moves
  byte focuserdirection;                  // keeps track of last focuser move direction
  byte backlash_in_enabled;               // enable or disable backlash compensation for IN
  byte backlash_out_enabled;              // enable or disable backlash compensation for OUT
  byte tcdirection;
  byte motorSpeed;
} myfocuser;

// ----------------------------------------------------------------------------------------------------------
// PROGRAM VARIABLES
char programName[] = "myFP2.DRV8825HW203.I";
char programVersion[] = "280";

int currentaddr;                    // will be address in eeprom of the data stored
byte writenow;                      // should we update values in eeprom
long previousMillis;                // used as a delay whenever the EEPROM settings need to be updated
byte movestarted;
long motorSpeedDelay;
byte savedmotorSpeed;               // used to save original speed if slowing down when nearing target position
byte jogging;
byte joggingDirection;              // defined jogging direction, 0 = IN, 1 = OUT
long currentPosition;               // current position
long targetPosition;                // target position
long newPos;                        // used to calculate and validate target position requests
byte isMoving;                      // is the motor currently moving
Queue<String> queue(10);            // receive serial queue of commands
char line[MAXCOMMAND];
byte eoc;                           // end of command
byte idx;                           // index into command string
byte displayenabled;                // used to enable and disable the display
byte tprobe1;                       // indicate if there is a probe attached to myFocuserPro2
double ch1tempval;                  // temperature value for probe
double lasttempval;                 // holds previous temperature value - used if ismoving and if temp request < 10s apart
byte motorspeedchange;              // change motorspeed to slow when nearing target position?
byte tswthreshold;                  // number of steps at which stepper slows down as target position is approached
byte movedirection;                 // holds direction of new planned move
int stepperpowerread;
byte stepperpower;
String fastreply;
byte enablestate;

#ifdef TEMPERATUREPROBE
OneWire oneWirech1(TEMPPIN);        // setup temperature probe
DallasTemperature sensor1(&oneWirech1);
DeviceAddress tpAddress;            // holds address of the temperature probe
double starttempval;                // when using temperature compensation, holds the start temperature value
byte tcstarted;                     // indicates if temperature compensation is enabled
long lasttempconversion;            // holds time of last conversion
byte requesttempflag;
#endif


// ----------------------------------------------------------------------------------------------------------
// DO NOT CHANGE ANY OF THE FOLLOWING LINES

// do not change
//#define DEBUG     1
//#define DEBUGHPSW 1


// ----------------------------------------------------------------------------------------------------------
// CODE START
void settimer1()
{
  cli();                                  //stop interrupts
  TCCR1A = 0;                             // set entire TCCR1A register to 0
  TCCR1B = 0;                             // same for TCCR1B
  TCNT1  = 0;                             //initialize counter value to 0
  OCR1A = 8;                              // output compare register - when tcnt1 == ocr then interrupt occurs
  TCCR1B |= (1 << WGM12);                 // turn on CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10);    // Set CS12 and CS10 bits for 1024 prescaler
  TIMSK1 |= (1 << OCIE1A);                // enable timer compare interrupt
  sei();                                  // allow interrupts
}

ISR(TIMER1_COMPA_vect)
{
  motorskip++;
  if ( motorskip < stepontime )
    return;
  else
    motorskip = 0;

  if ( targetPosition != currentPosition )
  {
    if ( enablestate == 0 )                 // if board is not enabled, we need to enable it else it will not step
    {
      enableoutput();                       // have to enable driver board
      writenow = 1;                         // updating of EEPROM ON
    }
    isMoving = 1;
    movestarted = 1;
    if ( motorspeedchange == 1 )
    {
      // Slow down if approaching home position
      long nearinghomepos = currentPosition - targetPosition;
      nearinghomepos = abs(nearinghomepos);
      if ( nearinghomepos < tswthreshold )
      {
        savedmotorSpeed = myfocuser.motorSpeed;
        myfocuser.motorSpeed = SLOW;        // slow
        updatemotorSpeedDelay();
      }
    }
    if (currentPosition > targetPosition)   // Going Anticlockwise to lower position
    {
      if ( stepperpower == 1)
      {
        anticlockwise();
        currentPosition = currentPosition - 1;
        if ( currentPosition < 0 )
          currentPosition = 0;
      }
      else
      {
        targetPosition = currentPosition;     // do not change the position
      }
    }
    else if (currentPosition < targetPosition)    // Going Clockwise to higher position
    {
      if ( stepperpower == 1)
      {
        // do not need to check if > maximumPosition as its done when a target command is receieved
        clockwise();
        currentPosition = currentPosition + 1;
        if ( currentPosition > myfocuser.maxstep )
          currentPosition = myfocuser.maxstep;
      }
      else
      {
        targetPosition = currentPosition;     // do not change the position
      }
    }
  }
  else
  {
    isMoving = 0;
    // reset motorSpeed
    myfocuser.motorSpeed = savedmotorSpeed;         // restore original motorSpeed
    updatemotorSpeedDelay();                        // set the correct delay based on motorSpeed
  }
}

// TonyWilk
inline void asm2uS()  __attribute__((always_inline));

inline void asm2uS()
{
  asm volatile (
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    ::
  );
}

// disable the stepper motor outputs - coil power off
void disableoutput()
{
  digitalWrite(DRV8825ENABLE, HIGH);
  enablestate = 0;
  // control via ENABLE pin, but this turns off indexer inside DRV8825
  // which means the stepper motor will loose power and move position to the nearest full step
  // not an issue if using full steps but major issue if using microstepping as will cause change
  // in focus position
}

// enable the stepper motor outputs - coil power on
void enableoutput()
{
  digitalWrite(DRV8825ENABLE, LOW);
  delay(1);                             // need to wait 1ms before driver chip is ready for stepping
  enablestate = 1;
}

// Move stepper anticlockwise
void anticlockwise()
{
  if ( stepperpower == 1 )
  {
#ifdef INOUTLEDS
    (!myfocuser.ReverseDirection) ? digitalWrite( OUTLED, 1) : digitalWrite( INLED, 1);
#endif
    (!myfocuser.ReverseDirection) ? digitalWrite(DRV8825DIR, LOW ) : digitalWrite(DRV8825DIR, HIGH );
    digitalWrite(DRV8825STEP, 1 );
    asm2uS();
    digitalWrite(DRV8825STEP, 0 );
#ifdef INOUTLEDS
    (!myfocuser.ReverseDirection) ? digitalWrite( OUTLED, 0 ) : digitalWrite( INLED, 0);
#endif
  }
}

void clockwise()
{
  if ( stepperpower == 1 )
  {
#ifdef INOUTLEDS
    (!myfocuser.ReverseDirection) ? digitalWrite( INLED, 1) : digitalWrite( OUTLED, 1);
#endif
    (!myfocuser.ReverseDirection) ? digitalWrite(DRV8825DIR, HIGH ) : digitalWrite(DRV8825DIR, LOW );
    digitalWrite(DRV8825STEP, 1 );
    asm2uS();
    digitalWrite(DRV8825STEP, 0 );
#ifdef INOUTLEDS
    (!myfocuser.ReverseDirection) ? digitalWrite( INLED, 0 ) : digitalWrite( OUTLED, 0);
#endif
  }
}

// set the microstepping mode for DRV8825
void setstepmode( byte stepmode)
{
  switch ( stepmode )
  {
    case STEP1:                         // full step
      digitalWrite(DRV8825M0, 0);
      digitalWrite(DRV8825M1, 0);
      digitalWrite(DRV8825M2, 0);
      break;
    case STEP2:                         // half step
      digitalWrite(DRV8825M0, 1);
      digitalWrite(DRV8825M1, 0);
      digitalWrite(DRV8825M2, 0);
      break;
    case STEP4:                         // quarter step
      digitalWrite(DRV8825M0, 0);
      digitalWrite(DRV8825M1, 1);
      digitalWrite(DRV8825M2, 0);
      break;
    case STEP8:                         // eighth step
      digitalWrite(DRV8825M0, 1);
      digitalWrite(DRV8825M1, 1);
      digitalWrite(DRV8825M2, 0);
      break;
    case STEP16:                        // sixteenth step
      digitalWrite(DRV8825M0, 0);
      digitalWrite(DRV8825M1, 0);
      digitalWrite(DRV8825M2, 1);
      break;
    case STEP32:                        // thirty-two step
      digitalWrite(DRV8825M0, 1);
      digitalWrite(DRV8825M1, 0);
      digitalWrite(DRV8825M2, 1);
      break;
    default:                            // half step
      digitalWrite(DRV8825M0, 1);
      digitalWrite(DRV8825M1, 0);
      digitalWrite(DRV8825M2, 0);
      myfocuser.stepmode = STEP2;
      break;
  }
}

void updatemotorSpeedDelay()
{
  cli();
  switch ( myfocuser.motorSpeed )
  {
    case SLOW:
      switch ( myfocuser.stepmode )
      {
        case STEP1:
          stepontime = SLOW1DELAY;
          break;
        case STEP2:
          stepontime = SLOW2DELAY;
          break;
        case STEP4:
          stepontime = SLOW4DELAY;
          break;
        case STEP8:
          stepontime = SLOW8DELAY;
          break;
        case STEP16:
          stepontime = SLOW16DELAY;
          break;
        case STEP32:
          stepontime = SLOW32DELAY;
          break;
      }
      break;
    case MED:
      switch ( myfocuser.stepmode )
      {
        case STEP1:
          stepontime = MED1DELAY;
          break;
        case STEP2:
          stepontime = MED2DELAY;
          break;
        case STEP4:
          stepontime = MED4DELAY;
          break;
        case STEP8:
          stepontime = MED8DELAY;
          break;
        case STEP16:
          stepontime = MED16DELAY;
          break;
        case STEP32:
          stepontime = MED32DELAY;
          break;
      }
      break;
    case FAST:
      switch ( myfocuser.stepmode )
      {
        case STEP1:
          stepontime = FAST1DELAY;
          break;
        case STEP2:
          stepontime = FAST2DELAY;
          break;
        case STEP4:
          stepontime = FAST4DELAY;
          break;
        case STEP8:
          stepontime = FAST8DELAY;
          break;
        case STEP16:
          stepontime = FAST16DELAY;
          break;
        case STEP32:
          stepontime = FAST32DELAY;
          break;
      }
      break;
  }
  motorskip = 0;
  sei();
}

// reboot the Arduino
void software_Reboot()
{
  asm volatile ( "jmp 0");          // jump to the start of the program
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

// Serial Commands
void processCommand()
{
  byte len;
  byte cmdval;
  long pos;
  int paramval;
  String replystr = "";
  String mycmd = "";
  String param = "";

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
  Serial.print("replystr = "); Serial.println(replystr);
  Serial.print("len = "); Serial.println(len);
  Serial.print("mycmd = "); Serial.println(mycmd);
  Serial.print("param = "); Serial.println(param);
  Serial.print("cmdval = "); Serial.println(cmdval);
#endif
  switch ( cmdval )
  {
    case 0:    // :00#    Pxxxx#    get current focuser position
      replystr = "P" + String(currentPosition) + "#";
      sendresponsestr(replystr);
      break;
    case 1:    // :01#    Ixx#    get motor moving status - 01 if moving, 00 otherwise
      if (isMoving == 1)
        sendresponsestr("I01#");
      else
      {
        if ( movestarted )
        {
          // call asm2us the required amount of times
          // convert DelayAfterMove in milliseconds to microseconds and multiply by 2
          unsigned long tdelay = myfocuser.DelayAfterMove * 500;
          for (unsigned long tp1 = 0; tp1 < tdelay; tp1++)
            asm2uS();
          movestarted = 0;
        }
        sendresponsestr("I00#");
      }
      writenow = 1;             // updating of EEPROM ON
      previousMillis = millis();   // start 30s time interval
      break;
    case 2:     // :02#   EOK#    get motor controller status - Controller Response to "Are we connected"- also see Wifi
      sendresponsestr("EOK#");
      break;
    case 3:     // :03#    Fxxx#   get firmware version string
      replystr = "F" + String(programVersion) + "#";
      sendresponsestr(replystr);
      break;
    case 4:     // :04#     FString#  get firmware version string (Fprogram name, version, #)
      sendresponsestr("F");
      sendresponsestr(String(programName));
      sendresponsenl();
      sendresponsestr(String(programVersion) + "#");
      break;
    case 6: // :06#       Zxxxxxx#  get temperature as a double XXXX
      // Send the command to get temperatures from DS18B20 probe
      // is there a probe?
      // there is not a probe so just return 20
      replystr = "Z" + String(ch1tempval, 3) + "#";
      sendresponsestr(replystr);
      break;
    case 8:   // :08#       Mxxxxxx#  get MaxStep, returns XXXXXX
      replystr = "M" + String(myfocuser.maxstep) + "#";
      sendresponsestr(replystr);
      break;
    case 10: // :10#        Yxxxxxx#  get MaxIncrement, returns xxxxxx
      replystr = "Y" + String(myfocuser.maxstep) + "#";
      sendresponsestr(replystr);
      break;
    case 11:   // :11#        Oxx#    get coil pwr setting (00 = coils released after move, 01 = coil pwr on after move)
      replystr = "O" + String(myfocuser.coilPwr) + "#";
      sendresponsestr(replystr);
      break;
    case 13:  // :13#       Rxx#    get reverse direction setting, 00 off, 01 on
      replystr = "R" + String(myfocuser.ReverseDirection) + "#";
      sendresponsestr(replystr);
      break;
    case 21: // :21#    Qxx#    get temperature probe resolution setting (9, 10, 11 or 12)
#ifdef TEMPERATUREPROBE
      if ( tprobe1 == 1 )
      {
        myfocuser.ds18b20resolution = sensor1.getResolution(tpAddress);
        writenow = 1;             // updating of EEPROM ON
        previousMillis = millis();   // start 30s time interval
      }
#endif
      replystr = "Q" + String(myfocuser.ds18b20resolution) + "#";
      sendresponsestr(replystr);
      break;
    case 24:   // :24#    1x#   get state of Temperature Compensation, 0=disabled, 1=enabled
      replystr = "1" + String(myfocuser.tempcompenabled) + "#";
      sendresponsestr(replystr);
      break;
    case 25:   // :25#    Ax#   get if Temperature Compensation available 0=No, 1=Yes
      replystr = "A0#";
#ifdef TEMPERATUREPROBE
      replystr = "A1#";                    // this focuser supports temperature compensation
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
    case 32:    // :32#   Ux#   get if stepsize is enabled in controller (1 or 0, 0/1)
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
    case 37:    // :37#   Dxx#    get Display status 0=disabled, 1=enabled
      replystr = "D" + String(displayenabled) + "#";
      sendresponsestr(replystr);
      break;
    case 38:  // :38#   Dxx#      Get Temperature mode 0=Celsius, 1=Fahrenheight
      replystr = "b" + String(myfocuser.tempmode) + "#";
      sendresponsestr(replystr);
      break;
    case 39:    // :39#            Nxxxxxx# get the new motor position (target) XXXXXX (not used yet)
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
    case 45:    // :45#         Gxxx#      Return TSWTHRESHOLD - value for which stepper slows down at end of its move
      replystr = "G" + String(tswthreshold) + "#";
      sendresponsestr(replystr);
      break;
    case 47:    // 47#          Gxxx#      Return if motorspeedchange enabled/disabled
      replystr = "J" + String(motorspeedchange) + "#";
      sendresponsestr(replystr);
      break;
    case 49:    // :49#         aXXXXX
      sendresponsestr("ab552efd25e454b36b35795029f3a9ba7#");
      break;
    case 62:    // :62#   Lxx#      get update of position on lcd when moving (00=disable, 01=enable)
      replystr = "L" + String(myfocuser.lcdupdateonmove) + "#";
      sendresponsestr(replystr);
      break;
    case 63:  // :63#   Hxx#      get status of home position switch (0=open, 1=closed)
      replystr = "H00#";
      sendresponsestr(replystr);
      break;
    case 66:   // :66#          Kxx#          Return jogging state enabled/disabled
      replystr = "K" + String(jogging) + "#";
      sendresponsestr(replystr);
      break;
    case 68:   // :68#       Vxx#      Return jogging direction 0=IN, 1=OUT
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
    case 78: // return number of backlash steps IN
#ifdef BACKLASH
      replystr = "6" + String(myfocuser.backlashsteps_in) + "#";
      sendresponsestr(replystr);
#else
      sendresponsestr("60#");
#endif
      break;
    case 80: // return number of backlash steps OUT
#ifdef BACKLASH
      replystr = "7" + String(myfocuser.backlashsteps_out) + "#";
      sendresponsestr(replystr);
#else
      sendresponsestr("70#");
#endif
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
#ifdef STEPPERPWRDETECT
      pinMode( STEPPERDETECTPIN, INPUT );
      stepperpower = (byte) digitalRead( STEPPERDETECTPIN );
      replystr = "9" + String(stepperpower) + "#";
#endif
      sendresponsestr(replystr);
      break;
    case 5:     // :05xxxxxx#     None    set new target position to xxxxxx (and focuser initiates immediate move to xxxxxx)
    case 28:    // :28#       None    home the motor to position 0
      if ( myfocuser.tempcompenabled == 0 )
      {
        if ( stepperpower == 1 )
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
                enableoutput();
                anticlockwise();
                asm2uS();
                asm2uS();
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
              for ( int steps = 0; steps < myfocuser.backlashsteps_in; steps++ )
              {
                enableoutput();
                clockwise();
                asm2uS();
                asm2uS();
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
          cli();
          isMoving = 1;
          targetPosition = newPos;
          sei();
#ifdef LCDDISPLAY
          updatepositionlcd();
#endif
          writenow = 1;             // updating of EEPROM ON
          previousMillis = millis();   // start 30s time interval
        }
      }
      break;
    case 7:       // :07xxxxxx#    None    set MaxStep
      pos = decstr2long(param);
      if ( pos > FOCUSERUPPERLIMIT )            // range check the new value for maxSteps
        pos = FOCUSERUPPERLIMIT;
      if ( pos < FOCUSERLOWERLIMIT )            // avoid setting maxSteps too low
        pos = FOCUSERLOWERLIMIT;
      myfocuser.maxstep = pos;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
      break;
    case 12:   // :12xx#      None    set coil pwr 0=release pwr after move, 1=keep power on after move
      paramval = decstr2int(param);
      myfocuser.coilPwr = (byte) paramval & 0x01;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
      break;
    case 14:    // :14xx#      None    set reverse direction setting 0=normal, 1=reverse
      paramval = decstr2int(param);
      myfocuser.ReverseDirection = (byte) paramval & 0x01;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
      break;
    case 15:  // :15XX#   None    Set MotorSpeed, 00 = Slow, 01 = Med, 02 = Fast
      paramval = decstr2int(param);
      myfocuser.motorSpeed = savedmotorSpeed = (byte) paramval & 0x03;
      updatemotorSpeedDelay();
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
      break;
    case 16:  // :16#     None    Display in Celsius (LCD or TFT)
      myfocuser.tempmode = 1;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
      break;
    case 17:   // :17#    None    Display in Fahrenheit (LCD or TFT)
      myfocuser.tempmode = 0;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
      break;
    case 18:
      // :180#    None    set the return of user specified stepsize to be OFF - default
      // :181#    None    set the return of user specified stepsize to be ON - reports what user specified as stepsize
      paramval = decstr2int(param);
      myfocuser.stepsizeenabled = (byte) paramval & 0x01;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
      break;
    case 19:  // :19xxxx#  None    set the step size value - double type, eg 2.1
      {
        double tempstepsize = (double) param.toFloat();
        if ( tempstepsize < MINSTEPSIZE )
          tempstepsize = DEFAULTSTEPSIZE;       // set default maximum stepsize
        myfocuser.stepsize = tempstepsize;
        writenow = 1;                 // updating of EEPROM ON
        previousMillis = millis();    // start 30s time interval
      }
      break;
    case 20:   // :20xx#    None    set the temperature resolution setting for the DS18B20 temperature probe
      // no probe, set to default
      myfocuser.ds18b20resolution = TEMP_PRECISION;
#ifdef TEMPERATUREPROBE
      paramval = decstr2int(param);   // read precision
      if ( tprobe1 == 1 )
      {
        sensor1.setResolution( tpAddress, paramval );
        myfocuser.ds18b20resolution = (byte) paramval;        // save setting in EEPROM
        writenow = 1;                 // updating of EEPROM ON
        previousMillis = millis();    // start 30s time interval
      }
#endif
      break;
    case 22:  // :22xxx#    None    set the temperature compensation value to xxx
#ifdef TEMPERATUREPROBE
      paramval = decstr2int(param);
      if ( paramval < 0 )
        paramval = 0;
      else if ( paramval > 200 )
        paramval = 200;
      myfocuser.tempcoefficient = (byte) paramval; // save setting in EEPROM
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
#endif
      break;
    case 23:   // :23x#   None    Set the temperature compensation ON (1) or OFF (0)
#ifdef TEMPERATUREPROBE
      paramval = decstr2int(param);
      myfocuser.tempcompenabled = (byte) paramval & 0x01;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
#endif
      break;
    case 27:    // :27#   None    stop a move - like a Halt
      jogging = 0;
      cli();
      isMoving = 0;
      targetPosition = currentPosition;
      sei();
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
      break;
    case 30:    // :30xx#       None        Set stepmode (1=Full, 2=Half, 4=1/4, 8=1/8, 16=1/16, 32=1/32, 64=1/64, 128=1/128)
      paramval = decstr2int(param);
      myfocuser.stepmode = (byte) paramval;
      setstepmode(myfocuser.stepmode);
      updatemotorSpeedDelay();
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
      break;
    case 31:    // :31xxxxxx#     None      Set current motor position to xxxxxx (does not move, updates currentpos and targetpos to xxxxxx)
      pos = decstr2long(param);
      newPos = pos;
      // rangecheck target
      if ( newPos < 0 )
        newPos = 0;
      if ( newPos > myfocuser.maxstep)
        newPos = myfocuser.maxstep;
      cli();
      isMoving = 0;
      currentPosition = targetPosition = newPos;
      sei();
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
      break;
    case 35:  // :35xxxx# None    Set length of time an LCD page is displayed for in milliseconds
      pos = decstr2long(param) / 100;
      if ( pos < DISPLAYPAGETIMEMIN )           // bounds check to 2000-4000 2s-4s
        pos = DISPLAYPAGETIMEMIN;
      if ( pos > DISPLAYPAGETIMEMAX )
        pos = DISPLAYPAGETIMEMAX;
      myfocuser.pagedisplaytime = (byte) pos;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
      break;
    case 36:
      // :360#    None    Disable Display
      // :361#    None    Enable Display
#ifdef LCDDISPLAY
      paramval = decstr2int(param);
      if ( paramval == 0 )
      {
        lcd.noDisplay();
        lcd.noBacklight();
        displayenabled = 0;
      }
      else if ( paramval == 1 )
      {
        lcd.display();
        lcd.backlight();
        displayenabled = 1;
      }
#endif
      break;
    case 40:    // :40#           None          Reset Arduino myFocuserPro2 controller
      software_Reboot();
      break;
    case 44:    // :44xxx#         None         Set motorspeed threshold when moving - switches to slowspeed when nearing destination
      paramval = decstr2int(param);
      if ( paramval < 50 )                      // range check
        paramval = 50;
      else if ( paramval > 200 )
        paramval = 200;
      tswthreshold = (byte) paramval;
      break;
    case 46:    // :46x#           None         Enable/Disable motorspeed change when moving
      paramval = decstr2int(param);
      motorspeedchange = (byte) paramval & 0x01;
      break;
    case 48:     // :48#            None        Save settings to EEPROM
      // copy current settings and write the data to EEPROM
      myfocuser.validdata = VALIDDATAFLAG;
      myfocuser.fposition = currentPosition;
      writeEEPROMNow();
      writenow = 0;
      break;
    case 61:  // :61xx#             None        Set update of position on lcd when moving (00=disable, 01=enable)
      paramval = decstr2int(param);
      myfocuser.lcdupdateonmove = (byte) paramval & 0x01;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
      break;
    case 64:   // :64xxx#           None        Move a specified number of steps
      if ( stepperpower == 1)
      {
        paramval = decstr2int(param);
        newPos = currentPosition + paramval;
        // rangecheck target
        if ( newPos < 0 )
          newPos = 0;
        if ( newPos > myfocuser.maxstep)
          newPos = myfocuser.maxstep;
        cli();
        isMoving = 1;
        targetPosition = newPos;
        sei();
        writenow = 1;                 // updating of EEPROM ON
        previousMillis = millis();    // start 30s time interval
      }
      break;
    case 65:   // :65xx#          None      Set jogging state enable/disable
      if ( stepperpower == 1)
      {
        paramval = decstr2int(param);
        jogging = (byte) paramval & 0x01;
      }
      break;
    case 67:  // :67#             None      Set jogging direction, 0=IN, 1=OUT
      paramval = decstr2int(param);
      joggingDirection = (byte) paramval & 0x01;
      break;
    case 42:  // :42#             None      Reset focuser defaults
      currentaddr = 0;
      setfocuserdefaults();
      // Set focuser defaults.
      cli();
      currentPosition = myfocuser.fposition;
      targetPosition = myfocuser.fposition;
      sei();
      break;
    case 71:  // :71xxx#          None      Set DelayAfterMove in milliseconds
      paramval = decstr2int(param);
      // bounds check to 0-250
      if ( paramval < 0 )
        paramval = 0;
      else if ( paramval > 250 )
        paramval = 250;
      myfocuser.DelayAfterMove = (byte) paramval;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
      break;
    case 73: // Disable/enable backlash IN (going to lower focuser position)
#ifdef BACKLASH
      paramval = decstr2int(param);
      myfocuser.backlash_in_enabled = (byte) paramval & 0x01;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
#endif
      break;
    case 75: // Disable/enable backlash OUT (going to lower focuser position)
#ifdef BACKLASH
      paramval = decstr2int(param);
      myfocuser.backlash_out_enabled = (byte) paramval & 0x01;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
#endif
      break;
    case 77: // set backlash in steps
#ifdef BACKLASH
      paramval = decstr2int(param);
      if ( paramval < 0 )           // range check
        paramval = 0;
      myfocuser.backlashsteps_in = (byte) paramval & 0xff;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
#endif
      break;
    case 79: // set backlash OUT steps
#ifdef BACKLASH
      paramval = decstr2int(param);
      if ( paramval < 0 )           // range check
        paramval = 0;
      myfocuser.backlashsteps_out = (byte) paramval & 0xff;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
#endif
      break;
    case 88:  //:88#          Set temp comp direction 1=IN
      paramval = decstr2int(param);
      myfocuser.tcdirection = (byte) paramval & 0x01;
      writenow = 1;                 // updating of EEPROM ON
      previousMillis = millis();    // start 30s time interval
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
  myfocuser.stepmode = STEP1;                             // full stepping
  myfocuser.pagedisplaytime = DISPLAYPAGETIMEMIN;
  myfocuser.stepsizeenabled = 0;                      // default state is step size OFF
  myfocuser.stepsize = DEFAULTSTEPSIZE;
  myfocuser.tempmode = 1;                              // default is celsius
  myfocuser.ds18b20resolution = TEMP_PRECISION;
  myfocuser.tempcompenabled = 0;
  myfocuser.tempcoefficient = 0;
  myfocuser.lcdupdateonmove = 0;
  myfocuser.DelayAfterMove = 0;
  myfocuser.backlashsteps_in = 0;
  myfocuser.backlashsteps_out = 0;
  myfocuser.focuserdirection = MOVINGIN;
  myfocuser.backlash_in_enabled = 0;
  myfocuser.backlash_out_enabled = 0;
  myfocuser.tcdirection = 1;
  myfocuser.motorSpeed = FAST;
  writeEEPROMNow();                                       // update values in EEPROM
  isMoving = 0;
  movestarted = 0;
}

#ifdef TEMPERATUREPROBE
// find the address of the DS18B20 sensor probe
void findds18b20address()
{
  // look for probes
  // Search the wire for address
  if (sensor1.getAddress(tpAddress, 0))
  {
    tprobe1 = 1;                            // there is a probe1
  }
  // device address is now saved in tpAddress
}
#endif

// Setup
void setup()
{
  int datasize;                             // will hold size of the struct myfocuser - 6 bytes
  int nlocations;                           // number of storage locations available in EEPROM
  bool found;

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

  stepperpower = 1;
#ifdef STEPPERPWRDETECT
  pinMode( STEPPERDETECTPIN, INPUT );
  stepperpowerread = analogRead( STEPPERDETECTPIN );
  (stepperpowerread > 900) ? stepperpower = 1 : stepperpower = 0;
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

  pinMode(  DRV8825DIR, OUTPUT );
  pinMode(  DRV8825STEP, OUTPUT );
  pinMode(  DRV8825M0, OUTPUT );
  pinMode(  DRV8825M1, OUTPUT );
  pinMode(  DRV8825M2, OUTPUT );
  digitalWrite( DRV8825DIR, 0 );            // set direction and step to low
  digitalWrite( DRV8825STEP, 0 );
  pinMode( DRV8825ENABLE, OUTPUT );         // enable the driver board

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
      currentaddr = addr;                   // data was erased so write some default values
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
  movedirection = MOVINGIN;             // start with a known state
#endif
  if (myfocuser.tempcoefficient > 200)
    myfocuser.tempcoefficient = 200;
  myfocuser.motorSpeed = myfocuser.motorSpeed & 0x03;

  if ( myfocuser.coilPwr == 1 )
    enableoutput();
  else
    disableoutput();
  if ( myfocuser.stepmode < STEP1 )         // validate stepmode
    myfocuser.stepmode = STEP1;
  if ( myfocuser.stepmode > STEP32 )
    myfocuser.stepmode = STEP32;
  motorskip = 0;
  savedmotorSpeed = myfocuser.motorSpeed;
  setstepmode(myfocuser.stepmode);
  updatemotorSpeedDelay();

  writenow = 1;                          // ensure validated values are saved
  currentPosition = myfocuser.fposition;    // Set focuser defaults from saved values in EEPROM.
  targetPosition = myfocuser.fposition;
#ifdef DEBUG
  Serial.print("myfocuser.fposition = ");
  Serial.print(myfocuser.fposition);
#endif

#ifdef INOUTLEDS
  digitalWrite( INLED, 0 );                // turn off the IN/OUT LEDS and BUZZER
  digitalWrite( OUTLED, 0 );
#endif

#ifdef BUZZER
  digitalWrite( BUZZERPIN, 0);
#endif

  settimer1();                              // setup timer to handle moving stepper if required
}

// Main Loop
void loop()
{
#ifdef STEPPERPWRDETECT
  stepperpowerread = analogRead( STEPPERDETECTPIN );
  (stepperpowerread > 900) ? stepperpower = 1 : stepperpower = 0;
#endif

  if ( queue.count() >= 1 )                 // check for serial command
  {
    processCommand();
  }

#ifdef PUSHBUTTONS
  if ( stepperpower == 1)
  {
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
              isMoving = 1;
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
  }
#endif

  if ( stepperpower == 1)
  {
    if ( myfocuser.tempcompenabled == 0 )
    {
      if ( jogging == 1 )
      {
        if ( joggingDirection == 0 )
        {
          // move IN
          movedirection = MOVINGIN;
          myfocuser.focuserdirection = movedirection;
          movestarted = 1;
          cli();
          isMoving = 1;
          targetPosition = targetPosition - 1;
          if ( targetPosition < 0 )
            targetPosition = 0;
          sei();
        }
        else
        {
          // move OUT
          movedirection = MOVINGOUT;
          myfocuser.focuserdirection = movedirection;
          movestarted = 1;
          cli();
          isMoving = 1;
          if ( targetPosition > myfocuser.maxstep )
            targetPosition = myfocuser.maxstep;
          sei();
        }
      }
    }
  }

  if ( isMoving == 1 )
  {
    // ignore
  }  // isMoving == 1?
  else
  {
    // focuser is not moving
    // reset motorSpeed
    myfocuser.motorSpeed = savedmotorSpeed;
    updatemotorSpeedDelay();                // set the correct delay based on motorSpeed

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
    if ( stepperpower == 1)
    {
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
          // move the focuser by the required amount, this should move focuser inwards
          newPos = targetPosition - myfocuser.tempcoefficient;      // range check the new position
          if ( newPos < 0 )
            newPos = 0;
          if ( newPos > myfocuser.maxstep )
            newPos = myfocuser.maxstep;
          movestarted = 1;
          cli();
          isMoving = 1;
          targetPosition = newPos;
          sei();
          tcstarted = 0;                  // indicate that temp compensation was done
        } // end of check for tempchange >=1
      } // end of check for tempcompenabled == 1)
    }
#endif

    // is it time to update EEPROM settings?
    if ( writenow == 1 )
    {
      // decide if we have waited 10s (value of EEPROMWRITEINTERVAL) after the last myfocuser key variable update, if so, update the EEPROM
      long currentMillis = millis();
      if ( ((currentMillis - previousMillis) > EEPROMWRITEINTERVAL) || (currentMillis < previousMillis ) )
      {
        myfocuser.validdata = VALIDDATAFLAG;
        myfocuser.fposition = currentPosition;
        writeEEPROMNow();                   // update values in EEPROM
        writenow = 0;
        previousMillis = currentMillis;     // update the timestamp
      }
    }
    if ( myfocuser.coilPwr == 0 )
      disableoutput();                      // release the stepper coils to save power
  }
}

void clearSerialPort()
{
  while (Serial.available())
  {
    Serial.read();
  }
}

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
          writenow = 1;                 // updating of EEPROM ON
          previousMillis = millis();    // start 30s time interval
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

