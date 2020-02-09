/*------------------------------------------------------------
| Dew_Cap_Heater_Controller.h           (c) 2019 M. Okincha
|-------------------------------------------------------------
|
| PURPOSE: Main Loop
|
| DESCRIPTION: Dew Cap Heater Controller
|
| HISTORY:
|   20May11 TL from PTC controller driver.
------------------------------------------------------------*/

#ifndef	DEW_CAP_HEATER_CONTROLLER_H
#define	DEW_CAP_HEATER_CONTROLLER_H

#define DEBUG // Define this symbol to enable debug code,
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


#include <OneWire.h>					// temp sensor
#include <DallasTemperature.h>			// temp sensor

#include <DHT.h>
#include <DHT_U.h>

//	CONSTANTS

#define NUM_CHANNELS	2
#define	ROTARYSTEPS		1

#define	DEW_CAP_HEATER_CHANNEL_MAIN_SCOPE	0
#define	DEW_CAP_HEATER_CHANNEL_GUIDE_SCOPE	1

#define	DEW_CAP_HEATER_POWER_OUTPUT_MAX		255.0f

#define	DEW_TEMPERATURE_OFFSET_IN_C			5.0f		// Keep the scope a little about the Dew point
#define	SAMPLE_RATE_IN_MS					5000		// The loop sample rate

#define PID_INTEGRATOR_UPPER_LIMIT  300.0f
#define PID_INTEGRATOR_LOWER_LIMIT  -50.f

// channel 0 is the main scope - 200mm Celestron 8SE
#define	HEATER_CHANNEL0_KP	1.0f
#define	HEATER_CHANNEL0_KI	1.0f
#define	HEATER_CHANNEL0_KD	0.0f

// channel 1 is the guide scope - 80mm Orion ST80
#define	HEATER_CHANNEL1_KP	1.0f
#define	HEATER_CHANNEL1_KI	1.0f
#define	HEATER_CHANNEL1_KD	0.0f

//	DATA STRUCTURES
typedef enum  eDisplay_Status {
  
  DISPLAY_STATUS_BOOTING_UP,
  DISPLAY_STATUS_SHOW_STATUS,
	  
}  tDisplay_Status;

// DHT22 Temp/humidity sensor structure
#define DHT_TYPE    DHT22     // DHT 22 (AM2302)

typedef	struct t_sTemp_Humidity_Sensor {
	DHT_Unified* 	pSensor;
	float			fTemperature_in_C;
	float			fRelative_Humidity_in_Percent;
} t_sTemp_Humidity_Sensor;

// PID Loop structure
typedef	struct t_sPID_State {
	float	fSetPoint;
	float	fInput;
	float	fIntegralAccumulator;
	float	fKp;		// propotional constant
	float	fKi;		// integral constant
	float	fKd;		// differential constant
	float	fOutput;	//
} t_sPID_State;

// DS1820+ Temperature Sensor structure
typedef struct t_sTemp_Sensor {
	DallasTemperature*	pSensor;
	float				fReading_in_C;
	bool				bReading_Is_Good;
} t_sTemp_Sensor;

// Heater Structure
typedef struct t_sHeater {
	uint16_t	uPin;
} t_sHeater;

// Composite struct for all dew cap heater components
typedef struct t_sDew_Cap_Heater {
	t_sPID_State	sPID_State;
	t_sTemp_Sensor	sTemp_Sensor;
	t_sHeater		sHeater;
} t_sDew_Cap_Heater;

//------------------------------------------------------------
//	Function Prototypes
//------------------------------------------------------------

float	Init_Temp_Humidity_Sensor( t_sTemp_Humidity_Sensor* pSensor_Struct );
float	Get_Dew_Point_Temp_in_C( t_sTemp_Humidity_Sensor* pSensor_Struct );

float	Init_Temp_Sensor( t_sTemp_Sensor* pTemp_Sensor_Struct );
float	Get_Temperature( t_sTemp_Sensor* pTemp_Sensor_Struct );

void	Init_PI_Loop( t_sPID_State* pPID_State_Struct );
void	Run_PI_Loop( t_sPID_State* pPID_State_Struct );

void	Init_Heater_Output( t_sHeater* pHeater );
void	Set_Heater_Output( t_sHeater* pHeater, float fHeater_Power_Level );

void	Display_Show_Status( tDisplay_Status display_status );
void	Display_Show_Position( uint8_t  pos );

#endif	// DEW_CAP_HEATER_CONTROLLER_H
