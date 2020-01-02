/*------------------------------------------------------------
| DRIVER_AD5066_ARRAY.h			(c) 2012 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION: See lamp_controller.cpp for more.
|
| HISTORY:
|	18Jun12	MO
------------------------------------------------------------*/

#ifndef FILTER_WHEEL_MOTOR_H
#define FILTER_WHEEL_MOTOR_H

#include "AccelStepper.h"
#include "Pin_Definitions.h"


//#define DEBUG // Define this symbol to enable debug code,
// or comment it out to disable debug code.

// debug output settings
#ifdef	DEBUG
	#define FWM_DEBUG // Define this symbol to enable debug code,
#else
//	#define FWM_DEBUG // Define this symbol to enable debug code,
              // or comment it out to disable debug code.
#endif

#ifdef  FWM_DEBUG
	#if defined(ARDUINO) && ARDUINO >= 100
		#define FWM_DEBUG_MSG_LN(value) Serial.println( F(value) )
		#define FWM_DEBUG_MSG(value) Serial.print( F(value) )
		#define FWM_DEBUG_MSG_VAL(value, format) Serial.print( value, format )
	#else
		#define FWM_DEBUG_MSG_LN(value) Serial.println( (value) )
		#define FWM_DEBUG_MSG(value) Serial.print( (value) )
		#define FWM_DEBUG_MSG_VAL(value, format) Serial.print( value, format )
	#endif
#else
    #define FWM_DEBUG_MSG_LN
    #define FWM_DEBUG_MSG
	#define FWM_DEBUG_MSG_VAL
#endif

// These are normal string-printing macros, not debug output
#if defined(ARDUINO) && ARDUINO >= 100
		#define PWM_MSG(value) Serial.print( F(value) )
		#define PWM_MSG_LN(value) Serial.println( F(value) )
		#define PWM_MSG_VAL(value, format) Serial.print( value, format )
#else
		#define PWM_MSG(value) Serial.print( (value) )
		#define PWM_MSG_LN(value) Serial.println( (value) )
		#define PWM_MSG_VAL(value, format) Serial.print( value, format )
#endif


//------------------------------------------------------------
// Constants
//------------------------------------------------------------

#define FULL_WHEEL_ANGLE		  			360.0f

#define MOTOR_STEPS_PER_REVOLUTION  		200
#define MICROSTEPS  						16
#define MICROSTEPS_PER_REVOLUTION           ( MOTOR_STEPS_PER_REVOLUTION * MICROSTEPS )

#define NORMAL_SPEED  						6000
#define	NORMAL_ACCELERATION					6000
#define	POSITION_SEEK_STEP_SIZE_IN_STEPS	( MICROSTEPS_PER_REVOLUTION / 360 )

#define	HOME_ACCELERATION					1000
#define HOME_SEARCH_SPEED_FAST  			10000
#define HOME_SEARCH_SPEED_SLOW  			500
#define HOME_SEEK_STEP_SIZE_IN_STEPS  		( MICROSTEPS_PER_REVOLUTION / 360 )
//#define HOME_SEEK_STEP_SIZE_IN_DEG  		0.25f
#define HOME_SEEK_STEP_SIZE_IN_DEG  		( 360.0f * HOME_SEEK_STEP_SIZE_IN_STEPS / ( MOTOR_STEPS_PER_REVOLUTION * MICROSTEPS ) )

// Result codes
typedef	enum eFWM_Result {
	FWM_RESULT_SUCCESS,
	FWM_RESULT_HOMING_FAILED,
	FWM_RESULT_INVALID_PARAMETER
} tFWM_Result;

// Motor states
typedef	enum eFWM_State {
	FWM_STATE_STOPPED,
	FWM_STATE_HOMING,
	FWM_RESULT_BLIND_MOVING,
	FWM_RESULT_SEARCHING_FOR_NEXT_POSITION
} tFWM_State;

//------------------------------------------------------------
// Structures
//------------------------------------------------------------



//------------------------------------------------------------
// Class Definition
//------------------------------------------------------------
class	Filter_Wheel_Motor_Class {

	// Public Methods
public:

	tFWM_Result		Init( float fFull_Angle, int8_t iNum_Filters );

	tFWM_Result	Find_Home( void );
	void		Set_Current_Position_As_Home( void );
	bool		Is_Moving( void );
	bool    	Service( void );

	// Global Operations
	tFWM_Result	Set_Target_Filter( int8_t iFilter );
	uint8_t		Get_Current_Filter( void );

	tFWM_Result	Set_Drive_Mode( uint8_t uFilter );
	uint8_t		Get_Drive_Mode( void );

	void		Set_Home_Search_Speed( uint16_t uSpeed );
	uint16_t	Get_Home_Search_Speed( void );
	void		Set_MoveSpeed( uint16_t uSpeed );
	uint16_t	Get_Move_Speed( void );


private:

    uint8_t     Position_To_Filter( long position );
    long    Filter_To_Position( uint8_t filter );

	// private member variables

	long	lSteps_per_Filter;

	float	fFull_Filter_Angle;
	int8_t	iNum_Filters;

	bool	bFound_Home;
	float	fCurrent_Angle;
	int8_t	iCurrent_Filter;
    int8_t	iTarget_Filter;

};

#endif // FILTER_WHEEL_MOTOR_H
