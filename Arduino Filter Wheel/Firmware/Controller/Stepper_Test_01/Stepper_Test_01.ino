// Stepper_Test_01.pde
// -*- mode: C++ -*-
//
// Shows how to use the blocking call runToNewPosition
// Which sets a new target position and then waits until the stepper has 
// achieved it.
//
// Copyright (C) 2009 Mike McCauley
// $Id: Blocking.pde,v 1.1 2011/01/05 01:51:01 mikem Exp mikem $

#define  PIN_MOTOR_DRIVER_ENABLE   4
#define PIN_MOTOR_DRIVER_STEP   2
#define PIN_MOTOR_DRIVER_DIR    7

#include <AccelStepper.h>

// Define a stepper and the pins it will use
AccelStepper stepper( AccelStepper::DRIVER, PIN_MOTOR_DRIVER_STEP, PIN_MOTOR_DRIVER_DIR );

void setup()
{  

    // Enable the motor
    stepper.setEnablePin( PIN_MOTOR_DRIVER_ENABLE );
    stepper.setPinsInverted( false, false, true );
  
    // energize coils - the motor will hold position
    stepper.enableOutputs();
  
    stepper.setMaxSpeed( 1000.0 );
    stepper.setAcceleration( 1000.0 );
}

void loop()
{    
    stepper.runToNewPosition(0);
    stepper.runToNewPosition( 200 * 16 );

    stepper.disableOutputs();
    while( 1 ) {}
}
