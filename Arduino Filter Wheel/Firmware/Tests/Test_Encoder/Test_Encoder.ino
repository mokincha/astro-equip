// -----
// LimitedRotator.ino - Example for the RotaryEncoder library.
// This class is implemented for use with the Arduino environment.
// Copyright (c) by Matthias Hertel, http://www.mathertel.de
// This work is licensed under a BSD style license. See http://www.mathertel.de/License.aspx
// More information on: http://www.mathertel.de/Arduino
// -----
// 26.03.2017 created by Matthias Hertel
// -----

// This example checks the state of the rotary encoder in the loop() function.
// The current position is printed on output when changed.
// In addition to the SimplePollRotator example here the range of the rotator is limited to the range 0 - 16 and only incremental steps of 2 are realized.
// To implement this limit the boundaries are checked and eventually the current position is adjusted.
// The internal (physical) position of the rotary encoder library remains by stepping with the increment 1
// so the the logical position is calculated by applying the ROTARYSTEPS factor.

// Hardware setup:
// Attach a rotary encoder with output pins to A2 and A3.
// The common contact should be attached to ground.

#include <RotaryEncoder.h>
#include "BasicStepperDriver.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define RPM 20
#define MOTOR_STEPS 200

#define ROTARYSTEPS 1
#define ROTARYMIN 0
#define ROTARYMAX 9


// Position sensor pins
#define PIN_SENSOR_LED          A2
#define PIN_SENSOR_POSITION     A0
#define PIN_SENSOR_HOME         A1

#if 0
#define SENSOR_HOME_THRESHOLD   167
#define SENSOR_POSITION_THRESHOLD   170
#else
#define SENSOR_HOME_THRESHOLD   155
#define SENSOR_POSITION_THRESHOLD   160
#endif

#define HOME_SEARCH_SPEED_FAST  100
#define HOME_SEARCH_SPEED_SLOW  10
#define HOME_SEEK_STEP_SIZE_IN_DEG  0.25f

// All the wires needed for full functionality
#define PIN_DIR     7
#define PIN_STEP    2
//Uncomment line to use enable/disable functionality
#define PIN_ENABLE  4

#define MICROSTEPS  16
#define NORMAL_SPEED  200

//Uncomment line to use enable/disable functionality
BasicStepperDriver stepper( MOTOR_STEPS, PIN_DIR, PIN_STEP, PIN_ENABLE );

// Setup a RoraryEncoder for pins A2 and A3:
RotaryEncoder encoder( 6, 5 );

// Last known rotary position.
int lastPos = 0;
int newPos;
bool  bUpdate_Display = true;


uint16_t  uHome_Sensor_Reading;
uint16_t  uPosition_Sensor_Reading;
bool      bIs_Wheel_At_Home = false;  
bool      bIs_Wheel_In_POsition = false;  

