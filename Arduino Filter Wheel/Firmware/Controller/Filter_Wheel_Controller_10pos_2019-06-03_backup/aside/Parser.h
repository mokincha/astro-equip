/*------------------------------------------------------------
| Parse.h                     (c) 2011 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE: To provide string parsing functions for a simple
|          command interpreter.
|
| DESCRIPTION: See Parse.cpp for more.
|
| HISTORY:
|   02Feb11 TL
------------------------------------------------------------*/

#ifndef PARSER_H
#define PARSER_H

//#define DEBUG // Define this symbol to enable debug code,
              // or comment it out to disable debug code.


#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------------------------------
// PARSE STATUS - Result codes for parsing a command string,
// values used in the variable TheParsingStatus.
//------------------------------------------------------------
#define PARSE_OK 0
            // A command was parsed OK, meaning that no syntax
            // violations were found and a valid command was
            // located in TheFunctionNameTable[].
            //
            // No command-specific parameter validation has
            // been done, just general numeric systax
            // checking.
            //
#define PARSE_NOP 1
            // No function name was found in the command
            // string, so treat this as a no-op command during
            // function evaluation.
            //
#define PARSE_BAD_NUMBER 2
            // While converting a parameter to binary, an
            // invalid character was found. Only integer
            // digits are supported for numbers.
            //
#define PARSE_TOO_MANY_PARAMETERS 3
            // Too many numeric parameters were found while
            // parsing a command string.
            //
#define PARSE_BAD_FUNCTION 4
            // The function name given is invalid, not
            // being found in TheFunctionNameTable[].
            //
#define PARSE_MISSING_PARAMETER 5
            // A command has been parsed, but an expected
            // parameter has not been found. This error code
            // is only generated from inside individual
            // command handler routines.
            //
#define PARSE_PARAMETER_OUT_OF_BOUNDS 6
            // A command has been parsed with a parameter, but
            // the parameter is out of bounds. This error code
            // is only generated from inside individual
            // command handler routines.

//------------------------------------------------------------

// Ascii codes by name:
#define    Tab               9
#define    LineFeed         10
#define    FormFeed         12
#define    CarriageReturn   13
#define    ControlZ         26 //  eof marker for DOS
#define    Space            32

// The ordering of comparison tests here is based on
// expected frequency.
#define IsWhiteSpace(a) \
            ( (a) == Space          || \
              (a) == Tab            || \
              (a) == CarriageReturn || \
              (a) == LineFeed       || \
              (a) == FormFeed       || \
              (a) == ControlZ )

//------------------------------------------------------------
// CONFIGURATION
//------------------------------------------------------------

#define COMMAND_BUFFER_SIZE 64
            // The maximum length of a command string in
            // bytes, including the terminal zero byte.

#define FUNCTION_NAME_BUFFER_SIZE 16
            // The maximum length of a function name string in
            // bytes, including the terminal zero byte.

#define MAX_PARAMETER_COUNT 6
            // The maximum number of parameters that can be
            // in a command string.

//#define USE_COMMENTS_IN_COMMANDS
// Define this to support the use of comments in commands.

//------------------------------------------------------------
// INPUTS
//------------------------------------------------------------

extern char TheCommandBuffer[COMMAND_BUFFER_SIZE];
        // Buffer holding the zero-terminated command string.
        //
extern char** TheFunctionNameTable;
        // Address of an array of function name strings, all
        // letters in upper case. The first and last entries
        // of the array must contain the value 0 as
        // placeholders.

//------------------------------------------------------------
// OUTPUTS
//------------------------------------------------------------

extern int TheFunctionIndex;
        // The index of the function just parsed in the
        // function name table.
        //
        // This index is used to locate the address of a
        // corresponding procedure that should be called to
        // execute the function.
        //
extern char TheFunctionName[FUNCTION_NAME_BUFFER_SIZE];
        // The function name as a zero-terminated string.
        //
extern long TheParameters[MAX_PARAMETER_COUNT];
extern float TheParametersFloat[MAX_PARAMETER_COUNT];
        // The function parameters as integer values.
        //
        // Use these values in function service routines.
        //
extern int TheParameterCount;
        // The number of parameters found when parsing the
        // command string.
        //
        // Use this value for error checking in function
        // service routines.
        //
extern int TheParsingStatus;
        // Status of the command parsing operation: set to
        // zero if the command was parsed OK, or to a non-zero
        // error code to identify the problem.

//------------------------------------------------------------

char* AddressOfLastCharacterInString( char* A );
void  ConvertStringToUpperCase( char* AString );
void  CopyString( char* Source, char* Destination );
int   LookUpBestMatchingString( char** AStringList, char* AString );
int   ParseTheCommandString();
long  ParseInteger( char** S );
float  ParseFloat( char** S );
char* ParseWord( char* Source, char* Target );
void  SkipWhiteSpace( char** Here );
void  StripTrailingWhiteSpace( char* Buffer );
void  ToStringTerminator( char** Here );

#ifdef USE_COMMENTS_IN_COMMANDS
char* FindByteInString( int SearchByte, char* AString );
void  StripComment( char* Buffer );
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // PARSER_H

