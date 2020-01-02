/*------------------------------------------------------------
| IR_Sensor.h			(c) 2012 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION: See lamp_controller.cpp for more.
|
| HISTORY:
|	18Jun12	MO
------------------------------------------------------------*/

#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include "Pin_Definitions.h"


//#define DEBUG // Define this symbol to enable debug code,
// or comment it out to disable debug code.

// debug output settings
#ifdef	DEBUG
	#define IRS_DEBUG // Define this symbol to enable debug code,
#else
//	#define IRS_DEBUG // Define this symbol to enable debug code,
              // or comment it out to disable debug code.
#endif

#ifdef  IRS_DEBUG
	#if defined(ARDUINO) && ARDUINO >= 100
		#define IRS_DEBUG_MSG_LN(value) Serial.println( F(value) )
		#define IRS_DEBUG_MSG(value) Serial.print( F(value) )
		#define IRS_DEBUG_MSG_VAL(value, format) Serial.print( value, format )
	#else
		#define IRS_DEBUG_MSG_LN(value) Serial.println( (value) )
		#define IRS_DEBUG_MSG(value) Serial.print( (value) )
		#define IRS_DEBUG_MSG_VAL(value, format) Serial.print( value, format )
	#endif
#else
    #define IRS_DEBUG_MSG_LN
    #define IRS_DEBUG_MSG
	#define IRS_DEBUG_MSG_VAL
#endif

// These are normal string-printing macros, not debug output
#if defined(ARDUINO) && ARDUINO >= 100
		#define IRS_MSG(value) Serial.print( F(value) )
		#define IRS_MSG_LN(value) Serial.println( F(value) )
		#define IRS_MSG_VAL(value, format) Serial.print( value, format )
#else
		#define IRS_MSG(value) Serial.print( (value) )
		#define IRS_MSG_LN(value) Serial.println( (value) )
		#define IRS_MSG_VAL(value, format) Serial.print( value, format )
#endif


//------------------------------------------------------------
// Constants
//------------------------------------------------------------

#define	LED_LIGHT_DELAY								10

#define	FILTER_WHEEL_SENSOR_SAMPLES					5

#define	IRS_HOME_THRESHOLD_FACTOR					( 0.7f )
#define	IRS_POSITION_THRESHOLD_FACTOR				( 0.7f )

#define SENSOR_HOME_ADAPTIVE_THRESHOLD_MAX_DIFF				775		// the global min/max difference has to be within this distance from the defaults to be considered valid
#define SENSOR_HOME_ADAPTIVE_THRESHOLD_MIN_DIFF				720		// the global min/max difference has to be at least this distance from the defaults to be considered valid
																	//
#define SENSOR_POSITION_ADAPTIVE_THRESHOLD_MAX_DIFF			850		// the global min/max difference has to be within this distance from the defaults to be considered valid
#define SENSOR_POSITION_ADAPTIVE_THRESHOLD_MIN_DIFF			650		// the global min/max difference has to be at least this distance from the defaults to be considered valid


//------------------------------------------------------------
// Functions
//------------------------------------------------------------

void	Init_Sensors( void );

// should be private.  Temporarily public for debugging of adaptive sensor reading
void    Read_Sensors( bool* bHome_Sensor_Active, bool* bPosition_Sensor_Active );

void	Read_Sensors_Debug( void );

bool   	Is_Home_Sensor_Active( void );
bool   	Is_Position_Sensor_Active( void );

#endif // IR_SENSOR_H
