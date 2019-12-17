/*------------------------------------------------------------
| Illumination_LED_Spectral_Light_Controller.cpp           (c) 2012 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE: Main application for theLED Spectral Light COntroller
|
| DESCRIPTION: This program controls an array of 20 LED channels.
|	Each channel has a 16 bits DAC.
|
|   extended_fuses = 0x05
|   high_fuses = 0xDE
|   low_fuses = 0xff
|
| HISTORY:
|   15Feb11 TL - original sample application
|	04Jun12 MAO - fleshing out the actual application
------------------------------------------------------------*/

#include "Arduino.h"

#include <avr/wdt.h>

#include "Illumination_LED_Spectral_v2.h"
#include "Command_Line_Interface.h"
#include "CommandParser.h"
#include "Parser.h"
#include "Driver_LED_array.h"
#include "EEPROM_Memory_Map.h"
#include "EEPROM.h"
#include "Pin_Definitions.h"
#include "Lamp_Controller.h"

extern	Driver_LED_Array_Class	Driver_LED_Array;		// Global instantiation of the class


volatile bool	g_bStrobe_Input_Needs_Resetting;

void	isr( void ) {

	Driver_LED_Array.Fire_Strobe();

}


/*------------------------------------------------------------
| Setup
|-------------------------------------------------------------
|
| PURPOSE: To initialize the system application.
|
| DESCRIPTION: In Arduino OS, this routine is called first.
|
| HISTORY:
|   05Feb11 TL
|	19Jan12 MO - Adding I2C master commands
------------------------------------------------------------*/
void setup() {

	// Open the serial port, and set the data rate to 9600 bps.
	// This should be done first so that error messages can be
	// seen if device configuration fails.
	Serial.begin(115200);

	IMA_DEBUG_MSG_LN( "Setup: start... " );

	// Indicate that the hardware has been initialized and is
	// now ready to respond to commands.
//	CmdHandler_IDN();

	// init the main application
	Driver_LED_Array.Init();

	Command_Line_Interface_Init();

	// init the main application
	Lamp_Init();

	// Hook the LED strobe function to the strobe input pin
	pinMode( PIN_STROBE_SENSE, INPUT_PULLUP );
    attachInterrupt( digitalPinToInterrupt(PIN_STROBE_SENSE), isr, FALLING );

	// Set up the defaults so strobes work right away
	Driver_LED_Array.Set_Strobe_Params( 15, 1000 );
	uAuto_Update_State = Driver_LED_Array.Get_Auto_Output_Update();
	Driver_LED_Array.Set_Auto_Output_Update( false );
	temp_buffer_ID = Driver_LED_Array.Get_Selected_Buffer();
	Driver_LED_Array.Set_Selected_Buffer( 15 );
	for( i=0; i!=Driver_LED_Array.Get_Channel_Count(); i++ ) {
		Driver_LED_Array.Set_Buffer_Channel_Level( i, 65535 );
	}
	Driver_LED_Array.Set_Selected_Buffer( temp_buffer_ID );
	Driver_LED_Array.Set_Auto_Output_Update( uAuto_Update_State );

	Driver_LED_Array.Set_LED_Power_Enable( true );				// turn on LED power

	IMA_DEBUG_MSG_LN( "Setup: done. " );
}

/*------------------------------------------------------------
| Loop
|-------------------------------------------------------------
|
| PURPOSE: The main operation loop for the whole application.
|
| DESCRIPTION: In Arduino OS, this routine repeats
| indefinitely.
|
| HISTORY:
|   05Feb11 TL
|   18Feb11 Added scroll-back buffer, prompt cursor.
|   21Feb11 Added DoMonochromator_BUTTONs().
|   25Feb11 Added command-specific parse error reporting.
------------------------------------------------------------*/
void    loop() {

	Command_Line_Interface_Service();

	Lamp_Service();

	Driver_LED_Array.Write_Active_to_DAC();

}


