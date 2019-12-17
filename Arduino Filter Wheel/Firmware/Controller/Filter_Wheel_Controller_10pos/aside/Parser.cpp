/*------------------------------------------------------------
| Parser.cpp                  (c) 2011 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE: To provide string parsing functions for a simple
|          command interpreter.
|
| The goal of this module is to convert a command string into
| separate global variables for use by other parts of the
| command interpreter.
|
| DESCRIPTION: Command strings are zero-terminated, of the
| form:
|       <command> <parm1> <parm2> ... <parmN> / <comment>
|
| Examples:      read 12345
|
|                copy x1230 xfffe 120
|
| Command strings may come from any source, such as files or
| a serial port, but this module is only concerned with
| single strings.
|
| Command strings may contain any number of items, delimited
| by one or more whitespace characters. The followings
| characters are recognized as being whitespace: space, tab,
| carriage return, linefeed, formfeed, or control-z.
|
| The first item in a command is a function word. Any items
| that follow are integer numeric parameters.
|
| Two different integer format are supported, hex and decimal.
|
| Hex integers begin with 'x' or 'X' characters, eg: "x1234"
|
| Decimal integers have no base indicator, eg: "1234".
|
| Negative integers are supported, eg: "-567" or "x-573".
|
| The function word is not case significant, so it may be
| be expressed as either upper or lower case characters. A
| function word may contain any non-whitespace characters.
|
| Comments in command strings are ignored. The rule for
| comments is that anything to right of a comment character
| is ignored. The comment character is '/'.
|
| HISTORY:
|   02Feb11 TL From TLParse.c.
|   03Mar11 Added support for signed integer parameters.
------------------------------------------------------------*/

// Making compatible with both Arduino 022 and 100 (1.0)
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif
// This is required for standard Arduino types
// including serial access.

#include "Parser.h"

//------------------------------------------------------------
// INPUTS
//------------------------------------------------------------

char   TheCommandBuffer[COMMAND_BUFFER_SIZE];
// Buffer holding the zero-terminated command string.
//
char** TheFunctionNameTable;
// Address of an array of function name strings, all
// letters in upper case. The first and last entries
// of the array must contain the value 0 as
// placeholders.

//------------------------------------------------------------
// OUTPUTS
//------------------------------------------------------------

int TheFunctionIndex;
// The index of the function just parsed in the
// function name table.
//
// This index is used to locate the address of a
// corresponding procedure that should be called to
// execute the function.
//
char TheFunctionName[FUNCTION_NAME_BUFFER_SIZE];
// The function name as a zero-terminated string.
//
long TheParameters[MAX_PARAMETER_COUNT];
float TheParametersFloat[MAX_PARAMETER_COUNT];

// The function parameters as integer values.
//
// Use these values in function service routines.
//
int TheParameterCount;
// The number of parameters found when parsing the
// command string.
//
// Use this value for error checking in function
// service routines.
//
int TheParsingStatus;
// Status of the command parsing operation: set to
// zero if the command was parsed OK, or to a non-zero
// error code to identify the problem.

/*------------------------------------------------------------
| AddressOfLastCharacterInString
|-------------------------------------------------------------
|
| PURPOSE: To find the address of the last character in a
|          string, or the place where the last character
|          should be if the string is empty.
|
| DESCRIPTION: Returns the address of the byte immediately
| prior to the terminating 0 byte if there are characters in
| the string.
|
| Empty strings return the address of the string terminator.
|
| EXAMPLE:
|          A = AddressOfLastCharacterInString(AString);
|
| ASSUMES: String is terminated with a 0.
|          Characters are single bytes.
|
| HISTORY: 01Feb89
|          08Mar97 changed name from FindLastByteInString();
|                  changed handling of empty string case.
------------------------------------------------------------*/
char* AddressOfLastCharacterInString( char* A ) {
	char* B;

	B = A;

	while ( *B ) B++;

	if ( B > A ) {
		B--;
	}

	return( B );
}

