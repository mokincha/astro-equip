/*------------------------------------------------------------
| Filter_Wheel_Motor.cpp           ( c ) 2019 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE: Control filter wheel stepper motor
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
#define	FWM_USE_WRAP_AROUND			1

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
tFWM_Result    Filter_Wheel_Motor_Class::Init( float fFull_Angle, int8_t iNum_Filters ) {

	FWM_DEBUG_MSG_LN( "FWM Init start" );

	// save passed params
	this->fFull_Filter_Angle = fFull_Angle;
	this->iNum_Filters = iNum_Filters;

	// set up internal variables
	this->bFound_Home = false;
	this->fCurrent_Angle = 0.0f;
	this->lSteps_per_Filter = ( ( FULL_WHEEL_ANGLE / 360.0f ) / this->iNum_Filters ) * MICROSTEPS_PER_REVOLUTION;

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
tFWM_Result     Filter_Wheel_Motor_Class::Set_Target_Filter( int8_t iFilter ) {

	tFWM_Result	result;
	int8_t	iFilter_Positions_to_Move_CW;
	long	lTarget_Postion;

	result = FWM_RESULT_SUCCESS;

    // see if we're already moving to the requested filter position
    if ( this->iTarget_Filter == iFilter ) {
		// already heading to this position.  No need to do anythning different
        return result;
    }

    // make sure the requested position is valid
    if ( iFilter >= this->iNum_Filters ) {
        return FWM_RESULT_INVALID_PARAMETER;
    }

    // save the new ( valid ) target filter
    this->iTarget_Filter = iFilter;

	//------------------------------------------------------------
	//	Ensure we're homed
	//------------------------------------------------------------
	if ( !this->bFound_Home ) {

		// try to find home again
		result = Find_Home();

		// exiting and report if any errors
		if ( result != FWM_RESULT_SUCCESS ) { 
			return result; 
		}
	}

	//------------------------------------------------------------
	//	Compute new position
	//------------------------------------------------------------

	FWM_DEBUG_MSG( "Current: " );
	FWM_DEBUG_MSG_VAL( this->iCurrent_Filter, DEC );
	FWM_DEBUG_MSG( ", Target: " );
	FWM_DEBUG_MSG_VAL( this->iTarget_Filter, DEC );
	FWM_DEBUG_MSG_LN( "" );

#if	FWM_USE_WRAP_AROUND == 0
	// Don't bother wrapping around or trying to optimize for the circular nature of the filter
	// I.e. assume the filters are a linear arrangement such that you can't go past the 0 or 9 positions

	// move to the desired position
	lTarget_Postion = (float)(this->iTarget_Filter) * this->lSteps_per_Filter; 
#else
	// Wrap around.  I.e. rotate in whichever direction gets you to the final position faster

	// First, decide which direction is faster
	// check CW (normal) direction
	iFilter_Positions_to_Move_CW = this->iTarget_Filter - this->iCurrent_Filter;

	if ( abs ( iFilter_Positions_to_Move_CW ) <= ( this->iNum_Filters / 2 ) ) {

		// the normal move is the shortest path.  Move as normal
		lTarget_Postion = (float)(this->iTarget_Filter) * this->lSteps_per_Filter; 

	} else {

		// the normal move is the long way.  Compute a short-cut
		if ( iFilter_Positions_to_Move_CW >= 0 ) {

			// if the normal move is positive, move negative
			lTarget_Postion = (float)(this->iTarget_Filter - this->iNum_Filters ) * this->lSteps_per_Filter; 

		} else {

			// normal move is negative, so move positive
			lTarget_Postion = (float)(this->iTarget_Filter + this->iNum_Filters ) * this->lSteps_per_Filter; 

		}
	}

#endif

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

	this->iCurrent_Filter = iFilter;

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

#if 0
	return this->iCurrent_Filter;
#else
	return Position_To_Filter( motor.currentPosition() );
#endif
}


/*------------------------------------------------------------
|  Is_Moving
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION:
|
| HISTORY:
|
------------------------------------------------------------*/
bool	Filter_Wheel_Motor_Class::Is_Moving( void ) {
	return ( motor.distanceToGo() != 0 );
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


	// Unwind the position if we're no longer moving
	if ( motor.distanceToGo() == 0 ) {

		// unwrap the current position around, in case we were taking a short-cut
		while ( motor.currentPosition() < 0 ) {
			// unwind the position one full CCW revolution
			motor.setCurrentPosition( motor.currentPosition() + MICROSTEPS_PER_REVOLUTION );
		}

		while ( motor.currentPosition() >= MICROSTEPS_PER_REVOLUTION ) {
			// unwind the position one full CW revolution
			motor.setCurrentPosition( motor.currentPosition() - MICROSTEPS_PER_REVOLUTION );
		}

	}

	// Service the state machine.  Not sure why is is required anymore...
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
	float 		fSteps_per_Filter = ( ( FULL_WHEEL_ANGLE / 360.0f ) / this->iNum_Filters ) * MICROSTEPS * MOTOR_STEPS_PER_REVOLUTION ;

	// unwrap the current position around, in case we were taking a short-cut
	while ( lPosition < 0 ) {
		// unwind the position one full CCW revolution
		lPosition += MICROSTEPS_PER_REVOLUTION;
	}

	while ( lPosition >= MICROSTEPS_PER_REVOLUTION ) {
		// unwind the position one full CW revolution
		lPosition -= MICROSTEPS_PER_REVOLUTION;
	}

	uFilter = floor( (float)lPosition / fSteps_per_Filter ); 

    // The position is 
    return uFilter;
}


