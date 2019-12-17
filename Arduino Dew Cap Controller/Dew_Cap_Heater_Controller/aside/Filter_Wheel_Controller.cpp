/*------------------------------------------------------------
| Filter_Wheel_Controller.cpp           (c) 2019 M. Okincha
|-------------------------------------------------------------
|
| PURPOSE: Spectralight_III_Booth_Controller
|
| DESCRIPTION: This is a template application that shows how
| to implement a serial command-line parser.
|
| This application is used for the Macbeth booth controller.
|
| HISTORY:
|   13Jun12	MAO	
------------------------------------------------------------*/

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <avr/power.h> // Comment out this line for non-AVR boards (Arduino Due, etc.)


#include "Filter_Wheel_Controller.h"

#include <RotaryEncoder.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
//#include "CommandParser.h"
//#include "Parser.h"
#include "Pin_Definitions.h"
#include "Filter_Wheel_Motor.h"


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire );

// Setup a RoraryEncoder for pins A2 and A3:
RotaryEncoder encoder( 6, 5 );

Filter_Wheel_Motor_Class  filter_wheel_controller;

// Last known rotary position.
int 	lastPos = 0;
int 	newPos;
bool  	bUpdate_Display = true;
bool	bDisplay_Ready;

uint16_t  uHome_Sensor_Reading;
uint16_t  uPosition_Sensor_Reading;
bool    bIs_Wheel_At_Home = false;  
bool    bIs_Wheel_In_POsition = false;  


void(* resetFunc) (void) = 0;//declare reset function at address 0


/*------------------------------------------------------------
| setup
|-------------------------------------------------------------
|
| PURPOSE: To initialize the booth controller application.
|
| DESCRIPTION: In Arduino OS, this routine is called first.
|
| HISTORY:
|   13Jun12	MAO
------------------------------------------------------------*/
void    setup() {

	// Open the serial port, and set the data rate to 9600 bps.
	// This should be done first so that error messages can be
	// seen if device configuration fails.
	Serial.begin( 115200 );

	IMA_DEBUG_MSG_LN( "Setup: start... " );

#if 0

	//---------------------------------------------
	// Configure CLI
	//---------------------------------------------

	// Indicate that the hardware has been initialized and is
	// now ready to respond to commands.
	Command_Line_Interface_Init();

	// Indicate that the hardware has been initialized and is
	// now ready to respond to commands.
	CmdHandler_IDN();

	// set up the command handler
	CommandParserInit( CommandStringList, CommandHandlerTable );
#endif


  //---------------------------------------------
  // Prepare home button
  //---------------------------------------------
  // Setup the button with an internal pull-up :
  pinMode( PIN_KNOB_SWITCH,INPUT_PULLUP);
  
	//---------------------------------------------
	// Prepare the display
	//---------------------------------------------
	bDisplay_Ready = display.begin(SSD1306_SWITCHCAPVCC, 0x3C );

	// wait for the display to wake up...
	delay( 100 );

	if ( bDisplay_Ready ) {

		IMA_DEBUG_MSG_LN( "SSD1306 allocation success" );

		// Update the display
   Display_Show_Status( DISPLAY_STATUS_BOOTING_UP );

	} else {
		IMA_DEBUG_MSG_LN( "Warning: SSD1306 allocation error" );
	}

	//---------------------------------------------
	// prepare the motor
	//---------------------------------------------
	IMA_DEBUG_MSG_LN( "FWC motor start" );
	filter_wheel_controller.Init( 360, NUM_FILTERS );
	IMA_DEBUG_MSG_LN( "FWC motor done" );

	IMA_DEBUG_MSG_LN( "FWC motor home start" );
  Display_Show_Status( DISPLAY_STATUS_SEARCHING_FOR_HOME );
	if ( filter_wheel_controller.Find_Home() == FWM_RESULT_SUCCESS ) {
    
      Display_Show_Status( DISPLAY_STATUS_FOUND_HOME );
      delay( 1000 );
      
	} else {
    
      Display_Show_Status( DISPLAY_STATUS_DIDNT_FIND_HOME );
      delay( 1000 );
	}

  
	IMA_DEBUG_MSG_LN( "FWC motor home done" );

	// Prepare the rotation knob
	encoder.setPosition( 0 / ROTARYSTEPS );	// start with the value of 10.
	newPos = 0;

	// Update the display
	if ( !bDisplay_Ready ) {

		// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
		if ( !display.begin(SSD1306_SWITCHCAPVCC, 0x3C ) ) { // Address 0x3C for 128x64

			Serial.println(F("SSD1306 allocation failed"));
//	      	for(;;); // Don't proceed, loop forever
		}

		// Clear the buffer
		display.clearDisplay();
	}

}