/*------------------------------------------------------------
| ConvertStringToUpperCase
|-------------------------------------------------------------
|
| PURPOSE: To convert a string in place to upper case.
|
| EXAMPLE: ConvertStringToUpperCase( AString );
|
| ASSUMES: An ASCII string terminated with a 0.
|
| HISTORY: 22Mar89
|          12Jan94 replaced toupper.
------------------------------------------------------------*/
void
ConvertStringToUpperCase( char* AString )
{
	char AByte;

///////////
	NextByte://
///////////

	AByte = *AString;

	if ( AByte == 0 ) return;

	if ( AByte >= 'a' && AByte <= 'z' ) {
		*AString = (char) ( AByte - 32 );
	}

	AString++;

	goto NextByte;
}

/*------------------------------------------------------------
| CopyString
|-------------------------------------------------------------
|
| PURPOSE: To copy a zero-terminated string from one place
|          to another.
|
| DESCRIPTION:
|
| EXAMPLE:  CopyString( FromString, ToBuffer );
|
| NOTE: The parameter order for this procedure is just the
|       opposite of that used by strcpy().
|
| ASSUMES: Source and Destination buffers are non-overlapping
|          or Destination is lower in memory than Source.
|
| HISTORY: 14Nov89
|          15Feb93 changed to quad count.
|          01Nov93 return value removed; now buffers must
|                   not overlap.
|          18May01 From TLStrings.c and revised comments.
------------------------------------------------------------*/
void
CopyString( char* Source, char* Destination )
{
	// Until the end of the source string.
	while ( *Source != 0 ) {
		// Copy a byte from the source to the destination.
		*Destination++ = *Source++;
	}

	// Add the string terminator to the destination string.
	*Destination = 0;
}

#ifdef USE_COMMENTS_IN_COMMANDS

/*------------------------------------------------------------
| FindByteInString
|-------------------------------------------------------------
|
| PURPOSE: To find a byte in a given string, if it is there.
|
| DESCRIPTION: Returns address of the byte or 0 if not found.
|
| ASSUMES: String is terminated with a 0.
|
| HISTORY: 12Dec96 from IsByteInString().
------------------------------------------------------------*/
char*
FindByteInString( int SearchByte, char* AString )
{
	char AByte;
	char SByte;

	SByte = (char) SearchByte;

	Another:

	AByte = *AString;

	if ( AByte == SByte ) return( AString );

	if ( AByte == 0 )	  return( 0 );

	AString++;

	goto Another;
}

#endif // USE_COMMENTS_IN_COMMANDS

/*------------------------------------------------------------
| LookUpBestMatchingString
|-------------------------------------------------------------
|
| PURPOSE: To find the best matching string in an array given
| a target string.
|
| DESCRIPTION: Returns the index of the string in the array if
| the string is found, or zero if no matching string is found.
|
| The array of strings must begin and end with placeholder
| entries that contain zero.
|
| The best matching string is the one that has the most
| number of characters in common with the search string.
|
| The letters in both the search string and the array strings
| must be upper case.
|
| EXAMPLE:
|
|    char*
|    CommandList[] =
|    {
|       0
|       "GO",
|       "STOP",
|       "READ",
|       "WRITE",
|       "COPY",
|       0
|    };
|
|    i = LookUpBestMatchingString( CommandList, "COPY" );
|
|    returns i = 5, the index for "COPY"
|
|    i = LookUpBestMatchingString( CommandList, "R" );
|
|    returns i = 3, the index for "READ"
|
| HISTORY:
|   03Feb11 TL from LookUpString().
------------------------------------------------------------*/
// OUT: Index of the best matching string.
int	//
LookUpBestMatchingString( char** AStringList, char* AString )
{
	char* BString;
	int BestMatchingIndex;
	int BestMatchingCharacterCount;
	int i;
	int m;

	// Start with the first, empty, entry as the best
	// matching index.
	BestMatchingIndex = 0;

	// Start with no characters matched.
	BestMatchingCharacterCount = 0;

	// Refer to the first non-blank string in the array
	// using the index 'i'.
	i = 1;

/////////////
	NextString://
/////////////

	// Refer to the current string in the array.
	BString = AStringList[i];

	// If the end of the list has been reached, then return
	// the index of the best matching string, or zero if
	// no matching string has been found.
	if ( BString == 0 ) {
		return( BestMatchingIndex );
	}

	// Count how many of the charcters match in the
	// current string.

	// Initialize the character match counter to zero.
	m = 0;

	// Until either the end of either string is reached.
	while ( AString[m] && BString[m] ) {
		// If the characters match, then increment the
		// match counter.
		if ( AString[m] == BString[m] ) {
			// Advance the match counter.
			m++;
		} else { // Mismatch.
			// Stop matching.
			break;
		}
	}

	// If the current string is a better match than the one
	// found so far, then make this the new best match.
	if ( m > BestMatchingCharacterCount ) {
		BestMatchingCharacterCount = m;
		BestMatchingIndex = i;
	}

	// Advance to the next string in the array.
	i++;

	// Go try matching the next string.
	goto NextString;
}

