/*------------------------------------------------------------
| Pin_Definitions.h			(c) 2019 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE:	Define pin usage
|
| DESCRIPTION:	MCU pin assignments for the Dew Cap Heater Controller
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

//	OLED Display
#define	PIN_LCD_SCL					A4
#define	PIN_LCD_SDA					A5

// Temp/Humidity Sensor	DHT22
#define	PIN_TEMP_HUMIDITY_SENSOR	9

// Heater 0 Pin set 
#define	PIN_HEATER_POWER_CH0		5
#define	PIN_TEMP_SENSOR_CH0			7

// Heater 1 Pin set 
#define	PIN_HEATER_POWER_CH1		6
#define	PIN_TEMP_SENSOR_CH1			8

// Rotary Encoder
#define	PIN_KNOB_PHASE_A			2
#define	PIN_KNOB_PHASE_B			3
#define	PIN_KNOB_SWITCH				4


//------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------

#ifdef __cplusplus
} // extern "C"
#endif

#endif // PIN_DEFINITIONS_H

