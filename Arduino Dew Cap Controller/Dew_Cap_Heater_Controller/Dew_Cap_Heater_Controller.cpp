/*------------------------------------------------------------
| Dew_Cap_Heater_Controller.cpp           (c) 2019 M. Okincha
|-------------------------------------------------------------
|
| PURPOSE: Dew Cap Heater Controller
|
| DESCRIPTION: This is a template application that shows how
| to implement a serial command-line parser.
|
| Requires:
| DS1820+ sensor
| https://github.com/milesburton/Arduino-Temperature-Control-Library
| DHT22 Sensor
| https://www.arduinolibraries.info/libraries/dht-sensor-library
| DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
| Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor
| LED Display
| HISTORY:
|   13Jun12	MAO	
------------------------------------------------------------*/

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <avr/power.h> // Comment out this line for non-AVR boards (Arduino Due, etc.)


#include "Dew_Cap_Heater_Controller.h"
#include "Pin_Definitions.h"


//============================================
// 	Program Options
//============================================
//#define		USE_KNOB
#define	USE_SSD1306_DISPLAY
//#define	SERVICE_SENSORS_AND_HEATERS

//============================================
// 	Rotary Knob
//============================================
#ifdef	USE_KNOB
#include <RotaryEncoder.h>				// rotary knob
int 	lastPos = 0;
// Setup a RotaryEncoder for pins A2 and A3:
RotaryEncoder encoder( PIN_KNOB_PHASE_A, PIN_KNOB_PHASE_B );
#endif	// USE_KNOB
int 	newPos;


//============================================
// Display
//============================================

#ifdef	USE_SSD1306_DISPLAY
#include <Wire.h>						// Display and T/RH sensor
#include <Adafruit_GFX.h>				// Display
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire );
#endif
bool  	bUpdate_Display = true;
bool	bDisplay_Ready;


//============================================
// Temp Sensors
//============================================
#ifdef	SERVICE_SENSORS_AND_HEATERS
#include <OneWire.h>					// temp sensor
#include <DallasTemperature.h>			// temp sensor

OneWire g_OneWire0( PIN_TEMP_SENSOR_CH0 );
OneWire g_OneWire1( PIN_TEMP_SENSOR_CH1 );

// Pass our oneWire references to Dallas Temperature. 
DallasTemperature g_DS1820_Sensor0( &g_OneWire0 );
DallasTemperature g_DS1820_Sensor1( &g_OneWire1 );
#endif	// SERVICE_SENSORS_AND_HEATERS


//============================================
// Temp/RH Sensor
//============================================
#ifdef	SERVICE_SENSORS_AND_HEATERS
#include <DHT.h>
#include <DHT_U.h>

DHT_Unified		g_DHT22_Sensor( PIN_TEMP_HUMIDITY_SENSOR, DHT_TYPE );

t_sTemp_Humidity_Sensor	g_Temp_Humidity_Sensor;
#endif	// SERVICE_SENSORS_AND_HEATERS

//============================================
// Heaters
//============================================
// Heater and PID loop data
#ifdef	SERVICE_SENSORS_AND_HEATERS
t_sDew_Cap_Heater	g_Dew_Cap_Heater[ NUM_CHANNELS ];
#endif

//============================================
// Misc
//============================================
static uint32_t	g_uLast_Sample_Time_in_ms;
uint32_t	uOld_Now;

void(* resetFunc) (void) = 0;//declare reset function at address 0
//#include "CommandParser.h"
//#include "Parser.h"