/*------------------------------------------------------------
| ParseTheCommandString
|-------------------------------------------------------------
|
| PURPOSE: To parse the input command string to globals.
|
| DESCRIPTION: See description at the beginning of this file.
|
| This routine takes TheCommandBuffer[] as input, and it may
| change the contents of that buffer while parsing the
| command to TheFunctionName[] and TheParameters[].
|
| HISTORY:
|   02Feb11 TL
|   03Mar11 Revised to parse signed integer parameters.
|   11Apr11 Deleted StripLeadingWhiteSpace() to save space:
|           ParseWord() ignores leading whitespace already.
|           Changed order of comment and trailing whitespace
|           stripping code because the last parameter parsed
|           by ParseInteger() needs to be terminated with the
|           end-of-string byte rather than any whitespace,
|           otherwise the parameter count will be one more
|           than it should be.
------------------------------------------------------------*/
// OUT: The parsing status result, one of the PARSE_...
int	//      codes.
ParseTheCommandString()
{
	char* AtString;// This is the parsing cursor, the address
				   // of the first byte of the rest of the
				   // string.
				   //
	char* AtStringEnd;
	// Address of terminal zero of the string.

#ifdef DEBUG
//    Serial.println("ParseTheCommandString called.");
#endif

	// Start with no function name parsed.
	TheFunctionName[0] = 0;

	// Start with no integer parameters parsed.
	TheParameterCount = 0;

	// Refer to the first byte of the command string.
	AtString = (char*) TheCommandBuffer;

#ifdef DEBUG
//    Serial.print("TheCommandBuffer = [");
//    Serial.print((const String&)TheCommandBuffer);
//    Serial.println("]");
#endif

#ifdef USE_COMMENTS_IN_COMMANDS
	// Strip any comment characters.
	StripComment( AtString );
#endif

	// Strip any trailing whitespace from the command string.
	StripTrailingWhiteSpace( AtString );

	// Find the address of the last byte of the string, the 0
	// byte.
	AtStringEnd = AtString;
	ToStringTerminator( &AtStringEnd );

	// If the command string is empty, then just return
	// with a status that indicates that the command is
	// a no-op.
	if ( AtString == AtStringEnd ) {
		TheParsingStatus = PARSE_NOP;

		return( PARSE_NOP );
	}

	// At this point it is known that there is at least one
	// word in the command string.

	// Parse the first word to the function name buffer.
	AtString = ParseWord( AtString, (char*) TheFunctionName );

#ifdef DEBUG
//    Serial.print("After ParseWord, TheFunctionName = [");
//    Serial.print((const String&)TheFunctionName);
//    Serial.println("]");
#endif

	// Convert the function name to upper-case letters to
	// make subsequent command matching easier.
	ConvertStringToUpperCase( (char*) TheFunctionName );

#ifdef DEBUG
//    Serial.print("After ConvertStringToUpperCase, TheFunctionName = [");
//    Serial.print((const String&)TheFunctionName);
//    Serial.println("]");
#endif

	// Find index of the function in the table of all
	// recognized functions -- choose the best match,
	// allowing function abbreviation.
	TheFunctionIndex =
	LookUpBestMatchingString(
							TheFunctionNameTable,
							(char*) TheFunctionName );

	// If no matching function was found, then return the
	// appropriate error code.
	if ( TheFunctionIndex == 0 ) {
		TheParsingStatus = PARSE_BAD_FUNCTION;

		return( PARSE_BAD_FUNCTION );
	}

	// Now parse any remaining parameters, presuming them
	// to be unsigned decimal or hex integers.

////////////////
	NextParameter://
////////////////

	// If there is no next word, then just return with a
	// status that indicates the successful parsing of a
	// command.
	if ( AtString == AtStringEnd ) {
		TheParsingStatus = PARSE_OK;

		return( PARSE_OK );
	}

	// Convert the current word to an integer, advancing
	// TheWord cursor.
	TheParametersFloat[TheParameterCount] = ParseFloat( &AtString );
	TheParameters[TheParameterCount] = ParseInteger( &AtString );

#ifdef DEBUG
//   Serial.print( "After ParseInteger, param = [" );
//   print_dec( TheParameters[TheParameterCount] );
//   Serial.println( "]" );
#endif

	// If the cursor is not at a whitespace character or the
	// end of a string, then an error has occurred while
	// parsing the integer -- return the appropriate status.
	if ( *AtString != ' ' && *AtString != 0 ) {
		TheParsingStatus = PARSE_BAD_NUMBER;

		return( PARSE_BAD_NUMBER );
	}

	// Account for having parsed a valid parameter.
	TheParameterCount++;

	// If more parameters remain, but there isn't room to
	// store the converted binary values, then abort with
	// the appropriate error code.
	if ( *AtString &&
		 (TheParameterCount == MAX_PARAMETER_COUNT - 1) ) {
		TheParsingStatus = PARSE_TOO_MANY_PARAMETERS;

		return( PARSE_TOO_MANY_PARAMETERS );
	}

	// Go get the next parameter, if any.
	goto NextParameter;
}

