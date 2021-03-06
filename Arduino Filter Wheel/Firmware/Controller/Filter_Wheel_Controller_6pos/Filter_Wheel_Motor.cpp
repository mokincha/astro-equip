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
#include "Filter_Wheel_Motor.h"
#include "BasicStepperDriver.h"

//------------------------------------------------------------
//	Constants
//------------------------------------------------------------


//------------------------------------------------------------
//	Global Variables
//------------------------------------------------------------

BasicStepperDriver stepper( MOTOR_STEPS, PIN_MOTOR_DRIVER_DIR, PIN_MOTOR_DRIVER_STEP, PIN_MOTOR_DRIVER_ENABLE );


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
tFWM_Result    Filter_Wheel_Motor_Class::Init( float fFull_Angle, uint8_t uNum_Filters ) {

	FWM_DEBUG_MSG_LN( "FWM Init start" );

	// save passed params
	this->fFull_Filter_Angle = fFull_Angle;
	this->uNum_Filters = uNum_Filters;

	// set up internal variables
	this->bFound_Home = false;
	this->fCurrent_Angle = 0.0f;

	// Enable the stepper
	stepper.begin( NORMAL_SPEED, MICROSTEPS );
	stepper.setEnableActiveState( LOW );
	stepper.setSpeedProfile( stepper.LINEAR_SPEED, 1000, 1000 );

	// energize coils - the motor will hold position
	stepper.enable();
//	stepper.rotate( 90 );

	// prepare the sensors
	pinMode( PIN_SENSOR_LED, OUTPUT );
	digitalWrite( PIN_SENSOR_LED, LOW );

	FWM_DEBUG_MSG_LN( "FWM Init done" );
	return FWM_RESULT_SUCCESS;
}