/*------------------------------------------------------------
| setup
|-------------------------------------------------------------
|
| PURPOSE: To initialize the booth controller application.
|
| DESCRIPTION: In Arduino OS, this routine is called first.
|
| HISTORY:
|   13Jun12	MAO
------------------------------------------------------------*/
void    setup() {

	uint16_t	i;

	// Open the serial port, and set the data rate to 9600 bps.
	// This should be done first so that error messages can be
	// seen if device configuration fails.
	Serial.begin( 115200 );

	IMA_DEBUG_MSG_LN( "Setup: start... " );

#if 0

	//---------------------------------------------
	// Configure CLI
	//---------------------------------------------

	// Indicate that the hardware has been initialized and is
	// now ready to respond to commands.
	Command_Line_Interface_Init();

	// Indicate that the hardware has been initialized and is
	// now ready to respond to commands.
	CmdHandler_IDN();

	// set up the command handler
	CommandParserInit( CommandStringList, CommandHandlerTable );
#endif


	//---------------------------------------------
	// Prepare home button
	//---------------------------------------------
	// Setup the button with an internal pull-up :
	pinMode( PIN_KNOB_SWITCH, INPUT_PULLUP );
  
	//---------------------------------------------
	// Prepare the display
	//---------------------------------------------
#ifdef	USE_SSD1306_DISPLAY
	bDisplay_Ready = display.begin(SSD1306_SWITCHCAPVCC, 0x3C );

	// wait for the display to wake up...
	delay( 100 );

	if ( bDisplay_Ready ) {

		IMA_DEBUG_MSG_LN( "SSD1306 allocation success" );

		// Update the display
		Display_Show_Status( DISPLAY_STATUS_BOOTING_UP );

	} else {
		IMA_DEBUG_MSG_LN( "Warning: SSD1306 allocation error" );
	}
#endif	// USE_SSD1306_DISPLAY


	//---------------------------------------------
	// Prepare the rotation knob
	//---------------------------------------------
#ifdef	USE_KNOB
	encoder.setPosition( 0 / ROTARYSTEPS );	// start with the value of 10.
	newPos = 0;
#endif	// USE_KNOB


	//---------------------------------------------
	// Prepare PID Loops, temp sensors, and PWM channels
	//---------------------------------------------
#ifdef	SERVICE_SENSORS_AND_HEATERS
	g_Dew_Cap_Heater[ 0 ].sHeater.uPin = PIN_HEATER_POWER_CH0; 
	g_Dew_Cap_Heater[ 1 ].sHeater.uPin = PIN_HEATER_POWER_CH1; 
	g_Dew_Cap_Heater[ 0 ].sTemp_Sensor.pSensor = &g_DS1820_Sensor0; 
	g_Dew_Cap_Heater[ 1 ].sTemp_Sensor.pSensor = &g_DS1820_Sensor1; 

	for ( i=0; i!=NUM_CHANNELS; i++) {

		// Prepare the temp sensors
		Init_Temp_Sensor( &(g_Dew_Cap_Heater[i].sTemp_Sensor) );

		// prepare the dew_cap heaters
		Init_PI_Loop( &(g_Dew_Cap_Heater[i].sPID_State) );

		Init_Heater_Output( &(g_Dew_Cap_Heater[i].sHeater) );

	}

	g_Dew_Cap_Heater[0].sPID_State.fKp = HEATER_CHANNEL0_KP;
	g_Dew_Cap_Heater[0].sPID_State.fKi = HEATER_CHANNEL0_KI;
	g_Dew_Cap_Heater[0].sPID_State.fKd = HEATER_CHANNEL0_KD;

	g_Dew_Cap_Heater[1].sPID_State.fKp = HEATER_CHANNEL1_KP;
	g_Dew_Cap_Heater[1].sPID_State.fKi = HEATER_CHANNEL1_KI;
	g_Dew_Cap_Heater[1].sPID_State.fKd = HEATER_CHANNEL1_KD;

	//---------------------------------------------
	// Prepare the DHT22 temp/humidity sensor
	//---------------------------------------------
	Init_Temp_Humidity_Sensor( &g_Temp_Humidity_Sensor );
#endif	// SERVICE_SENSORS_AND_HEATERS


	//---------------------------------------------
	// Finish up
	//---------------------------------------------
	// Clear the buffer
#ifdef	USE_SSD1306_DISPLAY
	display.clearDisplay();
#endif

	g_uLast_Sample_Time_in_ms = millis();
}


