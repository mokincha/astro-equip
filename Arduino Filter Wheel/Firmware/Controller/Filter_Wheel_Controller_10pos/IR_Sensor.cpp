/*------------------------------------------------------------
| IR_Sensor.cpp           ( c ) 2012 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE: IR refelctive sensor
|
| DESCRIPTION:
|
| HISTORY:
|   18Jun12	MAO
------------------------------------------------------------*/

#include "Arduino.h"
#include "Pin_Definitions.h"
#include "IR_Sensor.h"

//------------------------------------------------------------
//	Constants
//------------------------------------------------------------


//------------------------------------------------------------
//	Global Variables
//------------------------------------------------------------


// global min/max values for adaptively setting thresholds
int16_t	g_Sensor_Home_Diff_Max;
int16_t	g_Sensor_Home_Diff_Min;
int16_t	g_Sensor_Position_Diff_Max;
int16_t	g_Sensor_Position_Diff_Min;

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
void	Init_Sensors( void ) {

	// prepare the sensors
	pinMode( PIN_SENSOR_LED, OUTPUT );
	digitalWrite( PIN_SENSOR_LED, LOW );

	// set the global min/max values
	g_Sensor_Home_Diff_Max = -32760;
	g_Sensor_Home_Diff_Min = 32760;
	g_Sensor_Position_Diff_Max = -32760;
	g_Sensor_Position_Diff_Min = 32760;
}


