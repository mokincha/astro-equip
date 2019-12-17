/*------------------------------------------------------------
| CommandParser.cpp           (c) 2011 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE: To provide command parser functions.
|
| DESCRIPTION: This is a template application that shows how
| to implement a serial command-line parser.
|
| HISTORY:
|   15Feb11 TL
------------------------------------------------------------*/

#include "Arduino.h"

#include "CommandParser.h"
#include "Parser.h"

//#include "HardwareSerial.h"
//ADDED FOR COMPATIBILITY WITH WIRING

//------------------------------------------------------------
// Command parser global variables - see also Parser.cpp.
//------------------------------------------------------------

int TheCommandLength;
// The number of characters in TheCommandBuffer[].

#ifdef USE_SCROLLBACK_BUFFER

char TheScrollBackBuffer[COMMAND_BUFFER_SIZE];
// Buffer for the last command entered. This is to
// support a one-line scroll-back feature for the
// command interpreter.

int TheScrollBackLength;
// The number of characters in TheScrollBackBuffer[].

#endif // USE_SCROLLBACK_BUFFER


// pointer to the command string list
char** CommandListPtr;

// pointer to the command function table
Command*	TheCommandHandlerTable;


/*------------------------------------------------------------
| setup
|-------------------------------------------------------------
|
| PURPOSE: CommandParserInit
|
| DESCRIPTION:
|
| HISTORY:
|   05Feb11 TL
|	19Jan12 MO - Adding I2C master commands
|	24Jan12 MO - Breaking out from setup()
------------------------------------------------------------*/
void CommandParserInit( char* command_list[], Command* command_fn_table ) {


	CommandListPtr = command_list;
	TheCommandHandlerTable = command_fn_table;

	// set up the I2C Bus

	// Configure the command string parser to look for the
	// instrument commands.
	TheFunctionNameTable = command_list;

	// Start with TheCommandBuffer[] empty of all characters.
	TheCommandLength = 0;

	// Output the first command prompt.

	Serial.print(">");
}

/*------------------------------------------------------------
| CommandParserService
|-------------------------------------------------------------
|
| PURPOSE: Service the command handler
|
| DESCRIPTION:
|
| HISTORY:
|   05Feb11 TL
|   18Feb11 Added scroll-back buffer, prompt cursor.
|   21Feb11 Added DoMonochromator_BUTTONs().
|   25Feb11 Added command-specific parse error reporting.
------------------------------------------------------------*/
void    CommandParserService( void ) {
	
	int  IncomingByte;

//////////////
	NextCharacter:
//////////////

	// Poll and respond to all the monochromator buttons such
	// as Forward, Reverse, and Stop.
//    DoMonochromator_BUTTONs();

	// If a byte is waiting to be read, then process it.
	if ( Serial.available() > 0 ) {
		// Read the first byte of incoming serial data if
		// available, or return -1 if no data is available.
		IncomingByte = Serial.read();

#ifdef    USE_SCROLLBACK_BUFFER
		// If the incoming byte was 'z', (MNEMONIC: the LAST
		// letter of the alphabet gets you the last command),
		// then interpret that as an instruction to reprint
		// the last command from the scroll-back buffer.
		if ( IncomingByte == (int) 'z' ) {
			// Move the terminal emulator cursor back one position.
			Serial.write(8);

			// Print a space to erase the '-' on the terminal
			// emulator.
			Serial.write(32);

			// Move the terminal emulator cursor back one position.
			Serial.write(8);

			// Copy the command string to the one-line
			// scroll-back buffer.
			CopyString( (char*) TheScrollBackBuffer,
						(char*) TheCommandBuffer );

			// Update the length of the current command to
			// be the same as the last command.
			TheCommandLength = TheScrollBackLength;

			// Show the user the command, printing it on
			// a new line, leaving the cursor at the end
			// of the command to allow for changes.
			Serial.print(">");

			Serial.write( (const uint8_t*) TheCommandBuffer, TheCommandLength );

			// Go wait for the next character.
			goto NextCharacter;
		}
#endif    // USE_SCROLLBACK_BUFFER

		// If the incoming character was a backspace (8) or
		// delete (127) character, then trim off one character from
		// the command buffer.
		if ( IncomingByte == 8 || IncomingByte == 127 ) {
			// If not at the beginning of a command string, then
			// trim off the last character, overwriting it with
			// a space.
			if ( TheCommandLength > 0 ) {
				// Local echo moves character back one position on the
				// terminal emulator.

				// Print a space to erase the character on the terminal
				// emulator.
				Serial.write(32);

				// Move the terminal emulator cursor back one position
				// to compensate for the space just written.
				Serial.write(8);

				// Decrement the command character counter.
				TheCommandLength--;

				// Put a space into the command buffer at the cursor.
				TheCommandBuffer[TheCommandLength] = ' ';
			}

			// Go wait for the next character.
			goto NextCharacter;
		}

		// Append the character to the command buffer.
		TheCommandBuffer[TheCommandLength] = (char) IncomingByte;

		// Advance the command character counter.
		TheCommandLength++;

		// If the byte is a command delimiter character, then
		// process the command.
		if ( IncomingByte == LineFeed ||
			 IncomingByte == CarriageReturn ) {
			// Mark the end of the command string with a
			// zero byte.
			TheCommandBuffer[TheCommandLength] = 0;

#ifdef    USE_SCROLLBACK_BUFFER
			// Copy the command string to the one-line
			// scroll-back buffer.
			CopyString( (char*) TheCommandBuffer,
						(char*) TheScrollBackBuffer );

			// Save the length of the scroll-back command.
			TheScrollBackLength = TheCommandLength;
#endif    // USE_SCROLLBACK_BUFFER

			// Parse the command string.
			ParseTheCommandString();

			// Advance the cursor to the next line before
			// interpreting the command to preserve the command.
//            Serial.println();

			// If an executable command was parsed OK, then
			// run it.
			if ( TheParsingStatus == PARSE_OK ) {
				(*TheCommandHandlerTable[TheFunctionIndex])();

				// If there was a command-specific parsing
				// error, then report it.
				if ( TheParsingStatus != PARSE_OK ) {
					ReportParsingError();
				}
			}
			else {					 // There was a parsing error, so report it.
				ReportParsingError();
			}

			Serial.print(">");

			// Empty the command buffer.
			TheCommandLength = 0;

			// Output a command prompt.
		}

		// Echo the incoming printable character to the serial
		// port so the user can see what he typed in remote
		// echo mode. Comment this out if the terminal
		// emulator is configured to use local echo.
		//
		//Serial.print( IncomingByte, BYTE );

		// If the end of the command buffer has been reached,
		// then report this as a parsing error.
		if ( TheCommandLength >= COMMAND_BUFFER_SIZE ) {
			Serial.println("Command too long");

			// Empty the command buffer.
			TheCommandLength = 0;

			// Output a command prompt.
			Serial.print(">");
		}
	}

}