/*------------------------------------------------------------
| loop
|-------------------------------------------------------------
|
| PURPOSE: The main operation loop for the whole application.
|
| DESCRIPTION: In Arduino OS, this routine repeats
| indefinitely.
|
| HISTORY:
|   13Jun12	MAO

------------------------------------------------------------*/
void    loop() {

	uint16_t	i;
	float	fDew_Point_in_C;
	uint32_t	uNow;

#if 0
// service serial commands
	CommandParserService();
#endif

#ifdef	USE_KNOB
	//---------------------------------------------
	// Service rotary switch
	//---------------------------------------------
	encoder.tick();

	// Get the updated value :
	if ( digitalRead( PIN_KNOB_SWITCH ) == LOW ) {

	}
  
	//---------------------------------------------
	// get the current physical position and calc the logical position
	//---------------------------------------------
	newPos = encoder.getPosition();

	if ( newPos < ROTARYMIN ) {

		newPos = ROTARYMAX;

	} else if ( newPos > ROTARYMAX ) {

		newPos = ROTARYMIN;
	} // if
#endif	// USE_KNOB


#ifdef	SERVICE_SENSORS_AND_HEATERS
	//---------------------------------------------
	// Get ambient Temp
	//---------------------------------------------
	uNow = millis();

	// Service the PID loops if enough time has elapsed
	if ( uNow < g_uLast_Sample_Time_in_ms ) {

		// handle overflows
		if ( uNow < uOld_Now ) {
			g_uLast_Sample_Time_in_ms = uNow + SAMPLE_RATE_IN_MS;
		}

	} else {

		IMA_MSG_LN( "");
		IMA_MSG_LN( "======================================");
			IMA_MSG_LN( "Servicing PID Loops");
		IMA_MSG_LN( "======================================");

		// Update the sample time
		g_uLast_Sample_Time_in_ms = uNow + SAMPLE_RATE_IN_MS;
		IMA_DEBUG_MSG( "Sample time: ");
		IMA_DEBUG_MSG_VAL( g_uLast_Sample_Time_in_ms, DEC );
		IMA_DEBUG_MSG_LN( "" ); 

		fDew_Point_in_C = Get_Dew_Point_Temp_in_C( &g_Temp_Humidity_Sensor );

		//---------------------------------------------
		// Service PID loops
		//---------------------------------------------
		for ( i=0; i!= NUM_CHANNELS; i++ ) {

			IMA_DEBUG_MSG( "Servicing Channel ");
			IMA_DEBUG_MSG_VAL( i, DEC );
			IMA_DEBUG_MSG_LN( "" ); 

			// Get the current channel temperature
			Get_Temperature( &( g_Dew_Cap_Heater[i].sTemp_Sensor ) );

			// service the PID if the reading is good
			if ( g_Dew_Cap_Heater[i].sTemp_Sensor.bReading_Is_Good) {

				// Update the setpoint temp
				g_Dew_Cap_Heater[i].sPID_State.fSetPoint = fDew_Point_in_C + DEW_TEMPERATURE_OFFSET_IN_C;

				// Update the current temp
				g_Dew_Cap_Heater[i].sPID_State.fInput = g_Dew_Cap_Heater[i].sTemp_Sensor.fReading_in_C;

				// service the loop
				Run_PI_Loop( &( g_Dew_Cap_Heater[i].sPID_State ) );

			} else {

				// Temp reading is bad; reset the PID loop
				Init_PI_Loop( &(g_Dew_Cap_Heater[i].sPID_State) );
			}

			// Update the output
			Set_Heater_Output( &(g_Dew_Cap_Heater[i].sHeater), g_Dew_Cap_Heater[i].sPID_State.fOutput );

			IMA_DEBUG_MSG( "Done with  Channel ");
			IMA_DEBUG_MSG_VAL( i, DEC );
			IMA_DEBUG_MSG_LN( "" ); 
		}

#ifdef	USE_SSD1306_DISPLAY
		// Update the display
		if ( bDisplay_Ready ) {
			Display_Show_Status( DISPLAY_STATUS_SHOW_STATUS );
		}
#endif	// USE_SSD1306_DISPLAY

	}
#endif	// SERVICE_SENSORS_AND_HEATERS

	// save current timestamp so we can detect overflows
	uOld_Now = uNow;

#ifdef USE_SSD1306_DISPLAY
	//---------------------------------------------
	// Update the display
	//---------------------------------------------
//	if ( bDisplay_Ready && bUpdate_Display && bDone_Moving ) {
	if ( bDisplay_Ready && bUpdate_Display ) {

		Display_Show_Position( newPos );

		bUpdate_Display = false;
	}
#endif	// USE_SSD1306_DISPLAY

}

