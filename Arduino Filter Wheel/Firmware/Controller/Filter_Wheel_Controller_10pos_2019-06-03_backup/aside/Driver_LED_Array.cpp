/*------------------------------------------------------------
| Lamp_Controller.cpp           (c) 2012 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE: Basic lamp control functions
|
| DESCRIPTION:
|
| HISTORY:
|   18Jun12	MAO
------------------------------------------------------------*/

#include "Arduino.h"
#include "Pin_Definitions.h"
#include "Driver_LED_Array.h"
#include "Driver_AD5066_Array.h"

//------------------------------------------------------------
//	Constants
//------------------------------------------------------------


//------------------------------------------------------------
//	Global Variables
//------------------------------------------------------------
Driver_LED_Array_Class	Driver_LED_Array;		// Global instantiation of the class


//------------------------------------------------------------
//	Function Bodies
//------------------------------------------------------------


/*------------------------------------------------------------
|  Init
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void	Driver_LED_Array_Class::Init( void ) {

	uint8_t		i, j;

	// config LED power control
	pinMode( PIN_LED_POWER_EN, OUTPUT);		// set the LED power pin to an output
	Set_LED_Power_Enable( false );				// turn off LED power

	// Init the DAC array and define the DAC pins
	uint8_t uPIN_ENABLE[] = { PIN_DAC_SPI_EN0, PIN_DAC_SPI_EN1, PIN_DAC_SPI_EN2, PIN_DAC_SPI_EN3, PIN_DAC_SPI_EN4 };
	uint8_t uPIN_PRESET[] = { PIN_DAC_PRESET0, PIN_DAC_PRESET1, PIN_DAC_PRESET2, PIN_DAC_PRESET3, PIN_DAC_PRESET4 };
	Driver_AD5066_Array.Init( uPIN_ENABLE, uPIN_PRESET, PIN_DAC_LOAD );

	// start on Buffer 0
	uActive_Buffer = 0;

	// Clear the arrays and buffers
	for( i=0; i!=LED_ARRAY_NUM_CHANNELS; i++ ) {

		// Zero the currently active buffer
		uActive_Level[i] = 0;

		// Zero all the buffers
		for( j=0; j!=LED_ARRAY_NUM_BUFFERS; j++ ) {
			uLevel_Buffer[j][i] = 0;
		}
	}

	// Global level defaults
	uGlobal_Level = 65535;

	// Modulation defaults
	fModulation_Level = 1.0;

	bAuto_Output_Update = true;

	// Sequencer defaults
	Clear_Sequence();			// load stop commands into the entire buffer

	// Strobe defaults
	ulStrobe_Period_in_us = 1000;
	tStrobe_Buffer_ID = 1;
}


/*------------------------------------------------------------
|  Set_LED_Power_Enable
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void	Driver_LED_Array_Class::Set_LED_Power_Enable( bool bEnable_param ) {

	bLED_Power_Enable = bEnable_param;

	if ( bLED_Power_Enable ) {
		digitalWrite( PIN_LED_POWER_EN, HIGH );
	} else {
		digitalWrite( PIN_LED_POWER_EN, LOW );
	}
}


/*------------------------------------------------------------
|  Set_Global_Level
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void	Driver_LED_Array_Class::Set_Global_Level( uint16_t uGlobal_Level_param ) {

	uGlobal_Level = uGlobal_Level_param;

	if ( bAuto_Output_Update ) {
		Apply_Buffer_to_Active( uActive_Buffer );
	}
}


/*------------------------------------------------------------
|  Set_Modulation_Level
|-------------------------------------------------------------
|
| PURPOSE:  Direct DAC Access.  These are mainly for debugging
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void	Driver_LED_Array_Class::Set_Modulation_Level( float fModulation_Level_param ) {

	fModulation_Level = fModulation_Level_param;
}

/*------------------------------------------------------------
|  Set_Selected_Buffer
|-------------------------------------------------------------
|
| PURPOSE:  Direct DAC Access.  These are mainly for debugging
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void	Driver_LED_Array_Class::Set_Selected_Buffer( uint8_t uBufferID ) {

	if ( uBufferID < LED_ARRAY_NUM_BUFFERS ) {
		uActive_Buffer = uBufferID;
	}

}


/*------------------------------------------------------------
|  Set_Buffer_Channel_Level
|-------------------------------------------------------------
|
| PURPOSE:  Direct DAC Access.  These are mainly for debugging
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void	Driver_LED_Array_Class::Set_Buffer_Channel_Level( uint8_t uChannel, uint16_t uLevel ) {

	uLevel_Buffer[ uActive_Buffer ][ uChannel ] = uLevel;

	if ( bAuto_Output_Update ) {
		Apply_Buffer_to_Active( uActive_Buffer );
	}
}


/*------------------------------------------------------------
|  Set_Auto_Output_Update
|-------------------------------------------------------------
|
| PURPOSE:  Enable or release output hold
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void	Driver_LED_Array_Class::Set_Auto_Output_Update( bool bAuto_Update ) {

	bAuto_Output_Update = bAuto_Update;

	// if we just released the output hold, update LED in case anything is being held
	if ( bAuto_Output_Update ) {
		Apply_Buffer_to_Active( uActive_Buffer );
//		Write_Active_to_DAC();
	}
}


/*------------------------------------------------------------
|  Apply_Buffer_to_Active
|-------------------------------------------------------------
|
| PURPOSE:  Scale the outputs bu the global brightness and write to the active buffer
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void	Driver_LED_Array_Class::Apply_Buffer_to_Active( uint8_t uBufferID ) {

	uint8_t		i;
	float	uOutput;

	// Loop through all the channels
	for ( i=0; i!=LED_ARRAY_NUM_CHANNELS; i++ ) {

		// scale all the channel levels by the global level
		uOutput = (float)uLevel_Buffer[ uBufferID ][i] * (float)uGlobal_Level;		// both values should be between 0.0 and 1.0
		uOutput /= 65536;

		// limit output to the DAC range
//		fOutput = ( fOutput > AD5066_MAX_OUTPUT_LEVEL ) ? AD5066_MAX_OUTPUT_LEVEL : fOutput;
//		fOutput = ( fOutput < 0.0 ) ? 0.0 : fOutput;

		uActive_Level[i] = (uint16_t)( uOutput );
	}

	// Copy the active to the DACs
//	if ( bAuto_Output_Update ) {
		Write_Active_to_DAC();
//	}
}


/*------------------------------------------------------------
|  Write_Active_to_DAC
|-------------------------------------------------------------
|
| PURPOSE:  Update all the outputs
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void	Driver_LED_Array_Class::Write_Active_to_DAC( void ) {

	uint8_t	i;
	float	fOutput;

	// Update the DAC output buffers
	for ( i=0; i!=LED_ARRAY_NUM_CHANNELS; i++ ) {

		fOutput = uActive_Level[i] * fModulation_Level;
		// Output the active level to the DAC
		Driver_AD5066_Array.Buffer_Set_Single( i, (uint16_t)fOutput );
	}

#if 1
	//Use the LDAC pin to update all DAC outputs at once
	Driver_AD5066_Array.Output_Update_All();

#else

	// Issue commands, one per DAC channel to update DAC outputs one at a time
	for ( i=0; i!=LED_ARRAY_NUM_CHANNELS; i++ ) {
		Driver_AD5066_Array.Output_Update_Single( i );
	}

#endif

}


/*------------------------------------------------------------
|  Write_DAC_Channel_Unbuffered
|-------------------------------------------------------------
|
| PURPOSE:  Direct DAC Access.  These are mainly for debugging
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void	Driver_LED_Array_Class::Write_DAC_Channel_Unbuffered( uint8_t uChannel, uint16_t uLevel ) {

	uActive_Level[uChannel] = uLevel;

	Driver_AD5066_Array.Buffer_Set_Single( uChannel, uLevel );
	Driver_AD5066_Array.Output_Update_Single( uChannel );
}


/*------------------------------------------------------------
|  Set_Sequence_Entry
|-------------------------------------------------------------
|
| PURPOSE:  Set a sequence entry
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void	Driver_LED_Array_Class::Set_Sequence_Entry( tSequence_Entry_t tSequence_Entry, tBuffer_ID_t uBufferID, uint32_t uTimestamp ) {

	if ( tSequence_Entry > LED_ARRAY_NUM_SEQUENCE_BUFFER_LENGTH ) {
		return;
	}

	if ( uBufferID > LED_ARRAY_NUM_BUFFERS ) {
		if ( uBufferID != LED_ARRAY_NUM_SEQUENCE_BUFFER_ID_EOF ) {
			return;
		}
	}

	asEvent_Sequence[ tSequence_Entry ].buffer_ID = uBufferID;
	asEvent_Sequence[ tSequence_Entry ].timestamp = uTimestamp;
}


/*------------------------------------------------------------
|  Clear_Sequence
|-------------------------------------------------------------
|
| PURPOSE:  Erase a sequence
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void	Driver_LED_Array_Class::Clear_Sequence( void ) {

	uint16_t	i;

	for ( i=0; i != LED_ARRAY_NUM_SEQUENCE_BUFFER_LENGTH; i++ ) {
		asEvent_Sequence[ i ].buffer_ID = LED_ARRAY_NUM_SEQUENCE_BUFFER_ID_EOF;
		asEvent_Sequence[ i ].timestamp = 0;
	}
}


/*------------------------------------------------------------
|  Run_Sequence
|-------------------------------------------------------------
|
| PURPOSE:  Set a sequence entry
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void	Driver_LED_Array_Class::Run_Sequence( void ) {

	uint16_t	uSequence_Index;
	uint32_t	ulStart_Time, ulNow;
	uint32_t	ulTimestamp;

	uSequence_Index = 0;

	ulStart_Time = micros();


	// Run the sequence
	while ( ( asEvent_Sequence[ uSequence_Index ].buffer_ID != LED_ARRAY_NUM_SEQUENCE_BUFFER_ID_EOF ) || ( uSequence_Index >= LED_ARRAY_NUM_SEQUENCE_BUFFER_LENGTH ) ) {

		ulNow = micros();

		if ( ulNow >= ( ulStart_Time + asEvent_Sequence[ uSequence_Index ].timestamp ) ) {

			Apply_Buffer_to_Active( asEvent_Sequence[ uSequence_Index ].buffer_ID );
			uSequence_Index++;
		}

	}

}


/*------------------------------------------------------------
|  Fire_Strobe
|-------------------------------------------------------------
|
| PURPOSE:  
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void		Driver_LED_Array_Class::Fire_Strobe( void ) {

	// Apply the strobe buffer immediately
	Apply_Buffer_to_Active( tStrobe_Buffer_ID );

	// Run the timer
	if ( ulStrobe_Period_in_us < 16000) {
		delayMicroseconds( ulStrobe_Period_in_us );
	} else {
		delay( ulStrobe_Period_in_us / 1000 );
	}

	// return to the active buffer once the strobe period is over
	Apply_Buffer_to_Active( uActive_Buffer );

}

/*------------------------------------------------------------
|  Set_Strobe_Params
|-------------------------------------------------------------
|
| PURPOSE:  
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void		Driver_LED_Array_Class::Set_Strobe_Params( tBuffer_ID_t uBufferID, uint32_t uFire_Period ) {

	// Save the buffer ID if it's a valid value
	if ( uBufferID < LED_ARRAY_NUM_BUFFERS ) {
		tStrobe_Buffer_ID = uBufferID;
	}

	// Save the strobe fire period
	ulStrobe_Period_in_us = uFire_Period;
}

