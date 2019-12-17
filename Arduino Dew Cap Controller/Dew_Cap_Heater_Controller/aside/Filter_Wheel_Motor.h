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

#include "BasicStepperDriver.h"
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
#if 0
#define SENSOR_HOME_THRESHOLD   167
#define SENSOR_POSITION_THRESHOLD   170
#else
#define SENSOR_HOME_THRESHOLD   160
#define SENSOR_POSITION_THRESHOLD   160
#endif

#define HOME_SEARCH_SPEED_FAST  100
#define HOME_SEARCH_SPEED_SLOW  10
#define MOTOR_STEPS  200
#define MICROSTEPS  16

#define HOME_SEEK_STEP_SIZE_IN_STEPS  2
//#define HOME_SEEK_STEP_SIZE_IN_DEG  0.25f
#define HOME_SEEK_STEP_SIZE_IN_DEG  ( 360.0f * HOME_SEEK_STEP_SIZE_IN_STEPS / ( MOTOR_STEPS * MICROSTEPS ) )

#define NORMAL_SPEED  200

typedef	enum eFWM_Result {

	FWM_RESULT_SUCCESS,
	FWM_RESULT_HOMING_FAILED,
	FWM_RESULT_INVALID_PARAMETER

} tFWM_Result;


//------------------------------------------------------------
// Structures
//------------------------------------------------------------



//------------------------------------------------------------
// Class Definition
//------------------------------------------------------------
class	Filter_Wheel_Motor_Class {

	// Public Methods
public:

	tFWM_Result		Init( float fFull_Angle, uint8_t uNum_Filters );
	tFWM_Result		Find_Home( void );
	bool		Is_Moving( void );
  bool    Service( void );

	// Global Operations
	tFWM_Result		Set_Current_Filter( uint8_t uFilter );
	uint8_t		Get_Current_Filter( void );

	tFWM_Result		Set_Drive_Mode( uint8_t uFilter );
	uint8_t		Get_Drive_Mode( void );

	void		Set_Home_Search_Speed( uint16_t uSpeed );
	uint16_t	Get_Home_Search_Speed( void );
	void		Set_MoveSpeed( uint16_t uSpeed );
	uint16_t	Get_Move_Speed( void );

private:

	void	Read_Sensors( bool* bHome_Sensor_Active, bool* bPosition_Sensor_Active );
	bool	Is_Home_Sensor_Active( void );
	bool	Is_Position_Sensor_Active( void );

	// private member variables
	float	fFull_Filter_Angle;
	uint8_t	uNum_Filters;

	bool	bFound_Home;
	float	fCurrent_Angle;
	uint8_t	uCurrent_Filter;

};

#endif // FILTER_WHEEL_MOTOR_H

