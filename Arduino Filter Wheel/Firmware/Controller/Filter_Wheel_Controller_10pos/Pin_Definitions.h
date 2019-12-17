/*------------------------------------------------------------
| Pin_Definitions.h			(c) 2016 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE:	Define pin usage
|
| DESCRIPTION:	MCU pin assignments for the LED Spectral Illuminator v1.0
|
| HISTORY:
| $Rev: 2615 $
| $Author: mokincha $
| $Date: 2013-01-17 17:20:04 -0800 (Thu, 17 Jan 2013) $
------------------------------------------------------------*/

#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

#ifdef __cplusplus
extern "C"
{
#endif


//------------------------------------------------------------
// Constants
//------------------------------------------------------------
#define	STD_IO_SERIAL	Serial

//------------------------------------------------------------
// Pin Definitions
//------------------------------------------------------------

//------------------------------------------------------------
//	Pin Definitions
//------------------------------------------------------------

// Stepper Motor Driver
#define	PIN_MOTOR_DRIVER_ENABLE		4
#define	PIN_MOTOR_DRIVER_STEP		2
#define	PIN_MOTOR_DRIVER_DIR		7

// Position sensor pins
#define PIN_SENSOR_LED          A2
#define PIN_SENSOR_POSITION     A0
#define PIN_SENSOR_HOME         A1

// Rotary Encoder
#define	PIN_KNOB_PHASE_A			5
#define	PIN_KNOB_PHASE_B			6
#define	PIN_KNOB_SWITCH				9

//	OLED Display
#define	PIN_LCD_SCL					A4
#define	PIN_LCD_SDA					A5

//------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------

#ifdef __cplusplus
} // extern "C"
#endif

#endif // PIN_DEFINITIONS_H

