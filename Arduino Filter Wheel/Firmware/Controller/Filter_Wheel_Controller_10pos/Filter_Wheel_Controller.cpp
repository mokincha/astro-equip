/*------------------------------------------------------------
| Filter_Wheel_Controller.cpp           ( c ) 2019 M. Okincha
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

#if defined( ARDUINO ) && ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

#include <avr/power.h> // Comment out this line for non-AVR boards ( Arduino Due, etc. )


#include "Filter_Wheel_Controller.h"

#include <RotaryEncoder.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
//#include "CommandParser.h"
//#include "Parser.h"
#include "Pin_Definitions.h"
#include "Filter_Wheel_Motor.h"
#include "IR_Sensor.h"


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


//*********************************************************
//  Constants
//*********************************************************
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels



//*********************************************************
//  Class Instances
//*********************************************************

// Declaration for an SSD1306 display connected to I2C ( SDA, SCL pins )
Adafruit_SSD1306 display( SCREEN_WIDTH, SCREEN_HEIGHT, &Wire );

// Setup a RoraryEncoder for pins A2 and A3:
RotaryEncoder encoder( 6, 5 );

Filter_Wheel_Motor_Class  filter_wheel_controller;

//*********************************************************
//  Global Variables
//*********************************************************

// Position tracking variables
int		iEncoder_Last_Position;

int 	iLast_Reported_Position;
int 	iTarget_Position;
int     iCurrent_Position;

// display management
bool  	bUpdate_Display = true;
bool	bDisplay_Ready;

// Sensor readings
uint16_t  uHome_Sensor_Reading;
uint16_t  uPosition_Sensor_Reading;
bool    bIs_Wheel_At_Home = false;
bool    bIs_Wheel_In_POsition = false;


//*********************************************************
//  Functions
//*********************************************************

void ( *resetFunc )( void ) = 0; //declare reset function at address 0


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
    Serial.begin( 9600 );
	Serial.setTimeout( 100 );

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
    pinMode( PIN_KNOB_SWITCH, INPUT_PULLUP );

    //---------------------------------------------
    // Prepare the display
    //---------------------------------------------
    bDisplay_Ready = display.begin( SSD1306_SWITCHCAPVCC, 0x3C );

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
	// Prepare IR sensors
	//---------------------------------------------
	Init_Sensors();

	while ( 0 ) {

		Read_Sensors( NULL, NULL );
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
//        delay( 1000 );

    } else {

		IMA_MSG_LN( "Boot-up homing failed!" );

        Display_Show_Status( DISPLAY_STATUS_DIDNT_FIND_HOME );
//        delay( 1000 );
    }


    IMA_DEBUG_MSG_LN( "FWC motor home done" );


    //---------------------------------------------
    // Set the position variables
    //---------------------------------------------
    iTarget_Position = 0;
    iCurrent_Position = 0;
    iLast_Reported_Position = 0;

    //---------------------------------------------
    // Prepare the rotation knob
    //---------------------------------------------
    encoder.setPosition( 0  );
	iEncoder_Last_Position = encoder.getPosition();


    //---------------------------------------------
    // Update the display
    //---------------------------------------------
    if ( !bDisplay_Ready ) {

        // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
        if ( !display.begin( SSD1306_SWITCHCAPVCC, 0x3C ) ) { // Address 0x3C for 128x64

            Serial.println( F( "SSD1306 allocation failed" ) );
//	      	for( ;; ); // Don't proceed, loop forever
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

    String ASCOMcmd;
	static bool	bWas_Moving = false;

	bool	bHome_Sensor_Active;
	bool	bPosition_Sensor_Active;


#if 0
// service serial commands
    CommandParserService();
#endif

	//---------------------------------------------
	// Reset the MCU if the knob is pressed.  This seems a little extreme...
	//---------------------------------------------
	if ( digitalRead( PIN_KNOB_SWITCH ) == LOW ) {
		//  Call reset
		resetFunc(); 
	}

    //---------------------------------------------
    // Monitor knob
    //---------------------------------------------
    encoder.tick();


    //---------------------------------------------
    // get the current physical position and calc the logical position
    //---------------------------------------------
    if ( iEncoder_Last_Position != encoder.getPosition() ) {

//		Serial.print( "Encoder: " );
//		Serial.println( encoder.getPosition(), DEC );

		iTarget_Position += ( encoder.getPosition() - iEncoder_Last_Position );

		iEncoder_Last_Position = encoder.getPosition();
	}

	//---------------------------------------------
    // Roll over/under knob position
	//---------------------------------------------
    while ( iTarget_Position < 0 ) {
        iTarget_Position += NUM_FILTERS;
	}
    while ( iTarget_Position >= NUM_FILTERS ) {
        iTarget_Position -= NUM_FILTERS;
    }


    //---------------------------------------------
    // Service serial commands
    // 
    // Serial command overrides any position commands from the knob
    //---------------------------------------------
    if ( Serial.available() > 0 ) {

        ASCOMcmd = Serial.readStringUntil( '#' );  // Terminator so arduino knows when the message ends
#if 0
        // Print the command to the LCD
        display.clearDisplay();
        display.setTextSize( 2 ); // Draw 2X-scale text
        display.setTextColor( WHITE );
        display.setCursor( 0, 16 );
        display.print( ASCOMcmd );
        display.display();
#endif
              
        if ( ASCOMcmd == "GETFILTER" ) {
            Serial.print( iLast_Reported_Position );
            Serial.println( "#" );  // Similarly, so ASCOM knows

        } else if ( ASCOMcmd == "FILTER0" ) {
            iTarget_Position = 0;

        } else if ( ASCOMcmd == "FILTER1" ) {
            iTarget_Position = 1;

        } else if ( ASCOMcmd == "FILTER2" ) {
            iTarget_Position = 2;

        } else if ( ASCOMcmd == "FILTER3" ) {
            iTarget_Position = 3;

        } else if ( ASCOMcmd == "FILTER4" ) {
            iTarget_Position = 4;

        } else if ( ASCOMcmd == "FILTER5" ) {
            iTarget_Position = 5;

        } else if ( ASCOMcmd == "FILTER6" ) {
            iTarget_Position = 6;

        } else if ( ASCOMcmd == "FILTER7" ) {
            iTarget_Position = 7;

        } else if ( ASCOMcmd == "FILTER8" ) {
            iTarget_Position = 8;

        } else if ( ASCOMcmd == "FILTER9" ) {
            iTarget_Position = 9;

		} else if ( ASCOMcmd == "R" ) {
			Read_Sensors_Debug();

        }
    }


    //---------------------------------------------
    // Move the wheel if it's not where we want it to be
    //---------------------------------------------
	if ( ( iCurrent_Position != iTarget_Position ) ) {
//    if ( ( iCurrent_Position != iTarget_Position ) && filter_wheel_controller.Is_Moving() ) {

        // request the wheel move to the new position.   This routine is non-blocking
        // since the move takes time.
        filter_wheel_controller.Set_Target_Filter( iTarget_Position );

//		Serial.println( "Starting move" );
    }


    //---------------------------------------------
    //  Service the filter wheel motor task
    //---------------------------------------------
    filter_wheel_controller.Service();


	//---------------------------------------------
	// check if we just stopped moving
	//---------------------------------------------
	if ( bWas_Moving && !filter_wheel_controller.Is_Moving() ) {

//        Serial.println( "Stopped moving" );

		Read_Sensors( &bHome_Sensor_Active, &bPosition_Sensor_Active );

		// Just stopped moving.  Check the position sensor
		if ( !bPosition_Sensor_Active ) {

//			Serial.println( "Out of position.  Homing" );

			// save the current target position
			iTarget_Position = filter_wheel_controller.Get_Current_Filter();

			// We have a problem.  Can't find the position sensor.  Need to home and return to target position
			while ( filter_wheel_controller.Find_Home() != FWM_RESULT_SUCCESS ) {

				Display_Show_Status( DISPLAY_STATUS_DIDNT_FIND_HOME );
		        delay( 500 );
			}

			Display_Show_Status( DISPLAY_STATUS_FOUND_HOME );
	        delay( 500 );

			// go back to the target position
//			Serial.print( "Returning to " );
//			Serial.println( iTarget_Position, DEC );


			filter_wheel_controller.Set_Target_Filter( iTarget_Position );
		} else {

			// Position sensor is active
			if ( bHome_Sensor_Active ) {

//				IMA_MSG_LN( "Home active" );
				 if ( filter_wheel_controller.Get_Current_Filter() != 0 ) {
					
//					IMA_MSG_LN( "Auto-reseting to home position" );
					filter_wheel_controller.Set_Current_Position_As_Home();
				}
			}
		}
	}

	bWas_Moving = filter_wheel_controller.Is_Moving();


    //---------------------------------------------
    //  Report position changes
    //---------------------------------------------
    iCurrent_Position = filter_wheel_controller.Get_Current_Filter();

    // report the current position to the host PC and display
    if ( ( iLast_Reported_Position != iCurrent_Position) && !filter_wheel_controller.Is_Moving() ) {

        // report the current position to the host PC
        Serial.print( iCurrent_Position );
        Serial.println( "#" );      // send ASCOM terminator

		// set the encoder to the current position
//		encoder.setPosition( iCurrent_Position );

        // Update the display
        bUpdate_Display = true;

		// remember what the last reported position was
        iLast_Reported_Position = iCurrent_Position;
    }

    //---------------------------------------------
    // Update the display
    //---------------------------------------------
    if ( bDisplay_Ready && bUpdate_Display && !filter_wheel_controller.Is_Moving() ) {

		// Only update the display if we're not moving, so as to not screw with the stepper timing.
		Display_Show_Position(iTarget_Position);

		bUpdate_Display = false;

    }

}


//---------------------------------------------
// Display current status
//---------------------------------------------
void  Display_Show_Status( tDisplay_Status display_status ) {

    display.clearDisplay();

	display.setTextSize( 3 ); // Draw 2X-scale text
	display.setTextColor( WHITE );
	display.setCursor( 0, 16 );

	switch ( display_status ) {

    case  DISPLAY_STATUS_BOOTING_UP:

        break;

    case DISPLAY_STATUS_SEARCHING_FOR_HOME:

        display.print( F( "Finding" ) );
        display.println( F( " Home" ) );

        break;

    case DISPLAY_STATUS_FOUND_HOME:

        display.clearDisplay();

        display.print( F( "Located" ) );
        display.println( F( " Home" ) );

        break;

    case DISPLAY_STATUS_DIDNT_FIND_HOME:

        display.print( F( " Didn't" ) );
        display.println( F( "Find Home" ) );

        break;

    }

    display.display();
}


//---------------------------------------------
// Display the position
//---------------------------------------------
void  Display_Show_Position( uint8_t  pos ) {

#if 1
    display.clearDisplay();

    display.setTextSize( 3 ); // Draw 3X-scale text
    display.setTextColor( WHITE );
    display.setCursor( 10, 16 );
    display.print( F( "Pos: " ) );
    display.println( pos );

    display.setTextSize( 2 ); // Draw 2X-scale text
    display.setTextColor( WHITE );
    display.setCursor( 10, 45 );

    switch ( pos ) {

		case 0:
			display.println( F( "   Open" ) );
			break;
		case 1:
			display.println( F( "   Dark" ) );
			 break;
		case 2:
			display.println( F( " All Pass" ) );
			break;
		case 3:
			display.println( F( "   Red" ) );
			break;
		case 4:
			display.println( F( "  Green" ) );
			break;
		case 5:
			display.println( F( "   Blue" ) );
			break;
		case 6:
			display.println( F( " H-alpha" ) );
			break;
		case 7:
			display.println( F( " Grating" ) );
			break;
		case 8:
			display.println( F( " ND Moon" ) );
			break;
		case 9:
			display.println( F( "   LPF" ) );
			break;
    }

    display.display();

#endif
}