/*------------------------------------------------------------
| ParseInteger
|-------------------------------------------------------------
|
| PURPOSE: To parse a decimal or hexadecimal integer from a
|          string after skipping any white space.
|
| DESCRIPTION: This routine parses an integer string and
| converts it to binary. It supports two different formats of
| integer strings, hex and decimal.
|
| Hex integers begin with 'x' or 'X' characters, eg: "x1234"
|
| Decimal integers have no base indicator, eg: "1234".
|
| Negative numbers are indicated with a minus sign, eg. "-10",
| "-xF13", or "x-FFB", are all valid.
|
| This is how the character cursor behaves:
|
|       Entry:  *x = pointer to ASCII string.
|
|       Exit:   *x = pointer to first character following
|                    the number in the string.
| HISTORY:
|   02Feb11 Revised comments.
|   03Mar11 Added support for negative numbers. Name changed
|           from ParseUnsignedInteger(). Changed return value
|           to a 4-byte long from a 2-byte int.
------------------------------------------------------------*/
// OUT: Binary value of the number string parsed.
long ParseInteger( char** S ) {
	long result;
	char* s;
	char  c;
	char  IsNegative;
	long  Base;
	bool    bFoundDP;

	bFoundDP = false;
	// Set the default number base to decimal.
	Base = 10;

	// Set the flag used to track negative numbers to 0 to
	// mean that the number is not negative.
	IsNegative = 0;

	// Advance the parsing cursor past any leading whitespace.
	SkipWhiteSpace( S );

	// Refer to the string using a local string pointer.
	s = *S;

	// Accumulate the result.
	result = 0;

///////////
	GetAByte://
///////////

	// Get the current character.
	c = *s;

	// if we find a DP, skip everything afterwards
	if ( bFoundDP ) {
		if ( ( c != ' ') && ( c != 0 ) ) {

			s++;
			goto GetAByte;

		}
	} else {
		if ( c == '.' ) {
			bFoundDP = true;

			s++;

			goto GetAByte;
		}
		// If a negative number sign is found, track the
		// presence of a negative number.
		if ( c == '-' ) {
			IsNegative = 1;

			s++;

			goto GetAByte;
		}

		// If the hex number indicator is found.
		if ( c == 'x' || c == 'X' ) {
			Base = 16;

			s++;

			goto GetAByte;
		}

		// If the byte is an ASCII digit.
		if ( c >= '0' && c <= '9' ) {
			result = Base * result + (c - '0');

			s++;

			goto GetAByte;
		}

		// If in base 16.
		if ( Base == 16 ) {
			// If the byte is a lowercase ASCII hex digit.
			if ( c >= 'a' && c <= 'f' ) {
				result = ( result << 4 ) + ( c - 'a' + 10 );

				s++;

				goto GetAByte;
			}

			// If the byte is a uppercase ASCII hex digit.
			if ( c >= 'A' && c <= 'F' ) {
				result = ( result << 4 ) + ( c - 'A' + 10 );

				s++;

				goto GetAByte;
			}
		}
	}

		// Update the external parsing cursor.
		*S = s;

		// If the number is negative, then negate the result.
		if ( IsNegative ) {
			result = -result;
		}

		return( result );
	}

