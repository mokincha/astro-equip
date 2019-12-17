/*------------------------------------------------------------
| FILTER_WHEEL_CONTROLLER_H.h			(c) 2012 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION: See Fluorescent_T5HO_Light_Controller.cpp for more.
|
| HISTORY:
|   23Feb12 MO
|	22Mar12	MO Adding state output pins
------------------------------------------------------------*/

#ifndef FILTER_WHEEL_CONTROLLER_H
#define FILTER_WHEEL_CONTROLLER_H

//------------------------------------------------------------
// Compile Options
//------------------------------------------------------------
//#define DEBUG // Define this symbol to enable debug code,
              // or comment it out to disable debug code.


#ifdef __cplusplus
extern "C"
{
#endif


// debug output settings
#ifdef	DEBUG
	#define IMA_DEBUG // Define this symbol to enable debug code,
#else
//	#define IMA_DEBUG // Define this symbol to enable debug code,
              // or comment it out to disable debug code.
#endif

#ifdef  IMA_DEBUG
	#define IMA_DEBUG_MSG_LN(value) Serial.println( F(value) )
	#define IMA_DEBUG_MSG(value) Serial.print( F(value) )
	#define IMA_DEBUG_MSG_VAL(value, format) Serial.print( value, format )
#else
    #define IMA_DEBUG_MSG_LN(value)
    #define IMA_DEBUG_MSG(value)
	#define IMA_DEBUG_MSG_VAL(value, format)
#endif

// These are normal string-printing macros, not debug output
#define IMA_MSG(value) Serial.print( F(value) )
#define IMA_MSG_LN(value) Serial.println( F(value) )
#define IMA_MSG_VAL(value, format) Serial.print( value, format )


#define	FIRMWARE_VERSION	"$Rev: 4651 $"

//------------------------------------------------------------
// Constants
//------------------------------------------------------------

// Hardware Constants
#define	NUMBER_OF_FILTERS		10
#define	FILTER_ANGLE_TOTAL		360


// Derived values
#define	FILTER_ANGLE_BETWEEN_FILTERS	( FILTER_ANGLE_TOTAL / NUMBER_OF_FILTERS )


//------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------


#ifdef __cplusplus
} // extern "C"
#endif

#endif // FILTER_WHEEL_CONTROLLER_H

