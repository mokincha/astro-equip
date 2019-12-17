/*------------------------------------------------------------
| Command_Lne_Interface.h			(c) 2016 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION: See Fluorescent_T5HO_Light_Controller.cpp for more.
|
| HISTORY:
|   23Feb12 MO
|	22Mar12	MO Adding state output pins
------------------------------------------------------------*/

#ifndef COMMAND_LINE_INTERFACE_H
#define COMMAND_LINE_INTERFACE_H

//------------------------------------------------------------
// Compile Options
//------------------------------------------------------------
//#define DEBUG // Define this symbol to enable debug code,
              // or comment it out to disable debug code.


#ifdef __cplusplus
extern "C"
{
#endif


// debug output settings
#ifdef	DEBUG
	#define IMA_DEBUG // Define this symbol to enable debug code,
#else
//	#define IMA_DEBUG // Define this symbol to enable debug code,
              // or comment it out to disable debug code.
#endif

#ifdef  IMA_DEBUG
	#define IMA_DEBUG_MSG_LN(value) Serial.println( F(value) )
	#define IMA_DEBUG_MSG(value) Serial.print( F(value) )
	#define IMA_DEBUG_MSG_VAL(value, format) Serial.print( value, format )
#else
    #define IMA_DEBUG_MSG_LN(value)
    #define IMA_DEBUG_MSG(value)
	#define IMA_DEBUG_MSG_VAL(value, format)
#endif

// These are normal string-printing macros, not debug output
#define IMA_MSG(value) Serial.print( F(value) )
#define IMA_MSG_LN(value) Serial.println( F(value) )
#define IMA_MSG_VAL(value, format) Serial.print( value, format )



//------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------
void	Command_Line_Interface_Init( void );
void	Command_Line_Interface_Service( void );

bool	Command_Line_Interface_Param_Count_Check( uint8_t uMin_Count, uint8_t uMax_Count );
bool	Command_Line_Interface_Param_Integer_Value_Check( uint8_t uParam_ID, uint32_t uMin_Value, uint32_t uMax_Value );
bool	Command_Line_Interface_Param_Float_Value_Check( uint8_t uParam_ID, float fMin_Value, float fMax_Value );

// COMMAND FUNCTIONS - System Functions
void	CmdHandler_HELP( void );
void	CmdHandler_IDN( void );
void	CmdHandler_SW_RESET( void );

// SYSTEM INFO Commands
void	CmdHandler_GET_FIRMWARE_VERSION( void );
void	CmdHandler_SET_SERIAL_NUMBER( void );
void	CmdHandler_GET_SERIAL_NUMBER( void );

void	CmdHandler_GET_BUFFER_COUNT( void );
void	CmdHandler_GET_CHANNEL_COUNT( void );

// Global commands
void	CmdHandler_SET_LED_POWER_ENABLE( void );
void	CmdHandler_GET_LED_POWER_ENABLE( void );
void	CmdHandler_SET_GLOBAL_LEVEL( void );
void	CmdHandler_GET_GLOBAL_LEVEL( void );

// 	Buffer functions
void	CmdHandler_SET_SELECTED_BUFFER( void );
void	CmdHandler_GET_SELECTED_BUFFER( void );
void	CmdHandler_SET_BUFFER_LEVEL( void );
void	CmdHandler_GET_BUFFER_LEVEL( void );
void	CmdHandler_SET_BUFFER_LEVEL_ALL( void );

// Channel output control
void	CmdHandler_SET_AUTO_OUTPUT_UPDATE( void );
void	CmdHandler_GET_AUTO_OUTPUT_UPDATE( void );
void	CmdHandler_APPLY_BUFFER_TO_OUTPUT( void );

void	CmdHandler_GET_ACTIVE_LEVEL( void );
void	CmdHandler_SET_SINGLE_CHANNEL_UNBUFFERED( void );

void	CmdHandler_UPDATE_OUTPUT( void );

void	CmdHandler_APPLY_BUFFER( void );

// 	COMMAND FUNCTIONS - Function Generator
void	CmdHandler_SET_LAMP_FREQUENCY( void );
void	CmdHandler_GET_LAMP_FREQUENCY( void );
void	CmdHandler_SET_LAMP_MODULATION_DEPTH( void );
void	CmdHandler_GET_LAMP_MODULATION_DEPTH( void );
void	CmdHandler_SET_LAMP_WAVEFORM( void );
void	CmdHandler_GET_LAMP_WAVEFORM( void );


// 	Seqeuence functions
void	CmdHandler_SET_SEQUENCE_ENTRY( void );
void	CmdHandler_GET_SEQUENCE_ENTRY( void );
void	CmdHandler_RUN_SEQUENCE( void );


// 	LED Strobe functions
void	CmdHandler_SET_STROBE_PARAMS( void );
void	CmdHandler_GET_STROBE_PARAMS( void );
void	CmdHandler_FIRE_STROBE( void );

// 	I/O Jack Functions
void	CmdHandler_SET_JACK_OUTPUT( void );
void	CmdHandler_GET_JACK_OUTPUT( void );
void	CmdHandler_READ_JACK_LEVEL( void );


// private helper functions
void    Save_To_EEPROM( void* pSource, int iSizeInBytes, unsigned int uiEEPROMStartAddr );
void    Load_From_EEPROM( void* pDest, int iSizeInBytes, unsigned int uiEEPROMStartAddr );

#ifdef __cplusplus
} // extern "C"
#endif

#endif // COMMAND_LINE_INTERFACE_H