void setup() {

  bool  bDisplay_Ready = false;
  
  Serial.begin( 115200 );
  Serial.println("Filter Wheel Controller");

#if 0
    while ( !display.begin(SSD1306_SWITCHCAPVCC, 0x3C ) ) { // Address 0x3C for 128x64
      Serial.println(F("Warning: SSD1306 allocation error"));
    }

    {
    bDisplay_Ready = true;
    Serial.println(F("SSD1306 allocation success"));

    // Update the display
    display.clearDisplay();
    
    display.setTextSize(3); // Draw 2X-scale text
    display.setTextColor( WHITE );
    display.setCursor(10, 16);
    display.print( "Finding" );
    display.println( " Home" );
  }
#endif
    
  stepper.begin(RPM, MICROSTEPS);
  
  // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
  stepper.setEnableActiveState(LOW);
  stepper.setSpeedProfile( stepper.LINEAR_SPEED, 1000, 1000 );
    
  encoder.setPosition( 0 / ROTARYSTEPS ); // start with the value of 10.

      // energize coils - the motor will hold position
    stepper.enable();

    newPos = 0;

    // Search for home
    bool  Is_At_Position_0 = false;
    bool  bIs_Near_Home = false;
    bool  bIs_At_Position = false;

    // turn on the LED
    pinMode( PIN_SENSOR_LED, OUTPUT );
    digitalWrite( PIN_SENSOR_LED, HIGH );
    delay( 100 );
    
    // start moving quickly
    stepper.setRPM( HOME_SEARCH_SPEED_FAST );

    Serial.println( "Starting home seek." );
    
#if 1
    while( Is_At_Position_0 == false ) {
      
      uHome_Sensor_Reading = analogRead( PIN_SENSOR_HOME );
      bIs_Wheel_At_Home = ( uHome_Sensor_Reading < SENSOR_HOME_THRESHOLD ) ? true : false;
//      Serial.print( "Home Reading: " );
//      Serial.print( uHome_Sensor_Reading );

      if ( bIs_Wheel_At_Home && !bIs_Near_Home ) {
        
        // switch to low speed to zero in on the final position
        stepper.setRPM( HOME_SEARCH_SPEED_SLOW );
        Serial.println( "Near home.  switching to low speed" );
        bIs_Near_Home = true;
      }
      
      uPosition_Sensor_Reading = analogRead( PIN_SENSOR_POSITION );    
//      Serial.print( "\tPosition Reading: " );
//      Serial.println( uPosition_Sensor_Reading );
      
      bIs_At_Position = ( uPosition_Sensor_Reading < SENSOR_POSITION_THRESHOLD ) ? true : false;

      if ( bIs_At_Position && bIs_Wheel_At_Home ) {
        
        Serial.println( "Found home.  Exiting" );
        Is_At_Position_0 = true;
        
      } else {
        // move the wheel a little
//        Serial.println( "Moving wheel a little" );
        stepper.rotate( HOME_SEEK_STEP_SIZE_IN_DEG );
      }
    }    
    Serial.println( "Home seek complete" );
    digitalWrite( PIN_SENSOR_LED, LOW );
    
#endif

  if ( !bDisplay_Ready) {
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if( !display.begin(SSD1306_SWITCHCAPVCC, 0x3C ) ) { // Address 0x3C for 128x64
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }
  
    // Clear the buffer
    display.clearDisplay();
  }
    // energize coils - the motor will hold position
  stepper.enable();        
  stepper.setRPM( NORMAL_SPEED );
  

} // setup()


// Read the current position of the encoder and print out when changed.
void loop()
{

  
  encoder.tick();

  // get the current physical position and calc the logical position
  newPos = encoder.getPosition() * ROTARYSTEPS;

  if (newPos < ROTARYMIN) {
    encoder.setPosition(ROTARYMIN / ROTARYSTEPS);
    newPos = ROTARYMIN;

  } else if (newPos > ROTARYMAX) {
    encoder.setPosition(ROTARYMAX / ROTARYSTEPS);
    newPos = ROTARYMAX;
  } // if

  if (lastPos != newPos) {
    Serial.print(newPos);
    Serial.println();

    /*
     * Moving motor to original position using steps
     */
     if ( newPos > lastPos ) {
      
        stepper.rotate( ( 360 / ( ROTARYMAX + 1 ) ) );
        
     } else if ( newPos < lastPos ) {
      
        stepper.rotate( ( -360 / ( ROTARYMAX + 1 ) ) );
     }

     bUpdate_Display = true;

     lastPos = newPos;

  } // if

  if ( bUpdate_Display ) {

    // Update the display
    display.clearDisplay();
    
    display.setTextSize(3); // Draw 2X-scale text
    display.setTextColor( WHITE );
    display.setCursor(10, 16);
    display.print( "Pos: " );
    display.println( newPos );

    display.setTextSize( 2 ); // Draw 2X-scale text
    display.setTextColor( WHITE );
    display.setCursor(10, 45);
    switch( newPos ) {
      case 0:
        display.println( " Open" );
        break;
      case 1:
        display.println( " Dark" );
        break;
      case 2:
        display.println( " Red" );
        break;
      case 3:
        display.println( "Green" );
        break;
      case 4:
        display.println( " Blue" );
        break;
      case 5:
        display.println( "Aux 1" );
        break;
      case 6:
        display.println( "Aux 2" );
        break;
      case 7:
        display.println( "Aux 3" );
        break;
      case 8:
        display.println( " LPF" );
        break;
      case 9:
        display.println( "Grating" );
        break;
      
    }
    display.display();      // Show initial text

    bUpdate_Display = false;
  }
      
} // loop ()



// The End