/*------------------------------------------------------------
| loop
|-------------------------------------------------------------
|
| PURPOSE: The main operation loop for the whole application.
|
| DESCRIPTION: In Arduino OS, this routine repeats
| indefinitely.
|
| HISTORY:
|   13Jun12	MAO

------------------------------------------------------------*/
void    loop() {

#if 0
// service serial commands
	CommandParserService();
#endif

  // Monitor knob
	encoder.tick();

  // Get the updated value :
  if ( digitalRead( PIN_KNOB_SWITCH ) == LOW ) {
    
    resetFunc(); //call reset
    
  }
  

	//---------------------------------------------
	// get the current physical position and calc the logical position
	//---------------------------------------------
	newPos = encoder.getPosition();

	if ( newPos < ROTARYMIN ) {

		newPos = ROTARYMAX;

	} else if ( newPos > ROTARYMAX ) {

		newPos = ROTARYMIN;
	} // if


  String ASCOMcmd;
  
  if (Serial.available() >0) {
    ASCOMcmd = Serial.readStringUntil('#');  // Terminator so arduino knows when the message ends
    if (ASCOMcmd=="GETFILTER") {
      Serial.print( lastPos ); 
      Serial.println("#");  // Similarly, so ASCOM knows

    } else if (ASCOMcmd == "FILTER0") {
      newPos = 0;
    } else if (ASCOMcmd == "FILTER1") {
      newPos = 1;
    } else if (ASCOMcmd == "FILTER2") {
      newPos = 2;
    } else if (ASCOMcmd == "FILTER3") {
      newPos = 3;
    } else if (ASCOMcmd == "FILTER4") {
      newPos = 4;
    } else if (ASCOMcmd == "FILTER5") {
      newPos = 5;
    } else if (ASCOMcmd == "FILTER6") {
      newPos = 6;
    } else if (ASCOMcmd == "FILTER7") {
      newPos = 7;
    } else if (ASCOMcmd == "FILTER8") {
      newPos = 8;
    } else if (ASCOMcmd == "FILTER9") {
      newPos = 9;
    } 
  }

  if ( lastPos != newPos ) {

    encoder.setPosition( newPos );
    
    filter_wheel_controller.Set_Current_Filter( newPos );

    bUpdate_Display = true;

    lastPos = newPos;

    Serial.print( lastPos ); 
    Serial.println("#");  // Similarly, so ASCOM knows
  
  } // if

  //  Service the filter wheel motor task
  bool  bDone_Moving;
  
  bDone_Moving = filter_wheel_controller.Service();

#if 0
	//---------------------------------------------
	// Update the display
	//---------------------------------------------
	if ( bDisplay_Ready && bUpdate_Display && bDone_Moving ) {

    Display_Show_Position( newPos );

		bUpdate_Display = false;
	}
#endif

}


void  Display_Show_Status( tDisplay_Status display_status ) {

  display.clearDisplay();

  switch( display_status ) {

    case  DISPLAY_STATUS_BOOTING_UP:

      break;
      
    case DISPLAY_STATUS_SEARCHING_FOR_HOME:

      display.clearDisplay();
  
      display.setTextSize(3); // Draw 2X-scale text
      display.setTextColor( WHITE );
      display.setCursor(0, 16);
      display.print( "Finding" );
      display.println( " Home" );
      display.display();    // Show initial text
      
      break;
      
    case DISPLAY_STATUS_FOUND_HOME:

      display.clearDisplay();
  
      display.setTextSize(3); // Draw 2X-scale text
      display.setTextColor( WHITE );
      display.setCursor(0, 16);
      display.print( "Located" );
      display.println( " Home" );
      display.display();    // Show initial text
      
      break;
      
    case DISPLAY_STATUS_DIDNT_FIND_HOME:

      display.clearDisplay();
  
      display.setTextSize(3); // Draw 2X-scale text
      display.setTextColor( WHITE );
      display.setCursor(0, 16);
      display.print( "  Didn't" );
      display.println( "Find Home" );
      display.display();    // Show initial text
      
      break;
      
    }
  
    display.display();

}


void  Display_Show_Position( uint8_t  pos ) {

  display.clearDisplay();

    display.setTextSize(3); // Draw 2X-scale text
    display.setTextColor( WHITE );
    display.setCursor(10, 16);
    display.print( "Pos: " );
    display.println( pos );

    display.setTextSize( 2 ); // Draw 2X-scale text
    display.setTextColor( WHITE );
    display.setCursor(10, 45);

    switch ( pos ) {
      case 0:
        display.println( "   Open" );
        break;
      case 1:
        display.println( "   Dark" );
        break;
      case 2:
        display.println( " All Pass" );
        break;
      case 3:
        display.println( "   Red" );
        break;
      case 4:
        display.println( "  Green" );
        break;
      case 5:
        display.println( "   Blue" );
        break;
      case 6:
        display.println( "   LPF" );
        break;
      case 7:
        display.println( " Grating" );
        break;
      case 8:
        display.println( " ND Moon" );
        break;
      case 9:
        display.println( "  Aux 1" );
        break;
    }
  
    display.display();

}