/*------------------------------------------------------------
|  Find_Home
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
tFWM_Result    Filter_Wheel_Motor_Class::Find_Home( void ) {

	bool    bHome = false;
	bool    bGone_Too_Far = false;
	bool    bHome_Sensor_Active;
	bool    bPosition_Sensor_Active;
	bool    bFast_Search = true;
	float   fSearch_Angle = 0;
	tFWM_Result result;

	FWM_DEBUG_MSG_LN( "FWM Home start" );

	result = FWM_RESULT_SUCCESS;
	// ignore the state of the home flag, since we're going to look for home no matter what

	// start moving quickly
	stepper.setRPM( HOME_SEARCH_SPEED_FAST );
	stepper.enable();

	while ( !bHome && !bGone_Too_Far ) {

//		FWM_DEBUG_MSG_LN( "Not home yet" );

		// check the home & position sensors.  We're home when both are active
		Read_Sensors( &bHome_Sensor_Active, &bPosition_Sensor_Active );

#if 0
		FWM_DEBUG_MSG( "Home: " );
		FWM_DEBUG_MSG_VAL( bHome_Sensor_Active, DEC );
		FWM_DEBUG_MSG( ", Pos: " );
		FWM_DEBUG_MSG_VAL( bPosition_Sensor_Active, DEC );
		FWM_DEBUG_MSG_LN( "" );
#endif

		if ( bPosition_Sensor_Active && bHome_Sensor_Active ) {
			FWM_DEBUG_MSG_LN( "Found home." );
			bHome = true;
		}

		// we didn't find home, so move the wheel a little
		if ( !bHome ) {

			// if the home sensor is active, but the position sensor isn't, we're close.  Slow down
			if ( bHome_Sensor_Active && bFast_Search ) {

				stepper.setRPM( HOME_SEARCH_SPEED_SLOW );
				bFast_Search = false;
			}

			// move the wheel a little
//			FWM_DEBUG_MSG_LN( "Moving a little..." );
			stepper.rotate( HOME_SEEK_STEP_SIZE_IN_DEG );

			// check if we're searched the entire circle
			fSearch_Angle += HOME_SEEK_STEP_SIZE_IN_DEG;

			if ( fSearch_Angle > 360.0f ) {

				FWM_DEBUG_MSG_LN( "Can't find home." );

				bGone_Too_Far = true;
				result = FWM_RESULT_HOMING_FAILED;
			}
		}

	}

	this->bFound_Home = bHome;

	if ( bHome ) {
		this->fCurrent_Angle = 0.0f;
		this->uCurrent_Filter = 0;
	}

  // energize coils - the motor will hold position
  stepper.enable();        
  stepper.setRPM( NORMAL_SPEED );

	FWM_DEBUG_MSG_LN( "FWM Home done" );

	return result;
}



/*------------------------------------------------------------
|  Set_Current_Filter
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
tFWM_Result     Filter_Wheel_Motor_Class::Set_Current_Filter( uint8_t uFilter ) {

	tFWM_Result result;
	int8_t      iSteps_To_Move_Pos;
	int8_t      iSteps_To_Move_Neg;
	int8_t      iSteps_To_Move;

	result = FWM_RESULT_SUCCESS;

	// make sure the requested position is valid
	if ( uFilter >= this->uNum_Filters ) return FWM_RESULT_INVALID_PARAMETER;

	// first, make sure we know where home is...
	if ( !this->bFound_Home ) {

		result = Find_Home();

		if ( result != FWM_RESULT_SUCCESS ) {
			return result;
		}
	}

	// figure out how many filters and which direction to move
	iSteps_To_Move = uFilter - this->uCurrent_Filter;

	FWM_DEBUG_MSG( "Current: " );
	FWM_DEBUG_MSG_VAL( this->uCurrent_Filter, DEC );
	FWM_DEBUG_MSG( ", Target: " );
	FWM_DEBUG_MSG_VAL( uFilter, DEC );
	FWM_DEBUG_MSG_LN( "" );

	// wrap around if the requested filter is more than a half-circle from the current position
	if ( this->fFull_Filter_Angle == 360.0f ) {

		// chose whichever is shorter

		if ( abs( iSteps_To_Move ) > ( this->uNum_Filters / 2 ) ) {

			FWM_DEBUG_MSG_LN( "Wrapping around" );
//			iSteps_To_Move = uFilter - this->uNum_Filters;
			if ( iSteps_To_Move > 0 ) {
				iSteps_To_Move = iSteps_To_Move - this->uNum_Filters;
			} else {
				iSteps_To_Move = iSteps_To_Move + this->uNum_Filters;
			}
		}
	}

	FWM_DEBUG_MSG( "Going to move " );
	FWM_DEBUG_MSG_VAL( iSteps_To_Move, DEC );

	// move to the desired position

	float	fSteps = iSteps_To_Move * ( this->fFull_Filter_Angle / this->uNum_Filters );  
	FWM_DEBUG_MSG( ", Angle: " );
	FWM_DEBUG_MSG_VAL( fSteps, DEC );
	FWM_DEBUG_MSG_LN( "" );

	stepper.rotate( fSteps );

	this->uCurrent_Filter = uFilter;

	return result;
}

/*------------------------------------------------------------
|  Get_Current_Filter
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
uint8_t     Filter_Wheel_Motor_Class::Get_Current_Filter( void ) {

	return this->uCurrent_Filter;

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
void    Filter_Wheel_Motor_Class::Read_Sensors( bool* bHome_Sensor_Active, bool* bPosition_Sensor_Active ) {

	uint16_t    uReading;

	// turn on LED power
	digitalWrite( PIN_SENSOR_LED, HIGH );
	delay( 10 );

	// read the home sensor
	if ( bHome_Sensor_Active != NULL ) {

		uReading = analogRead( PIN_SENSOR_HOME );
		FWM_DEBUG_MSG( "Home: " );
		FWM_DEBUG_MSG_VAL( uReading, DEC );
		*bHome_Sensor_Active = ( uReading < SENSOR_HOME_THRESHOLD );
	}

	// read the position sensor
	if ( bPosition_Sensor_Active != NULL ) {

//		FWM_DEBUG_MSG_LN( "Reading position sensor" );
		uReading = analogRead( PIN_SENSOR_POSITION );
		FWM_DEBUG_MSG( "\tPos: " );
		FWM_DEBUG_MSG_VAL( uReading, DEC );
//		FWM_DEBUG_MSG_LN( "" );
		*bPosition_Sensor_Active = ( uReading < SENSOR_POSITION_THRESHOLD );
	}

	FWM_DEBUG_MSG( "\t" );
	FWM_DEBUG_MSG_VAL( *bHome_Sensor_Active, DEC );
	FWM_DEBUG_MSG( "\t" );
	FWM_DEBUG_MSG_VAL( *bPosition_Sensor_Active, DEC );
	FWM_DEBUG_MSG_LN( "" );

	// turn off LED power
	digitalWrite( PIN_SENSOR_LED, LOW );
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
bool    Filter_Wheel_Motor_Class::Is_Home_Sensor_Active( void ) {

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
bool    Filter_Wheel_Motor_Class::Is_Position_Sensor_Active( void ) {

	bool    bActive;

	Read_Sensors( NULL, &bActive );

	return  bActive;
}