/*------------------------------------------------------------
| ParseFloat
|-------------------------------------------------------------
|
| PURPOSE: To parse a decimal float from a
|          string after skipping any white space.
|
| DESCRIPTION: This routine parses an float string and
| converts it to binary.
|
| This is how the character cursor behaves:
|
|       Entry:  *x = pointer to ASCII string.
|
|       Exit:   *x = pointer to first character following
|                    the number in the string.
| HISTORY:
|   22Jul12 MAO copies from Arduino Stream.cpp
------------------------------------------------------------*/
// OUT: Binary value of the number string parsed.
	float  ParseFloat( char** S ) {
		bool    isNegative = false;
		bool    isFraction = false;
		char*   s;
		char    c;
		long   value = 0;
		float   fraction = 1.0;
		char*   S_saved = *S;

		// Advance the parsing cursor past any leading whitespace.
		SkipWhiteSpace( S );

		// Refer to the string using a local string pointer.
		s = *S;

		// Accumulate the result.
		value = 0;

///////////
		GetAByte:						   //
///////////

		// Get the current character.
		c = *s;

		// If a negative number sign is found, track the
		// presence of a negative number.
		if ( c == '-' ) {
			isNegative = true;
			s++;
			goto GetAByte;
		}

		// look for a decimal point
		if ( c == '.' ) {
			isFraction = true;
			s++;
			goto GetAByte;
		}

		// If the byte is an ASCII digit.
		if ( c >= '0' && c <= '9' ) {
			value = value * 10 + c - '0';
			if ( isFraction )
				fraction *= 0.1;

			s++;

			goto GetAByte;
		}


		// Update the external parsing cursor.
		*S = s;

		// If the number is negative, then negate the result.
		if ( isNegative ) {
			value = -value;
		}

		// restore the pointer
		*S = S_saved;

		if ( isFraction ) {
			return( (float)value * fraction );
		} else {
			return( (float)value );
		}
	}

/*------------------------------------------------------------
| ParseWord
|-------------------------------------------------------------
|
| PURPOSE: To extract a whitespace delimited word from a 0
|          terminated string.
|
| DESCRIPTION: Given source string and destination buffer,
| this procedure moves the first non-whitespace word to the
| buffer and returns the address of the byte immediately
| following the word.
|
| Returns 0 if word not found before end of string.
|
| EXAMPLE:
|
|    NextAddress = ParseWord( SourceBuffer, WordBuffer );
|
|    if SourceBuffer holds: <aaaaa "abdbd" adsfasdf>
|    then after ParseWord, WordBuffer will hold: <aaaaa>0
|    and the returned address will be that of the first space.
|
| NOTE: See also 'ParseBytes' in 'Bytes.c'.
|
| ASSUMES: String is terminated with a 0.
|
| HISTORY:
|   06Apr91
|   23Dec96 added empty string terminator for empty input
|           string condition.
|   15Feb99 Factored out IsWhiteSpace() and IsNotWhiteSpace().
|   15Feb11 Fixed bug where if first byte is non-whitespace
|           it was skipped.
------------------------------------------------------------*/
	char*
	ParseWord( char* Source, char* Target )
	{
		char* Scan;
		char  b;

		Scan = Source;

///////////
		NextByte://
///////////

		// Get a byte and advance the character pointer.
		b = *Scan;

		// If the end of the string has not been reached.
		if ( b ) {
			// If this is a white space character.
			if ( IsWhiteSpace( b ) ) {
				// Advance to the next byte.
				Scan++;

				// Go get the next byte.
				goto NextByte;
			}
		} else // End of string has been reached without
		// getting any word.
		{
			// Mark the result string as empty.
			*Target = 0;

			// Signal that no word was found.
			return( 0 );
		}

/////////////////
		NextByteOfWord://
/////////////////

		// Get a byte.
		b = *Scan;

		// If the end of the string has not been reached.
		if ( b ) {
			// If this is a white space character.
			if ( IsWhiteSpace( b ) ) {
				// Go finish up.
				goto Finish;
			} else { // Not whitespace.
				// Copy the byte to the target.
				*Target++ = b;

				// Advance to the next character.
				Scan++;

				// Go get the next byte of the word.
				goto NextByteOfWord;
			}
		}

/////////
		Finish://
/////////

		// Mark the end of the result string.
		*Target = 0;

		// Return the address of the first byte after the word.
		return( Scan );
	}