#ifdef	SERVICE_SENSORS_AND_HEATERS
//============================================
// Init_Temp_Humidity_Sensor
//============================================
float	Init_Temp_Humidity_Sensor( t_sTemp_Humidity_Sensor* pSensor_Struct ) {

	// Init the sensor
	pSensor_Struct->pSensor = &g_DHT22_Sensor;
	pSensor_Struct->pSensor->begin();

	// Set some sane intitial values
	pSensor_Struct->fTemperature_in_C = 20.0f;
	pSensor_Struct->fRelative_Humidity_in_Percent = 50.0f;
}
#endif	//	SERVICE_SENSORS_AND_HEATERS


#ifdef	SERVICE_SENSORS_AND_HEATERS
//============================================
// Get_Dew_Point_Temp_in_C
//============================================
float	Get_Dew_Point_Temp_in_C( t_sTemp_Humidity_Sensor* pSensor_Struct ) {

	float	T;
	float	RH;
	float	H;
	float	Tdp;
	sensors_event_t event;

	// Get temperature event and print its value.
	pSensor_Struct->pSensor->temperature().getEvent( &event );

	if ( isnan( event.temperature ) ) {

		IMA_DEBUG_MSG_LN( "Error reading temperature!" );

	} else {

		IMA_DEBUG_MSG( "Temperature: " );
		IMA_DEBUG_MSG_VAL( event.temperature, DEC );
		IMA_DEBUG_MSG_LN( "C" ); 

		pSensor_Struct->fTemperature_in_C = event.temperature;
	}

	// Get humidity event and print its value.
	pSensor_Struct->pSensor->humidity().getEvent(&event);

	if ( isnan( event.relative_humidity ) ) {

		Serial.println( "Error reading humidity!");

	} else {

		IMA_DEBUG_MSG( "Humidity: " );
		IMA_DEBUG_MSG_VAL( event.relative_humidity, DEC );
		IMA_DEBUG_MSG_LN( "%" );

		pSensor_Struct->fRelative_Humidity_in_Percent = event.relative_humidity;
	}

	// compute dew point
	T = pSensor_Struct->fTemperature_in_C;
	RH = pSensor_Struct->fRelative_Humidity_in_Percent;
	H = ( log10( RH )-2.0f )/0.4343f + ( 17.62f * T ) / ( 243.12f + T ); 
	Tdp = 243.12f * H / ( 17.62f - H ); // this is the dew point in Celsius 

	IMA_DEBUG_MSG( "Dew Point: " );
	IMA_DEBUG_MSG_VAL( Tdp, DEC );
	IMA_DEBUG_MSG_LN( "C" ); 

	return Tdp;
}
#endif	//	SERVICE_SENSORS_AND_HEATERS


#ifdef	SERVICE_SENSORS_AND_HEATERS
//============================================
// Init_Temp_Sensor
//============================================
float	Init_Temp_Sensor( t_sTemp_Sensor* pTemp_Sensor_Struct ) {

	// Init the hardware
	pTemp_Sensor_Struct->pSensor->begin();

	// Count how many devices on the bus
	uint16_t	device_count = pTemp_Sensor_Struct->pSensor->getDeviceCount();
	IMA_DEBUG_MSG( "Found " );
	IMA_DEBUG_MSG_VAL( device_count, DEC );
	IMA_DEBUG_MSG_LN( " temp sensor" ); 

	// Since we haven't gotten a reading, flag whatever we have as bad
	pTemp_Sensor_Struct->bReading_Is_Good = false;
}
#endif	//	SERVICE_SENSORS_AND_HEATERS