/*------------------------------------------------------------
|  Is_Home_Sensor_Active
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
bool    Is_Home_Sensor_Active( void ) {

	bool    bActive;

	Read_Sensors( &bActive, NULL );

	return  bActive;
}


/*------------------------------------------------------------
|  Is_Position_Sensor_Active
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
bool    Is_Position_Sensor_Active( void ) {

	bool    bActive;

	Read_Sensors( NULL, &bActive );

	return  bActive;
}


/*------------------------------------------------------------
|  Read_Sensors
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void    Read_Sensors( bool *bHome_Sensor_Active, bool *bPosition_Sensor_Active ) {

	uint8_t		i;

	int16_t		iHome_Reading_Dark;
	int16_t		iHome_Reading_Bright;
	int16_t		iHome_Reading_Diff;

	int16_t		iPosition_Reading_Dark;
	int16_t		iPosition_Reading_Bright;
	int16_t		iPosition_Reading_Diff;

	bool		bIs_Home_Sensor_Active;
	bool		bIs_Position_Sensor_Active;

	int16_t		iHome_Threshold;
	int16_t		iPosition_Threshold;

	// reset the accumulators
	iHome_Reading_Dark = 0;
	iHome_Reading_Bright = 0;
	iPosition_Reading_Dark = 0;
	iPosition_Reading_Bright = 0;

	//------------------------------------------------------------
	// Take readings with the LED on and off.  Use the difference to judge reflectance.  This removes temperature effects on output
	// sample the sensors a number of times
	//------------------------------------------------------------
	for ( i = 0; i != FILTER_WHEEL_SENSOR_SAMPLES; i++ ) {

		// turn ON LED power
		digitalWrite( PIN_SENSOR_LED, HIGH );
		delay( LED_LIGHT_DELAY );
		analogRead( PIN_SENSOR_HOME );
		analogRead( PIN_SENSOR_POSITION );
		iHome_Reading_Bright += analogRead( PIN_SENSOR_HOME );
		iPosition_Reading_Bright += analogRead( PIN_SENSOR_POSITION );

		// turn off LED power
		digitalWrite( PIN_SENSOR_LED, LOW );
		delay( LED_LIGHT_DELAY );
		analogRead( PIN_SENSOR_HOME );
		analogRead( PIN_SENSOR_POSITION );
		iHome_Reading_Dark += analogRead( PIN_SENSOR_HOME );
		iPosition_Reading_Dark += analogRead( PIN_SENSOR_POSITION );
	}

#if 0
	IRS_MSG( "HOME: Br:" );
	IRS_MSG_VAL( iHome_Reading_Bright, DEC );
	IRS_MSG( ", dk:" );
	IRS_MSG_VAL( iHome_Reading_Dark, DEC );
	IRS_MSG( ", POS: Br:" );
	IRS_MSG_VAL( iPosition_Reading_Bright, DEC );
	IRS_MSG( ", dk:" );
	IRS_MSG_VAL( iPosition_Reading_Dark, DEC );
	IRS_MSG_LN( "" );
#endif

	//------------------------------------------------------------
	// Compute the differences and scale the reading accumulators to compute the average
	//------------------------------------------------------------
	
	// Note that dark readings are higher than bright readings.  LED light causes the phototransistor to pull down
	iHome_Reading_Diff = ( iHome_Reading_Dark - iHome_Reading_Bright ) / FILTER_WHEEL_SENSOR_SAMPLES;
	iPosition_Reading_Diff = ( iPosition_Reading_Dark - iPosition_Reading_Bright ) / FILTER_WHEEL_SENSOR_SAMPLES;

	// update the global min/max
	g_Sensor_Home_Diff_Max = ( iHome_Reading_Diff > g_Sensor_Home_Diff_Max ) ? iHome_Reading_Diff : g_Sensor_Home_Diff_Max;
	g_Sensor_Home_Diff_Min = ( iHome_Reading_Diff < g_Sensor_Home_Diff_Min ) ? iHome_Reading_Diff : g_Sensor_Home_Diff_Min;
	g_Sensor_Position_Diff_Max = ( iPosition_Reading_Diff > g_Sensor_Position_Diff_Max ) ? iPosition_Reading_Diff : g_Sensor_Position_Diff_Max;
	g_Sensor_Position_Diff_Min = ( iPosition_Reading_Diff < g_Sensor_Position_Diff_Min ) ? iPosition_Reading_Diff : g_Sensor_Position_Diff_Min;

	//------------------------------------------------------------
	// Compute thresholds
	//------------------------------------------------------------
	iHome_Threshold = ( (float)( g_Sensor_Home_Diff_Max - g_Sensor_Home_Diff_Min ) * IRS_HOME_THRESHOLD_FACTOR ) + g_Sensor_Home_Diff_Min;
	iPosition_Threshold = ( (float)( g_Sensor_Position_Diff_Max - g_Sensor_Position_Diff_Min ) * IRS_POSITION_THRESHOLD_FACTOR ) + g_Sensor_Position_Diff_Min;

	// if the min or max HOME sensor readings are too wide or too narrow, then the sensor should be marked inactive
	if ( ( g_Sensor_Home_Diff_Max < SENSOR_HOME_ADAPTIVE_THRESHOLD_MAX_DIFF ) ||
		( g_Sensor_Home_Diff_Min > SENSOR_HOME_ADAPTIVE_THRESHOLD_MIN_DIFF ) ) {

		bIs_Home_Sensor_Active = false;
//		IRS_MSG_LN( "Home not ready" );

	} else {

		// Decide which if home sensor is active
		bIs_Home_Sensor_Active = ( iHome_Reading_Diff > iHome_Threshold );
//		IRS_MSG_LN( "Home NOW ready" );
#if 0
	//------------------------------------------------------------
	// report HOME sensor stats
	//------------------------------------------------------------
	IRS_MSG( "HOME: Br:" );
	IRS_MSG_VAL( iHome_Reading_Bright, DEC );
	IRS_MSG( ", dk:" );
	IRS_MSG_VAL( iHome_Reading_Dark, DEC );
	IRS_MSG_LN( "" );
	IRS_MSG( "HOME: Min:" );
	IRS_MSG_VAL( g_Sensor_Home_Diff_Min, DEC );
	IRS_MSG( ", Max: " );
	IRS_MSG_VAL( g_Sensor_Home_Diff_Max, DEC );
	IRS_MSG( ", Thr: " );
	IRS_MSG_VAL( iHome_Threshold, DEC );
	IRS_MSG( ", Cur:" );
	IRS_MSG_VAL( iHome_Reading_Diff, DEC );
	IRS_MSG( ", Active:" );
	IRS_MSG_VAL( bIs_Home_Sensor_Active, DEC );
//	IRS_MSG_LN( "" );
#endif

	}

	// if the min or max POSITION sensor readings are too wide or too narrow, then the sensor should be marked inactive
	if ( ( g_Sensor_Position_Diff_Max < SENSOR_POSITION_ADAPTIVE_THRESHOLD_MAX_DIFF ) ||
		( g_Sensor_Position_Diff_Min > SENSOR_POSITION_ADAPTIVE_THRESHOLD_MIN_DIFF ) ) {

		bIs_Position_Sensor_Active = false;

	} else {

		// Decide which if position sensor is active
		bIs_Position_Sensor_Active = ( iPosition_Reading_Diff > iPosition_Threshold );
	}

	//------------------------------------------------------------
	//	Report state states
	//------------------------------------------------------------

	// read the home sensor
	if ( bHome_Sensor_Active != NULL ) {

		*bHome_Sensor_Active = bIs_Home_Sensor_Active;
	}

	// read the position sensor
	if ( bPosition_Sensor_Active != NULL ) {

		*bPosition_Sensor_Active = bIs_Position_Sensor_Active;
	}

#if 0
	//------------------------------------------------------------
	// report HOME sensor stats
	//------------------------------------------------------------
	IRS_MSG( "HOME: Min:" );
	IRS_MSG_VAL( g_Sensor_Home_Diff_Min, DEC );
	IRS_MSG( ", Max: " );
	IRS_MSG_VAL( g_Sensor_Home_Diff_Max, DEC );
	IRS_MSG( ", Thr: " );
	IRS_MSG_VAL( iHome_Threshold, DEC );
	IRS_MSG( ", Cur:" );
	IRS_MSG_VAL( iHome_Reading_Diff, DEC );
	IRS_MSG( ", Active:" );
	IRS_MSG_VAL( bIs_Home_Sensor_Active, DEC );
//	IRS_MSG_LN( "" );
#endif

#if 0
	// report POSITION sensor stats
	IRS_MSG( ", POSITION: Min:" );
	IRS_MSG_VAL( g_Sensor_Position_Diff_Min, DEC );
	IRS_MSG( ", Max: " );
	IRS_MSG_VAL( g_Sensor_Position_Diff_Max, DEC );
	IRS_MSG( ", Thr: " );
	IRS_MSG_VAL( iPosition_Threshold, DEC );
	IRS_MSG( ", Cur:" );
	IRS_MSG_VAL( iPosition_Reading_Diff, DEC );
	IRS_MSG( ", Active:" );
	IRS_MSG_VAL( bIs_Position_Sensor_Active, DEC );
	IRS_MSG_LN( "" );
#endif

	// turn off LED power.  Redundant, but safe
	digitalWrite( PIN_SENSOR_LED, LOW );

}


/*------------------------------------------------------------
|  Read_Sensors_Debug
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
void    Read_Sensors_Debug( void ) {

	uint8_t		i;

	int16_t		iHome_Reading_Dark;
	int16_t		iHome_Reading_Bright;
	int16_t		iHome_Reading_Diff;

	int16_t		iPosition_Reading_Dark;
	int16_t		iPosition_Reading_Bright;
	int16_t		iPosition_Reading_Diff;

	bool		bIs_Home_Sensor_Active;
	bool		bIs_Position_Sensor_Active;

	int16_t		iHome_Threshold;
	int16_t		iPosition_Threshold;

	// reset the accumulators
	iHome_Reading_Dark = 0;
	iHome_Reading_Bright = 0;
	iPosition_Reading_Dark = 0;
	iPosition_Reading_Bright = 0;

	//------------------------------------------------------------
	// Take readings with the LED on and off.  Use the difference to judge reflectance.  This removes temperature effects on output
	// sample the sensors a number of times
	//------------------------------------------------------------
	for ( i = 0; i != FILTER_WHEEL_SENSOR_SAMPLES; i++ ) {

		// turn ON LED power
		digitalWrite( PIN_SENSOR_LED, HIGH );
		delay( LED_LIGHT_DELAY );
		analogRead( PIN_SENSOR_HOME );
		analogRead( PIN_SENSOR_POSITION );
		iHome_Reading_Bright += analogRead( PIN_SENSOR_HOME );
		iPosition_Reading_Bright += analogRead( PIN_SENSOR_POSITION );

		// turn off LED power
		digitalWrite( PIN_SENSOR_LED, LOW );
		delay( LED_LIGHT_DELAY );
		analogRead( PIN_SENSOR_HOME );
		analogRead( PIN_SENSOR_POSITION );
		iHome_Reading_Dark += analogRead( PIN_SENSOR_HOME );
		iPosition_Reading_Dark += analogRead( PIN_SENSOR_POSITION );
	}

#if 1
	IRS_MSG( "HOME: Br:" );
	IRS_MSG_VAL( iHome_Reading_Bright, DEC );
	IRS_MSG( ", dk:" );
	IRS_MSG_VAL( iHome_Reading_Dark, DEC );
	IRS_MSG( ", POS: Br:" );
	IRS_MSG_VAL( iPosition_Reading_Bright, DEC );
	IRS_MSG( ", dk:" );
	IRS_MSG_VAL( iPosition_Reading_Dark, DEC );
	IRS_MSG_LN( "" );
#endif

	//------------------------------------------------------------
	// Compute the differences and scale the reading accumulators to compute the average
	//------------------------------------------------------------
	
	// Note that dark readings are higher than bright readings.  LED light causes the phototransistor to pull down
	iHome_Reading_Diff = ( iHome_Reading_Dark - iHome_Reading_Bright ) / FILTER_WHEEL_SENSOR_SAMPLES;
	iPosition_Reading_Diff = ( iPosition_Reading_Dark - iPosition_Reading_Bright ) / FILTER_WHEEL_SENSOR_SAMPLES;

	//------------------------------------------------------------
	// Compute thresholds
	//------------------------------------------------------------
	iHome_Threshold = ( (float)( g_Sensor_Home_Diff_Max - g_Sensor_Home_Diff_Min ) * IRS_HOME_THRESHOLD_FACTOR ) + g_Sensor_Home_Diff_Min;
	iPosition_Threshold = ( (float)( g_Sensor_Position_Diff_Max - g_Sensor_Position_Diff_Min ) * IRS_POSITION_THRESHOLD_FACTOR ) + g_Sensor_Position_Diff_Min;

	// if the min or max HOME sensor readings are too wide or too narrow, then the sensor should be marked inactive
	if ( ( g_Sensor_Home_Diff_Max < SENSOR_HOME_ADAPTIVE_THRESHOLD_MAX_DIFF ) ||
		( g_Sensor_Home_Diff_Min > SENSOR_HOME_ADAPTIVE_THRESHOLD_MIN_DIFF ) ) {

		bIs_Home_Sensor_Active = false;
//		IRS_MSG_LN( "Home not ready" );

	} else {

		// Decide which if home sensor is active
		bIs_Home_Sensor_Active = ( iHome_Reading_Diff > iHome_Threshold );
//		IRS_MSG_LN( "Home NOW ready" );
	}

	// if the min or max POSITION sensor readings are too wide or too narrow, then the sensor should be marked inactive
	if ( ( g_Sensor_Position_Diff_Max < SENSOR_POSITION_ADAPTIVE_THRESHOLD_MAX_DIFF ) ||
		( g_Sensor_Position_Diff_Min > SENSOR_POSITION_ADAPTIVE_THRESHOLD_MIN_DIFF ) ) {

		bIs_Position_Sensor_Active = false;

	} else {

		// Decide which if position sensor is active
		bIs_Position_Sensor_Active = ( iPosition_Reading_Diff > iPosition_Threshold );
	}

#if 1
	//------------------------------------------------------------
	// report HOME sensor stats
	//------------------------------------------------------------
	IRS_MSG( "HOME: Min:" );
	IRS_MSG_VAL( g_Sensor_Home_Diff_Min, DEC );
	IRS_MSG( ", Max: " );
	IRS_MSG_VAL( g_Sensor_Home_Diff_Max, DEC );
	IRS_MSG( ", Thr: " );
	IRS_MSG_VAL( iHome_Threshold, DEC );
	IRS_MSG( ", Cur:" );
	IRS_MSG_VAL( iHome_Reading_Diff, DEC );
	IRS_MSG( ", Active:" );
	IRS_MSG_VAL( bIs_Home_Sensor_Active, DEC );
//	IRS_MSG_LN( "" );

	// report POSITION sensor stats
	IRS_MSG( ", POSITION: Min:" );
	IRS_MSG_VAL( g_Sensor_Position_Diff_Min, DEC );
	IRS_MSG( ", Max: " );
	IRS_MSG_VAL( g_Sensor_Position_Diff_Max, DEC );
	IRS_MSG( ", Thr: " );
	IRS_MSG_VAL( iPosition_Threshold, DEC );
	IRS_MSG( ", Cur:" );
	IRS_MSG_VAL( iPosition_Reading_Diff, DEC );
	IRS_MSG( ", Active:" );
	IRS_MSG_VAL( bIs_Position_Sensor_Active, DEC );
	IRS_MSG_LN( "" );
#endif

	// turn off LED power.  Redundant, but safe
	digitalWrite( PIN_SENSOR_LED, LOW );

}