//------------------------------------------------------------
//	Filter_To_Position
// 
//  Convert the filter number to a number of motor microsteps
//------------------------------------------------------------
long    Filter_Wheel_Motor_Class::Filter_To_Position( uint8_t filter ) {

	float 		fSteps_per_Filter = ( ( FULL_WHEEL_ANGLE / 360.0f ) / this->iNum_Filters ) * MICROSTEPS * MOTOR_STEPS_PER_REVOLUTION ;

    return( (long)( filter * fSteps_per_Filter ) ); 
 
}


/*------------------------------------------------------------
|  Find_Home
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION:
|		Step 1: find any position sensor
|		Step 2: move through all the filter positions, looking for which one is home.
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
	float   fSearch_Angle;

	int16_t iPosition_Marker_Width;

	tFWM_Result result;

	FWM_DEBUG_MSG_LN( "FWM Home start" );

	// Reset the sensor thresholds and IO pins
	Init_Sensors();

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

	//------------------------------------------------------------
	// Step 0: Move "backwards" a little, in case we're already home.
	// This allows the sensors to take a dark reading or two before 
	// almost immediately finding the homw and position sensors.
	//------------------------------------------------------------
	motor.move( -8.0f * HOME_SEEK_STEP_SIZE_IN_STEPS );
	motor.runToPosition();	   


	//------------------------------------------------------------
	// Step 1: Find a position sensor.
	// Search one full filter position for the position sensor mark
	//------------------------------------------------------------
	fSearch_Angle = 0;

	Read_Sensors( &bHome_Sensor_Active, &bPosition_Sensor_Active );		// take initial reading

	// we need to search more than one filter angle because of the back-off in Step 0
	while ( ( !bPosition_Sensor_Active ) && ( fSearch_Angle <= ( 1.25f * FULL_WHEEL_ANGLE / this->iNum_Filters ) ) ) {

		// position sensor not found.  Move a little
		motor.move( HOME_SEEK_STEP_SIZE_IN_STEPS );
		motor.runToPosition();	   

		// update the search angle
		fSearch_Angle += HOME_SEEK_STEP_SIZE_IN_DEG;

		// check the sensors again
		Read_Sensors( &bHome_Sensor_Active, &bPosition_Sensor_Active );
	}

	// Exit with error if we didn't find the sensor
	if ( !bPosition_Sensor_Active ) {

		FWM_DEBUG_MSG_LN( "Couldn't find position sensor reading" );
		return FWM_RESULT_HOMING_FAILED;
	}


	//------------------------------------------------------------
	// Step 2: Check all positions for the home sensor
	// At this point the wheel should been aligned with the sensor mark.  
	// WE just need to search the filter positions for the home sensor.
	//------------------------------------------------------------
	fSearch_Angle = 0;		// reset the search angle

	Read_Sensors( &bHome_Sensor_Active, &bPosition_Sensor_Active );
	while ( !bHome_Sensor_Active && ( fSearch_Angle <= FULL_WHEEL_ANGLE ) ) {

		// Move one filter position
		motor.move( this->lSteps_per_Filter );
		motor.runToPosition();

		// update the filter angle we've searched
		fSearch_Angle += ( FULL_WHEEL_ANGLE / this->iNum_Filters );

		// read the sensor
		Read_Sensors( &bHome_Sensor_Active, &bPosition_Sensor_Active );

	}

	if ( bHome_Sensor_Active ) {
		bHome = true;
	} else {

		FWM_DEBUG_MSG_LN( "Couldn't find home sensor reading" );
		return FWM_RESULT_HOMING_FAILED;	
	}

	this->bFound_Home = bHome;

	//------------------------------------------------------------
	//	Step 3:
	// Home found.  Align to the center of the position sensor mark.
	//------------------------------------------------------------
	this->fCurrent_Angle = 0.0f;
	this->iCurrent_Filter = 0;

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

	//------------------------------------------------------------
	//	Hold the wheel at the current position
	//------------------------------------------------------------
	motor.enableOutputs();
	Set_Current_Position_As_Home();

	//------------------------------------------------------------
	//	Return the normal settings
	//------------------------------------------------------------
	motor.setMaxSpeed( NORMAL_SPEED );
	motor.setAcceleration( NORMAL_ACCELERATION );

	FWM_DEBUG_MSG_LN( "FWM Home done" );

	// Update the state.  Since the function is blocking, these state changes are never really visbile
	g_State = FWM_STATE_STOPPED;

	return result;
}


void	Filter_Wheel_Motor_Class::Set_Current_Position_As_Home( void ) {

	motor.setCurrentPosition( 0 );
}