#ifdef	SERVICE_SENSORS_AND_HEATERS
//============================================
// Get_Temperature
//============================================
float	Get_Temperature(t_sTemp_Sensor* pTemp_Sensor_Struct ) {

	// call sensors.requestTemperatures() to issue a global temperature 
	// request to all devices on the bus
//	IMA_DEBUG_MSG("Requesting temperatures...");
	pTemp_Sensor_Struct->pSensor->requestTemperatures(); // Send the command to get temperatures
//	IMA_DEBUG_MSG_LN("DONE");
	// After we got the temperatures, we can print them here.
	// We use the function ByIndex, and as an example get the temperature from the first sensor only.

	 pTemp_Sensor_Struct->fReading_in_C = pTemp_Sensor_Struct->pSensor->getTempCByIndex(0);
//	 pTemp_Sensor_Struct->fReading_in_C = 20.0f;

	// Check if reading was successful
	if ( pTemp_Sensor_Struct->fReading_in_C != DEVICE_DISCONNECTED_C ) {

		IMA_DEBUG_MSG("Temperature for the device 1 (index 0) is: ");
		IMA_DEBUG_MSG_VAL( pTemp_Sensor_Struct->fReading_in_C, DEC );
		IMA_DEBUG_MSG_LN( "" );
		pTemp_Sensor_Struct->bReading_Is_Good = true;

	} else {

		IMA_DEBUG_MSG_LN( "Error: Could not read temperature data" );
		pTemp_Sensor_Struct->bReading_Is_Good = false;
	}

	return  pTemp_Sensor_Struct->fReading_in_C;
}
#endif	//	SERVICE_SENSORS_AND_HEATERS


#ifdef	SERVICE_SENSORS_AND_HEATERS
//============================================
// Init_PI_Loop
//============================================
void	Init_PI_Loop( t_sPID_State* pPID_State_Struct ) {

	// Set the initial values
	pPID_State_Struct->fIntegralAccumulator = 0.0f;
	pPID_State_Struct->fSetPoint = 0.0f;
	pPID_State_Struct->fInput = 0.0f;
	pPID_State_Struct->fOutput = 0.0f;
}
#endif	//	SERVICE_SENSORS_AND_HEATERS


#ifdef	SERVICE_SENSORS_AND_HEATERS
//============================================
// Run_PI_Loop
//============================================
void	Run_PI_Loop( t_sPID_State* pPID_State_Struct ) {

	// Compute the error
	float fError = pPID_State_Struct->fSetPoint - pPID_State_Struct->fInput;

#if	0

	// Run a proper PID loop
	// Update state variables
	pPID_State_Struct->fIntegralAccumulator += fError;

  // Clamp to reasonable values.
  pPID_State_Struct->fIntegralAccumulator = ( pPID_State_Struct->fIntegralAccumulator > PID_INTEGRATOR_UPPER_LIMIT ) ? PID_INTEGRATOR_UPPER_LIMIT : pPID_State_Struct->fIntegralAccumulator;
  pPID_State_Struct->fIntegralAccumulator = ( pPID_State_Struct->fIntegralAccumulator < PID_INTEGRATOR_LOWER_LIMIT ) ? PID_INTEGRATOR_LOWER_LIMIT : pPID_State_Struct->fIntegralAccumulator;
  
	// Compute the output
	pPID_State_Struct->fOutput = pPID_State_Struct->fKp * fError;
	pPID_State_Struct->fOutput += pPID_State_Struct->fKi * pPID_State_Struct->fIntegralAccumulator;
#else

	// Run a simple on/off controller
	if ( fError > 0 ) {
		// if the set point is greater than the current value, run the heater.
		pPID_State_Struct->fOutput = DEW_CAP_HEATER_POWER_OUTPUT_MAX;
	} else {
		pPID_State_Struct->fOutput = 0;
	}
#endif

}
#endif	//	SERVICE_SENSORS_AND_HEATERS