/*------------------------------------------------------------
| SkipWhiteSpace
|-------------------------------------------------------------
|
| PURPOSE: To advance a parsing cursor past white space
|          characters.
|
| EXAMPLE:  SkipWhiteSpace(&At);
|
| HISTORY: 25Dec96
------------------------------------------------------------*/
	void
	SkipWhiteSpace( char** Here )
	{
		char* At;
		char  c;

		At = *Here;

///////////
		GetAByte://
///////////

		c = *At;

		if ( IsWhiteSpace( c ) ) {
			At++;

			goto GetAByte;
		}

		*Here = At;
	}

#ifdef USE_COMMENTS_IN_COMMANDS

/*------------------------------------------------------------
| StripComment
|-------------------------------------------------------------
|
| PURPOSE: To strip comment characters from a zero-terminated
|          string.
|
| DESCRIPTION: Comments in command strings are ignored. The
| rule for comments is that anything to right of a comment
| character is ignored. The comment character is '/'.
|
| HISTORY:
|   01Feb11 TL
------------------------------------------------------------*/
	void
	StripComment( char* Buffer )
	{
		char* C;

		// Find the character delimiter '/' if any.
		C = FindByteInString( (int) '//', Buffer );

		// If a comment character was found, then replace it with
		// a zero string terminator byte.
		if ( C ) {
			*C = 0;
		}
	}

#endif // USE_COMMENTS_IN_COMMANDS


/*------------------------------------------------------------
| StripTrailingWhiteSpace
|-------------------------------------------------------------
|
| PURPOSE: To strip "white space" characters (defined below)
|          from the end of a 0 terminated string.
|
| DESCRIPTION: White space characters are any of the
| following:
|
|           spaces, tabs, carriage returns, line feeds
|
| EXAMPLE: StripTrailingWhiteSpace(MyInputBuffer);
|
| ASSUMES: String is terminated with a 0.
|
| HISTORY: 25Mar91
|          07Mar97 revised to handle empty string case.
------------------------------------------------------------*/
	void
	StripTrailingWhiteSpace( char* Buffer )
	{
		char* A;
		char  a;

		A = AddressOfLastCharacterInString( Buffer );

		Another:

		// Get the last character in the string.
		a = *A;

		// If the character is a whitespace character then
		// replace it with a zero.
		if ( IsWhiteSpace( a ) ) {
			*A = 0;
		} else { // Not whitespace, so return.
			return;
		}

		// If the beginning of the string hasn't been reached,
		// try another character.
		if ( A > Buffer ) {
			A--;

			goto Another;
		}
	}

/*------------------------------------------------------------
| ToStringTerminator
|-------------------------------------------------------------
|
| PURPOSE: To advance a parsing cursor to the terminating zero
|          of a string.
|
| DESCRIPTION:
|
| EXAMPLE:  ToStringTerminator(&At);
|
| HISTORY: 03Jan97 from ToWhiteSpace().
------------------------------------------------------------*/
	void
	ToStringTerminator( char** Here )
	{
		char* At;

		At = *Here;

		while ( *At ) {
			At++;
		}

		*Here = At;
	}


	
