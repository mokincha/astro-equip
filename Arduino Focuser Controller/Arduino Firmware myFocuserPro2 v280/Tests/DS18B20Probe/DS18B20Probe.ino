// myfocuserPro2 Test example for DS18B20 temperature probe
// (c) Copyright Robert Brown 2014-2016 All Rights Reserved.
// Permission is granted for personal and Academic/Educational use only.

// INSTRUCTIONS
// Ensure Dallas Temperature Library is installed
// This has been tested using Arduino v1.6.4
// Connect probe as per schematic to pin 2, with 4.7K resistor to +5V
// Probe is operated in non-parasitic mode
// Load and run this program
// Use the serial monitor output to view results

#include <Arduino.h>
#include <OneWire.h>                // needed for DS18B20 temperature probe
#include <myDallasTemperature.h>      // needed for DS18B20 temperature probe

// define serial port speed - valid values are 9600, 57600
#define SerialPortSpeed 9600

// DB18B20 temperature probe info
#define ch1temp 2              // temperature probe on pin 2, use 4.7k pullup
// define objects for temperature probes
OneWire oneWirech1(ch1temp);   // setup temperature probe 1
// Pass our oneWire reference to Dallas Temperature library
DallasTemperature sensor1(&oneWirech1); // probe
#define TEMP_PRECISION 10      // Set the DS18B20 precision to 0.25 of a degree 9=0.5, 10=0.25, 11=0.125, 12=0.0625
DeviceAddress tpAddress;       // holds address of the temperature probe
int tprobe1 = 0;               // indicate if there is a probe attached to myFocuserPro2
double ch1tempval;             // temperature value for probe
double lasttempval = 20.0;     // holds previous temperature value - used if ismoving and if temp request < 10s apart
char tempstr[8];

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
  Serial.println();
}

double gettemp()
{
  double result;
  // get channel 1 temperature;
  result = sensor1.getTempCByIndex(0);
  return result;
}

void setds18b20resolution( int pos )
{
  if ( pos < 9 )
  {
    pos = 9;
  }
  if ( pos > 12 )
  {
    pos = 12;
  }
  setds18b20resolution( pos, tpAddress );   // set probe resolution
}

int getds18b20resolution(DeviceAddress probeAddress)
{
  int resolution;
  resolution = sensor1.getResolution(probeAddress);
  return resolution;
}

bool parasitepower()
{
  // report parasite power requirements
  if (sensor1.isParasitePowerMode())
    return true;
  else
    return false;
}

// find the address of the DS18B20 sensor probe
void findds18b20address()
{
  // look for probes
  // Search the wire for address
  if (sensor1.getAddress(tpAddress, 0))
  {
    tprobe1 = 1;  // there is a probe1
  }
}

// set dsb1820 resolution
void setds18b20resolution( int resolution, DeviceAddress address )
{
  // range check
  if ( resolution < 9 )
    resolution = 9;
  if ( resolution > 12 )
    resolution = 12;
  sensor1.setResolution( address, resolution );
}

void setup()
{
  // initialize serial port
  Serial.begin(SerialPortSpeed);

  // start temperature sensor DS18B20
  tprobe1 = 0;                            // set probe indicator NOT FOUND
  sensor1.begin();                        // start the temperature sensor1
  sensor1.getDeviceCount();               // should return 1 if probe connected
  findds18b20address();
  Serial.println("Sensor Address =");
  printAddress(tpAddress);
  Serial.println("Setting precision to 9");
  setds18b20resolution( 9, tpAddress );    // set probe resolution
}

// Main Loop
void loop()
{

  int precision;

  if ( tprobe1 == 1 )
  {
    for ( precision = 9; precision < 13; precision++ )
    {
      Serial.print("Setting precision to = ");
      Serial.println(precision);
      // setds18b20resolution( precision, tpAddress );         // set sensor resolution
      // deviceprecision = getds18b20resolution(tpAddress);    // read what the sensor precision actually is
      // Serial.print("Probe precision is = ");
      // Serial.println(deviceprecision);                      // what it is
      Serial.println("Send request temperature then get temperature");
      setds18b20resolution( precision, tpAddress );   // set probe resolution
      sensor1.requestTemperatures();
      delay(600 / (1 << (12 - precision))); // should enough time to wait
      // get channel 1 temperature, always in celsius
      double result = sensor1.getTempCByIndex(0);
      Serial.print("Raw temperature reading returned = ");
      dtostrf(result, 4, 3, tempstr);
      {
        String tempretval(tempstr);
        Serial.print(tempretval);
      }
      Serial.println("c");
      Serial.println("myFocuser temperature reading would be = ");
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
        lasttempval = ch1tempval;         // remember last reading
      }
      dtostrf(ch1tempval, 4, 3, tempstr);
      {
        String tempretval(tempstr);
        Serial.print(tempretval);
      }
      Serial.println("c");
      Serial.println();
      Serial.println("Delay 2s");
      delay(2000);
    }
  }
  else
  {
    Serial.println("No probe detected");
  }
}

