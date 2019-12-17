/*------------------------------------------------------------
| CommandParser.h             (c) 2011 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE: To provide command parser functions.
|
| DESCRIPTION: This is a template application that shows how
| to implement a serial command-line parser.
|
| HISTORY:
|   19Jan12 MO

------------------------------------------------------------*/

#ifndef	COMMANDPARSER_H
#define	COMMANDPARSER_H

#ifdef __cplusplus
extern "C"
{
#endif

//#define	USE_SCROLLBACK_BUFFER

//#define DEBUG // Define this symbol to enable debug code,
// or comment it out to disable debug code.


//#define VERBOSE
#ifdef  VERBOSE
    #define VERBOSE_MSG(value) Serial.println( value )
#else
    #define VERBOSE_MSG
#endif



//------------------------------------------------------------
// Constants
//------------------------------------------------------------



//------------------------------------------------------------
//	Types
//------------------------------------------------------------

typedef	void (*Command)( void );



//------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------
void	CommandParserInit( char* command_list[], Command* command_fn_table );
void	CommandParserService( void );

void	ReportParsingError( void );
void	print_dec( long v );
void	println_dec( long v );

#ifdef __cplusplus
} // extern "C"
#endif


#endif // COMMANDPARSER_H

