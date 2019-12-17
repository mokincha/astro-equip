// Sizeof
// Print size variables and also struct size for myFocuserPro2

#include <Arduino.h>

#define EEPROMSIZE 1024                   // ATMEGA328P 1024 EEPROM

String myVersion="278";

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

int datasize;      // will hold size of the struct myfocuser - 6 bytes
int nlocations;    // number of storage locations available in EEPROM

// Setup
void setup()
{
  char ch;
  int ih;
  long lh;
  float fh;
  double dh;

  // initialize serial
  Serial.begin(9600);

  int sizeofvar = sizeof(ch);
  Serial.print("Sizeof char = "); Serial.println(sizeofvar);

  sizeofvar = sizeof(ih);
  Serial.print("Sizeof int = "); Serial.println(sizeofvar);

  sizeofvar = sizeof(lh);
  Serial.print("Sizeof long = "); Serial.println(sizeofvar);

  sizeofvar = sizeof(fh);
  Serial.print("Sizeof float = "); Serial.println(sizeofvar);

  sizeofvar = sizeof(dh);
  Serial.print("Sizeof double = "); Serial.println(sizeofvar);

  datasize = sizeof( myfocuser );    // should be 40 bytes
  nlocations = EEPROMSIZE / datasize;  // for AT328P = 1024 / datasize = 25 locations

  Serial.println("========================================================================");
  Serial.print("myFocuserPro2 Struct v");
  Serial.println(myVersion);
  Serial.print("Data Size=");
  Serial.println(datasize);
  Serial.print("nlocations=");
  Serial.println(nlocations);
  
  int numberoffocuserwritespersession = 30;
  Serial.println("Assuming "); Serial.print(numberoffocuserwritespersession); Serial.println(" focuser writes per session, ");
  float totalnumberofwritesavailable = nlocations * 10000.0;
  
  // calculate number of imaging sessions available
  float totalnumberofsessions = totalnumberofwritesavailable / numberoffocuserwritespersession;
  Serial.print(totalnumberofsessions); Serial.println(" sessions or nights of imaging.");
  
  float totalnumberofyears = (float) totalnumberofsessions / (float) 365;
  Serial.print("If imaging evernight, then focuser should last approx (years) "); Serial.println(totalnumberofyears);
  
  totalnumberofyears = (float) totalnumberofsessions / (float) 52;
  Serial.print("If imaging once a week, then focuser should last approx (years) "); Serial.println(totalnumberofyears);
}

// Main Loop
void loop()
{
  // do nothing;
}