/*------------------------------------------------------------
| ReportParsingError
|-------------------------------------------------------------
|
| PURPOSE: To tell the user about a parsing error.
|
| DESCRIPTION: This routine gets called any time a command
| string is parsed without a valid command being found.
|
| HISTORY:
|   06Feb11 TL
|   25Feb11 Added PARSE_PARAMETER_OUT_OF_BOUNDS.
|   28Feb11 Added PARSE_MISSING_PARAMETER.
------------------------------------------------------------*/
void    ReportParsingError() {
#ifdef SAVE_SPACE
	Serial.println( "?" );
#else
	switch ( TheParsingStatus ) {
		case PARSE_BAD_FUNCTION:
			Serial.println( "Command not recognized" );
			break;

		case PARSE_BAD_NUMBER:
			Serial.println( "Parameter isn't a number" );
			break;

		case PARSE_MISSING_PARAMETER:
			Serial.println( "Too few parameters" );
			break;

		case PARSE_TOO_MANY_PARAMETERS:
			Serial.println( "Too many parameters" );
			break;

		case PARSE_PARAMETER_OUT_OF_BOUNDS:
			Serial.println( "Parameter out of bounds" );
			break;

		case PARSE_NOP:
			// If no function was found in the command
			// string, but no error was detected either,
			// then just do nothing.
			//
			// This is here as a stub for what should be
			// done if a comment line has been parsed
			// from a file - it should be ignored rather
			// than be flagged as an invalid command.
			Serial.println( "" );
			break;

		default:
			break;
	}
#endif
}

/*------------------------------------------------------------
| print_dec
|-------------------------------------------------------------
|
| PURPOSE: To print a long value to the serial port as a
|          decimal value.
|
| DESCRIPTION: This routine saves 12 bytes per usage when
| used in place of "Serial.print( v, DEC );".
|
| HISTORY:
|   11Apr11 TL
------------------------------------------------------------*/
void    print_dec( long v ) {
	Serial.print( v, DEC );
}

/*------------------------------------------------------------
| println_dec
|-------------------------------------------------------------
|
| PURPOSE: To print a long value to the serial port as a
|          decimal value followed by a linefeed.
|
| DESCRIPTION: This routine saves 12 bytes per usage when
| used in place of "Serial.println( v, DEC );".
|
| HISTORY:
|   11Apr11 TL
------------------------------------------------------------*/
void    println_dec( long v ) {
	Serial.println( v, DEC );
}