#ifdef	SERVICE_SENSORS_AND_HEATERS
//============================================
// Init_Heater_Output
//============================================
void	Init_Heater_Output( t_sHeater* pHeater ) {

	// Set the pin to active output
	pinMode( pHeater->uPin, OUTPUT );

	// drive to zero.  I.e. turn off the heater.
	analogWrite( pHeater->uPin, 0 );
}
#endif	//	SERVICE_SENSORS_AND_HEATERS


#ifdef	SERVICE_SENSORS_AND_HEATERS
//============================================
// Set_Heater_Output
//============================================
void	Set_Heater_Output( t_sHeater* pHeater, float fHeater_Power_Level ) {

	uint8_t		uPower_Output;

	if ( fHeater_Power_Level <= 0.0f) {

		// clip negative values to zero.
		uPower_Output = 0;

	} else if ( fHeater_Power_Level >= DEW_CAP_HEATER_POWER_OUTPUT_MAX ) {

		// clip positive values to DEW_CAP_HEATER_POWER_OUTPUT_MAX
		uPower_Output = DEW_CAP_HEATER_POWER_OUTPUT_MAX;

	} else {

		uPower_Output = fHeater_Power_Level;
	}

	// Report what's going on...
	IMA_DEBUG_MSG( "Heater Drive is " );
	IMA_DEBUG_MSG_VAL( fHeater_Power_Level, DEC );
	IMA_DEBUG_MSG( ", setting Heater to " );
	IMA_DEBUG_MSG_VAL( uPower_Output, DEC );
	IMA_DEBUG_MSG_LN( "" );

	analogWrite( pHeater->uPin, uPower_Output );
	
}
#endif	//	SERVICE_SENSORS_AND_HEATERS


#ifdef	USE_SSD1306_DISPLAY
//============================================
// Display_Show_Status
//============================================
void  Display_Show_Status( tDisplay_Status display_status ) {

  display.clearDisplay();

  switch( display_status ) {

    case  DISPLAY_STATUS_BOOTING_UP:

      break;
      
    case DISPLAY_STATUS_SHOW_STATUS:

		/* Stuff to show:
			Ambient temp in C
			Ambient RH in %
			Dew Point in C

			Ch0 current temp
			Ch0 output power level

			Ch1 current temp
			Ch1 output power level
		*/
      display.clearDisplay();
  
      display.setTextSize(2); // Draw 2X-scale text
      display.setTextColor( WHITE );
      display.setCursor(0, 16);
      display.print( "  Didn't" );
      display.println( "Find Home" );
      display.display();    // Show initial text
      
      break;
      
    }
  
    display.display();

}
#endif	// USE_SSD1306_DISPLAY

#ifdef USE_SSD1306_DISPLAY
void  Display_Show_Position( uint8_t  pos ) {

  display.clearDisplay();

    display.setTextSize(3); // Draw 2X-scale text
    display.setTextColor( WHITE );
    display.setCursor(10, 16);
    display.print( "Pos: " );
    display.println( pos );

    display.setTextSize( 2 ); // Draw 2X-scale text
    display.setTextColor( WHITE );
    display.setCursor(10, 45);

    switch ( pos ) {
      case 0:
        display.println( "   Open" );
        break;
      case 1:
        display.println( "   Dark" );
        break;
      case 2:
        display.println( " All Pass" );
        break;
      case 3:
        display.println( "   Red" );
        break;
      case 4:
        display.println( "  Green" );
        break;
      case 5:
        display.println( "   Blue" );
        break;
      case 6:
        display.println( "   LPF" );
        break;
      case 7:
        display.println( " Grating" );
        break;
      case 8:
        display.println( " ND Moon" );
        break;
      case 9:
        display.println( "  Aux 1" );
        break;
    }
  
    display.display();

}
#endif	// USE_SSD1306_DISPLAY

