/*------------------------------------------------------------
| Lamp_Controller.cpp           ( c ) 2012 Peripheral Vision, Inc.
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
#include "AccelStepper.h"
#include "IR_Sensor.h"

//------------------------------------------------------------
//	Constants
//------------------------------------------------------------


//------------------------------------------------------------
//	Global Variables
//------------------------------------------------------------

AccelStepper motor( AccelStepper::DRIVER, PIN_MOTOR_DRIVER_STEP, PIN_MOTOR_DRIVER_DIR );

tFWM_State	g_State;

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

	// Init the motor state
	g_State = FWM_STATE_STOPPED;

	// Enable the motor
	motor.setEnablePin( PIN_MOTOR_DRIVER_ENABLE );
	motor.setPinsInverted( false, false, true );

	// set default parameters
    motor.setMaxSpeed( NORMAL_SPEED );
    motor.setAcceleration( NORMAL_ACCELERATION );

	// energize coils - the motor will hold position
	motor.enableOutputs();

	FWM_DEBUG_MSG_LN( "FWM Init done" );
	return FWM_RESULT_SUCCESS;
}


/*------------------------------------------------------------
|  Set_Target_Filter
|-------------------------------------------------------------
|
| PURPOSE:  Set the target filter wheel position
|
| DESCRIPTION:	If the target position is different from the current position,
| 				The wheel starts moving.
|
| HISTORY:
|
------------------------------------------------------------*/
tFWM_Result     Filter_Wheel_Motor_Class::Set_Target_Filter( uint8_t uFilter ) {

	tFWM_Result	result;

	long	lTarget_Postion;
	long	lSteps_per_Filter;

	result = FWM_RESULT_SUCCESS;

    // see if we're already moving to the requested filter position
    if ( this->uTarget_Filter == uFilter ) {
        return result;
    }

    // make sure the requested position is valid
    if ( uFilter >= this->uNum_Filters ) {
        return FWM_RESULT_INVALID_PARAMETER;
    }

    // save the new ( valid ) target filter
    this->uTarget_Filter = uFilter;

	//------------------------------------------------------------
	//	Ensure we're homed
	//------------------------------------------------------------

	// first, make sure we know where home is...
	if ( !this->bFound_Home ) {

		result = Find_Home();

		if ( result != FWM_RESULT_SUCCESS ) {
			return result;
		}

		motor.setMaxSpeed( NORMAL_SPEED );
		motor.setAcceleration( NORMAL_ACCELERATION );

	}

	//------------------------------------------------------------
	//	Compute new position
	//------------------------------------------------------------

	FWM_DEBUG_MSG( "Current: " );
	FWM_DEBUG_MSG_VAL( this->uCurrent_Filter, DEC );
	FWM_DEBUG_MSG( ", Target: " );
	FWM_DEBUG_MSG_VAL( this->uTarget_Filter, DEC );
	FWM_DEBUG_MSG_LN( "" );

	// Don't bother wrapping around or trying to optimize for the circular nature of the filter
	// I.e. assume this is linear arrangement

	// move to the desired position
	lSteps_per_Filter = ( ( FULL_WHEEL_ANGLE / 360.0f ) / this->uNum_Filters ) * MICROSTEPS * MOTOR_STEPS_PER_REVOLUTION ;
	lTarget_Postion = (float)(this->uTarget_Filter) * lSteps_per_Filter; 

	FWM_DEBUG_MSG( "Target Steps: " );
	FWM_DEBUG_MSG_VAL( lTarget_Postion, DEC );
	FWM_DEBUG_MSG_LN( "" );

	//------------------------------------------------------------
	//	Start the move
	//------------------------------------------------------------

	// we're moving blindly, hoping to end up in the correct position
	g_State = FWM_RESULT_BLIND_MOVING;

	motor.setMaxSpeed( NORMAL_SPEED );

	// start the moving
	motor.moveTo( lTarget_Postion );

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
|  Service
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
bool    Filter_Wheel_Motor_Class::Service( void ) {

	unsigned wait_time_micros;

	bool  bIs_Moving = false;

	// service the stepper motor
	motor.run();

	switch ( g_State ) {

		//------------------------------------------------------------
		// If the motor is stopped, there's nothing to do.
		//------------------------------------------------------------
		case 	FWM_STATE_STOPPED:
			break;

		//------------------------------------------------------------
		//------------------------------------------------------------
		case	FWM_STATE_HOMING:

			// we should never get here, since homing is a blocking operation
			break;

		//------------------------------------------------------------
		//	Move to where the filter ought to be.  Sometimes we're off because of slipage
		//------------------------------------------------------------
		case 	FWM_RESULT_BLIND_MOVING:

			bIs_Moving = true;


			if ( motor.distanceToGo() == 0 ) {

				// the blind move has completed.  Now we need to verify that the position sensor is active.
				g_State = FWM_RESULT_SEARCHING_FOR_NEXT_POSITION;
			}

			break;

		//------------------------------------------------------------
		//	Make sure we're in the final position by checking that the position sensor is active
		//------------------------------------------------------------
		case 	FWM_RESULT_SEARCHING_FOR_NEXT_POSITION:

#if 0			
			if ( Is_Position_Sensor_Active() ) {

				// the position sensor is active; we're done.  
				bIs_Moving = false;

				// update the motor position, since we may not be where we think we are

			} else {

				// move the wheel a little
				motor.move( POSITION_SEEK_STEP_SIZE_IN_STEPS );

				bIs_Moving = true;

			}
#else
			bIs_Moving = false;
			g_State = FWM_STATE_STOPPED;
#endif
			break;

	}

	return  bIs_Moving;
}


//------------------------------------------------------------
//	Position_To_Filter
// 
//  Convert microsteps to a filter position.  Round to the nearest filter
//------------------------------------------------------------
uint8_t     Filter_Wheel_Motor_Class::Position_To_Filter( long lPosition ) {

    uint8_t		uFilter;
	float 		fSteps_per_Filter = ( ( FULL_WHEEL_ANGLE / 360.0f ) / this->uNum_Filters ) * MICROSTEPS * MOTOR_STEPS_PER_REVOLUTION ;
	uFilter = round( (float)lPosition / fSteps_per_Filter ); 

    // The position is 
    return uFilter;
}


//------------------------------------------------------------
//	Filter_To_Position
// 
//  Convert the filter number to a number of motor microsteps
//------------------------------------------------------------
long    Filter_Wheel_Motor_Class::Filter_To_Position( uint8_t filter ) {

	float 		fSteps_per_Filter = ( ( FULL_WHEEL_ANGLE / 360.0f ) / this->uNum_Filters ) * MICROSTEPS * MOTOR_STEPS_PER_REVOLUTION ;

    return( (long)( filter * fSteps_per_Filter ) ); 
 
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

	int16_t iPosition_Marker_Width;

	tFWM_Result result;

	FWM_DEBUG_MSG_LN( "FWM Home start" );

	// Init the motor state
	g_State = FWM_STATE_HOMING;

	result = FWM_RESULT_SUCCESS;
	// ignore the state of the home flag, since we're going to look for home no matter what

	//------------------------------------------------------------
	//	Start the home search
	//------------------------------------------------------------

	// start moving quickly
	motor.setMaxSpeed( HOME_SEARCH_SPEED_FAST );
	motor.setAcceleration( HOME_ACCELERATION );
	motor.enableOutputs();

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

				motor.setMaxSpeed( HOME_SEARCH_SPEED_SLOW );
				bFast_Search = false;
			}

			// move the wheel a little
//			FWM_DEBUG_MSG_LN( "Moving a little..." );
//			motor.rotate( HOME_SEEK_STEP_SIZE_IN_DEG );
			motor.move( HOME_SEEK_STEP_SIZE_IN_STEPS );
			motor.runToPosition();

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

	//------------------------------------------------------------
	//	Home found.  Now measure the width of the sensor mark and center the wheel on it
	//------------------------------------------------------------
	if ( bHome ) {

		this->fCurrent_Angle = 0.0f;
		this->uCurrent_Filter = 0;

		// now find the width of the position mark in motor steps and position the wheel at the half-way point
		FWM_DEBUG_MSG_LN( "Finding positon mark width." );

		// first, make sure the position sensor is active
		do {
			FWM_DEBUG_MSG_LN( "Waiting for position sensor to be active" );
			Read_Sensors( &bHome_Sensor_Active, &bPosition_Sensor_Active );

			if ( !bPosition_Sensor_Active ) {
				motor.move( 1 );
				motor.runToPosition();
			}

		} while ( !bPosition_Sensor_Active );


		FWM_DEBUG_MSG_LN( "Position sensor is active" );

		// then wait for it to be inactive
		iPosition_Marker_Width = 0;
		do {
			FWM_DEBUG_MSG_LN( "Waiting for position sensor to be inactive" );
			Read_Sensors( &bHome_Sensor_Active, &bPosition_Sensor_Active );

			if ( bPosition_Sensor_Active ) {
				iPosition_Marker_Width++;
				motor.move( 1 );
				motor.runToPosition();
			}

		} while ( bPosition_Sensor_Active );

		FWM_DEBUG_MSG_LN( "Position sensor is inactive" );

		FWM_DEBUG_MSG( "Position mark is " );
		FWM_DEBUG_MSG_VAL( iPosition_Marker_Width, DEC );
		FWM_DEBUG_MSG_LN( " steps wide." );

		iPosition_Marker_Width = -( iPosition_Marker_Width / 2 );
		motor.move( iPosition_Marker_Width );
		motor.runToPosition();

		FWM_DEBUG_MSG( "Moved back " );
		FWM_DEBUG_MSG_VAL( iPosition_Marker_Width, DEC );
		FWM_DEBUG_MSG_LN( " steps." );
	}

	//------------------------------------------------------------
	//	Hold the wheel at the current position
	//------------------------------------------------------------

	// energize coils - the motor will hold position
	motor.enableOutputs();
	motor.setCurrentPosition( 0 );

	FWM_DEBUG_MSG_LN( "FWM Home done" );

	// Update the state.  Since the function is blocking, these state changes are never really visbile
	g_State = FWM_STATE_STOPPED;

	return result;
}




