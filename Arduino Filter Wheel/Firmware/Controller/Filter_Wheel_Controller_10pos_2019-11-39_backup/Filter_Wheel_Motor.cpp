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
#include "DRV8825.h"

//------------------------------------------------------------
//	Constants
//------------------------------------------------------------


//------------------------------------------------------------
//	Global Variables
//------------------------------------------------------------

//BasicStepperDriver stepper( MOTOR_STEPS, PIN_MOTOR_DRIVER_DIR, PIN_MOTOR_DRIVER_STEP, PIN_MOTOR_DRIVER_ENABLE );
//BasicStepperDriver stepper( MOTOR_STEPS, PIN_MOTOR_DRIVER_DIR, PIN_MOTOR_DRIVER_STEP );
DRV8825 stepper( MOTOR_STEPS, PIN_MOTOR_DRIVER_DIR, PIN_MOTOR_DRIVER_STEP );

// global min/max values for adaptively setting thresholds
int16_t	g_Sensor_Home_Max;
int16_t	g_Sensor_Home_Min;
int16_t	g_Sensor_Position_Max;
int16_t	g_Sensor_Position_Min;


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
 pinMode( PIN_MOTOR_DRIVER_ENABLE, OUTPUT );
 digitalWrite( PIN_MOTOR_DRIVER_ENABLE, LOW );
 
	stepper.begin( NORMAL_SPEED, MICROSTEPS );
	stepper.setEnableActiveState( LOW );
	stepper.setSpeedProfile( stepper.LINEAR_SPEED, 1000, 1000 );

	// energize coils - the motor will hold position
	stepper.enable();
//	stepper.rotate( 90 );

	// prepare the sensors
	pinMode( PIN_SENSOR_LED, OUTPUT );
	digitalWrite( PIN_SENSOR_LED, LOW );

	// set the global min/max values
	g_Sensor_Home_Max = -32760;    
	g_Sensor_Home_Min = 32760;    
	g_Sensor_Position_Max = -32760;
	g_Sensor_Position_Min = 32760;

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

  int16_t iPosition_Marker_Width;
  
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
//			stepper.rotate( HOME_SEEK_STEP_SIZE_IN_DEG );
      stepper.move( HOME_SEEK_STEP_SIZE_IN_STEPS );

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

    // now find the width of the position mark in motor steps and position the wheel at the half-way point
    FWM_DEBUG_MSG_LN( "Finding positon mark width." );

    // first, make sure the position sensor is active
    do {
          FWM_DEBUG_MSG_LN( "Waiting for position sensor to be active" );
          Read_Sensors( &bHome_Sensor_Active, &bPosition_Sensor_Active );
          if ( !bPosition_Sensor_Active ) { 
            stepper.move( 1 );
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
            stepper.move( 1 );
          }
    } while ( bPosition_Sensor_Active );
    FWM_DEBUG_MSG_LN( "Position sensor is inactive" );

    FWM_DEBUG_MSG( "Position mark is " );
    FWM_DEBUG_MSG_VAL( iPosition_Marker_Width, DEC );
    FWM_DEBUG_MSG_LN( " steps wide." );

    iPosition_Marker_Width = -(iPosition_Marker_Width/2 );
    stepper.move( iPosition_Marker_Width );
    FWM_DEBUG_MSG( "Moved back " );
    FWM_DEBUG_MSG_VAL( iPosition_Marker_Width, DEC );
    FWM_DEBUG_MSG_LN( " steps." );
	}

  // energize coils - the motor will hold position
