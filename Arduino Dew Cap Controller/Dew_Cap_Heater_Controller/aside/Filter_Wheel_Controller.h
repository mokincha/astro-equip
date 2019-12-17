/*------------------------------------------------------------
| Filter_Wheel_Controller.h           (c) 2019 M. Okincha
|-------------------------------------------------------------
|
| PURPOSE: To provide command parser functions.
|
| DESCRIPTION: This is a template application that shows how
| to implement a serial command-line parser.
|
| The constants in this file are intended for use in the
| booth driver code and also in corresponding Matlab
| interfacing code.
|
| HISTORY:
|   20May11 TL from PTC controller driver.
------------------------------------------------------------*/

#ifndef	FILTER_WHEEL_CONTROLLER_H
#define	FILTER_WHEEL_CONTROLLER_H

//#define DEBUG // Define this symbol to enable debug code,
// or comment it out to disable debug code.

// debug output settings
#ifdef	DEBUG
	#define IMA_DEBUG // Define this symbol to enable debug code,
#else
//	#define IMA_DEBUG // Define this symbol to enable debug code,
              // or comment it out to disable debug code.
#endif

#ifdef  IMA_DEBUG
	#if defined(ARDUINO) && ARDUINO >= 100
		#define IMA_DEBUG_MSG_LN(value) Serial.println( F(value) )
		#define IMA_DEBUG_MSG(value) Serial.print( F(value) )
		#define IMA_DEBUG_MSG_VAL(value, format) Serial.print( value, format )
	#else
		#define IMA_DEBUG_MSG_LN(value) Serial.println( (value) )
		#define IMA_DEBUG_MSG(value) Serial.print( (value) )
		#define IMA_DEBUG_MSG_VAL(value, format) Serial.print( value, format )
	#endif
#else
    #define IMA_DEBUG_MSG_LN
    #define IMA_DEBUG_MSG
	#define IMA_DEBUG_MSG_VAL
#endif

// These are normal string-printing macros, not debug output
#if defined(ARDUINO) && ARDUINO >= 100
		#define IMA_MSG(value) Serial.print( F(value) )
		#define IMA_MSG_LN(value) Serial.println( F(value) )
		#define IMA_MSG_VAL(value, format) Serial.print( value, format )
#else
		#define IMA_MSG(value) Serial.print( (value) )
		#define IMA_MSG_LN(value) Serial.println( (value) )
		#define IMA_MSG_VAL(value, format) Serial.print( value, format )
#endif


#define NUM_FILTERS 10

#define	ROTARYSTEPS		1

#define	ROTARYMIN		0
#define	ROTARYMAX		( NUM_FILTERS - 1 )

typedef enum  eDisplay_Status {
  
  DISPLAY_STATUS_BOOTING_UP,
  DISPLAY_STATUS_SEARCHING_FOR_HOME,
  DISPLAY_STATUS_FOUND_HOME,
  DISPLAY_STATUS_DIDNT_FIND_HOME,
  
}  tDisplay_Status;


//------------------------------------------------------------
//	Function Prototypes
//------------------------------------------------------------

// filter movement functions
uint16_t	Wheel_Find_Home( void );
uint16_t	Wheel_Go_To_Filter( uint8_t );

void      Display_Show_Status( tDisplay_Status display_status );
void  Display_Show_Position( uint8_t  pos );

#endif	// FILTER_WHEEL_CONTROLLER_H