//  stepper.enable();        
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

  float  fAngle_to_Move;
  long  iSteps_to_Move;
  // get the number of steps remaining so we can update
  iSteps_to_Move = stepper.getDirection() * stepper.getStepsRemaining();
  
  // move to the desired position
	fAngle_to_Move += iSteps_To_Move * ( this->fFull_Filter_Angle / this->uNum_Filters );  
	FWM_DEBUG_MSG( ", Angle: " );
	FWM_DEBUG_MSG_VAL( fAngle_to_Move, DEC );
	FWM_DEBUG_MSG_LN( "" );

  iSteps_to_Move += ( fAngle_to_Move / 360.0f ) * MICROSTEPS * MOTOR_STEPS;
  
  // start the moving
	stepper.startMove( iSteps_to_Move );

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

	uint8_t		i;

	// reset the accumulators
	iHome_Reading_Dark = 0;      
	iHome_Reading_Bright = 0;    
	iPosition_Reading_Dark = 0;  
	iPosition_Reading_Bright = 0;

	// Take readings with the LED on and off.  Use the difference to judge reflectance.  This removes temperature effects on output
	// sample the sensors a number of times
	for ( i=0; i!=FILTER_WHEEL_SENSOR_SAMPLES; i++ ) {

		// turn ON LED power
		digitalWrite( PIN_SENSOR_LED, HIGH );
		delay( 2 );
		iHome_Reading_Bright += analogRead( PIN_SENSOR_HOME );
		iPosition_Reading_Bright += analogRead( PIN_SENSOR_POSITION );

		// turn off LED power
		digitalWrite( PIN_SENSOR_LED, LOW );
		delay( 2 );
		iHome_Reading_Dark += analogRead( PIN_SENSOR_HOME );
		iPosition_Reading_Dark += analogRead( PIN_SENSOR_POSITION );
	}

	// Compute the differences and scale the reading accumulators to compute the average
	// Note that dark readings are higher than bright readings.  LED light causes the phototransistor to pull down
	iHome_Reading_Diff = ( iHome_Reading_Dark - iHome_Reading_Bright ) / FILTER_WHEEL_SENSOR_SAMPLES;
	iPosition_Reading_Diff = ( iPosition_Reading_Dark - iPosition_Reading_Bright ) / FILTER_WHEEL_SENSOR_SAMPLES;

	// update the global min/max
	g_Sensor_Home_Max = ( iHome_Reading_Diff > g_Sensor_Home_Max ) ? iHome_Reading_Diff : g_Sensor_Home_Max;    
	g_Sensor_Home_Min = ( iHome_Reading_Diff < g_Sensor_Home_Min ) ? iHome_Reading_Diff : g_Sensor_Home_Min;    
	g_Sensor_Position_Max = ( iPosition_Reading_Diff > g_Sensor_Position_Max ) ? iPosition_Reading_Diff : g_Sensor_Position_Max;
	g_Sensor_Position_Min = ( iPosition_Reading_Diff < g_Sensor_Position_Min ) ? iPosition_Reading_Diff : g_Sensor_Position_Min;

	// Compute thresholds
	iHome_Threshold = ( g_Sensor_Home_Max + g_Sensor_Home_Min )/2;
	iPosition_Threshold = ( g_Sensor_Position_Max + g_Sensor_Position_Min )/2;

	// if the min or max HOME sensor readings are too wide or too narrow, then the sensor should be marked inactive
	if ( ( g_Sensor_Home_Max > ( SENSOR_ADAPTIVE_THRESHOLD_MAX_DISTANCE + SENSOR_HOME_THRESHOLD_DEFAULT ) ) ||
		 ( ( g_Sensor_Home_Max - g_Sensor_Home_Min ) < SENSOR_ADAPTIVE_THRESHOLD_MIN_DISTANCE ) ||
		( g_Sensor_Home_Min < ( SENSOR_ADAPTIVE_THRESHOLD_MIN_DISTANCE - SENSOR_HOME_THRESHOLD_DEFAULT ) ) ) {

		bIs_Home_Sensor_Active = false;

	} else {

		// Decide which if home sensor is active
		bIs_Home_Sensor_Active = ( iHome_Reading_Diff > iHome_Threshold );
	}


	// if the min or max POSITION sensor readings are too wide or too narrow, then the sensor should be marked inactive
	if ( ( g_Sensor_Position_Max > ( SENSOR_ADAPTIVE_THRESHOLD_MAX_DISTANCE + SENSOR_POSITION_THRESHOLD_DEFAULT ) ) ||
		 ( ( g_Sensor_Position_Max - g_Sensor_Position_Min ) < SENSOR_ADAPTIVE_THRESHOLD_MIN_DISTANCE ) ||
		( g_Sensor_Position_Min < ( SENSOR_ADAPTIVE_THRESHOLD_MIN_DISTANCE - SENSOR_POSITION_THRESHOLD_DEFAULT ) ) ) {

		bIs_Position_Sensor_Active = false;

	} else {

		// Decide which if position sensor is active
		bIs_Position_Sensor_Active = ( iPosition_Reading_Diff > iPosition_Threshold );
	}

	// read the home sensor
	if ( bHome_Sensor_Active != NULL ) {

		*bHome_Sensor_Active = bIs_Home_Sensor_Active;
	}

	// read the position sensor
	if ( bPosition_Sensor_Active != NULL ) {

		*bPosition_Sensor_Active = bIs_Position_Sensor_Active;
	}

	// report HOME sensor stats
	FWM_DEBUG_MSG( "HOME: Min:" );
	FWM_DEBUG_MSG_VAL( g_Sensor_Home_Min, DEC );
	FWM_DEBUG_MSG( ", Max: " );
	FWM_DEBUG_MSG_VAL( g_Sensor_Home_Max, DEC );
	FWM_DEBUG_MSG( ", Thr: " );
	FWM_DEBUG_MSG_VAL( iHome_Threshold, DEC );
	FWM_DEBUG_MSG( ", Cur:" );
	FWM_DEBUG_MSG_VAL( iHome_Reading_Diff, DEC );
	FWM_DEBUG_MSG( ", Active:" );
	FWM_DEBUG_MSG_VAL( bIs_Home_Sensor_Active, DEC );
//	FWM_DEBUG_MSG_LN( "" );

	// report POSITION sensor stats
	FWM_DEBUG_MSG( ", POSITION: Min:" );
	FWM_DEBUG_MSG_VAL( g_Sensor_Position_Min, DEC );
	FWM_DEBUG_MSG( ", Max: " );
	FWM_DEBUG_MSG_VAL( g_Sensor_Position_Max, DEC );
	FWM_DEBUG_MSG( ", Thr: " );
	FWM_DEBUG_MSG_VAL( iPosition_Threshold, DEC );
	FWM_DEBUG_MSG( ", Cur:" );
	FWM_DEBUG_MSG_VAL( iPosition_Reading_Diff, DEC );
	FWM_DEBUG_MSG( ", Active:" );
	FWM_DEBUG_MSG_VAL( bIs_Position_Sensor_Active, DEC );
	FWM_DEBUG_MSG_LN( "" );

	// turn off LED power.  Redundant, but safe
	digitalWrite( PIN_SENSOR_LED, LOW );

}


bool    Filter_Wheel_Motor_Class::Service( void ) {

    unsigned wait_time_micros;
    bool  bIs_Moving;

    // motor control loop - send pulse and return how long to wait until next pulse
    wait_time_micros = stepper.nextAction();

    bIs_Moving = false;
    if ( wait_time_micros > 0 ) {
        bIs_Moving = true;
    }

#if 0    // 0 wait time indicates the motor has stopped
    // (optional) execute other code if we have enough time
    if ( wait_time_micros > 100 ) {
        // other code here
    }
#endif

  return  bIs_Moving;
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
