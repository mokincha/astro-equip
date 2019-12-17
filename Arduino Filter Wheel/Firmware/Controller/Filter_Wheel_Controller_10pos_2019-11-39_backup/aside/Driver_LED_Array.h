/*------------------------------------------------------------
| DRIVER_AD5066_ARRAY.h			(c) 2012 Peripheral Vision, Inc.
|-------------------------------------------------------------
|
| PURPOSE:
|
| DESCRIPTION: See lamp_controller.cpp for more.
|
| HISTORY:
|	18Jun12	MO
------------------------------------------------------------*/

#ifndef DRIVER_LED_ARRAY_H
#define DRIVER_LED_ARRAY_H

#include "Driver_AD5066_Array.h"

//------------------------------------------------------------
// Constants
//------------------------------------------------------------
#define LED_ARRAY_NUM_CHANNELS					20
#define LED_ARRAY_NUM_BUFFERS					16
#define LED_ARRAY_NUM_SEQUENCE_BUFFER_LENGTH	64
#define	LED_ARRAY_NUM_SEQUENCE_BUFFER_ID_EOF		( LED_ARRAY_NUM_BUFFERS + 1 )
#define	LED_ARRAY_NUM_SEQUENCE_BUFFER_ID_RESTART	( LED_ARRAY_NUM_BUFFERS + 2 )

//------------------------------------------------------------
// Structures
//------------------------------------------------------------

typedef	uint8_t	tBuffer_ID_t;
typedef	uint8_t	tChannel_ID_t;
typedef	uint16_t	tChannel_Level_t;
typedef	uint16_t	tSequence_Entry_t;

typedef	struct	{
	tBuffer_ID_t	buffer_ID;
	uint32_t		timestamp;
} sSequence_Entry;


//------------------------------------------------------------
// Class Definition
//------------------------------------------------------------
class	Driver_LED_Array_Class {

	// Public Methods
public:
	void	Init( void );
	uint8_t		Get_Buffer_Count( void ) { return LED_ARRAY_NUM_BUFFERS; };
	uint8_t		Get_Channel_Count( void ) { return LED_ARRAY_NUM_CHANNELS; };

	// Global Operations
	void		Set_LED_Power_Enable( bool bEnable_param );
	bool		Get_LED_Power_Enable( void ) { return bLED_Power_Enable; };

	void		Set_Global_Level( uint16_t uGlobal_Level_param );
	uint16_t	Get_Global_Level( void ) { return uGlobal_Level; };
	void		Set_Modulation_Level( float fModulation_Level_param );

	// Buffer operations.
	void		Set_Selected_Buffer( tBuffer_ID_t uBufferID );
	tBuffer_ID_t	Get_Selected_Buffer( void ) { return uActive_Buffer; };
	void		Set_Buffer_Channel_Level( tChannel_ID_t uChannel, tChannel_Level_t uLevel );
	tChannel_Level_t	Get_Buffer_Channel_Level( tChannel_ID_t uChannel ) { return uLevel_Buffer[ uActive_Buffer ][ uChannel ]; };

	// Sequence operations
	void		Set_Sequence_Entry( tSequence_Entry_t tSequence_Entry, tBuffer_ID_t uBufferID, uint32_t uTimestamp );
	tBuffer_ID_t	Get_Sequence_Entry_Buffer_ID( tSequence_Entry_t tSequence_Entry ) { return asEvent_Sequence[ tSequence_Entry ].buffer_ID; };
	uint32_t	Get_Sequence_Entry_Timestamp( tSequence_Entry_t tSequence_Entry ) { return asEvent_Sequence[ tSequence_Entry ].timestamp; };
	void		Clear_Sequence( void );
	void		Run_Sequence( void );

	// Strobe Controls
	void		Fire_Strobe( void );
	void		Set_Strobe_Params( tBuffer_ID_t uBufferID, uint32_t uFire_Period );
	tBuffer_ID_t	Get_Strobe_Buffer_ID( void ) { return tStrobe_Buffer_ID; };
	uint32_t	Get_Strobe_Period_in_us( void ) { return ulStrobe_Period_in_us; };

	// Output control
	void		Apply_Buffer_to_Active( tBuffer_ID_t uBufferID );
	tChannel_Level_t	Get_Actual_Channel_Level( tChannel_ID_t uChannel ) { return uActive_Level[ uChannel ]; };
	void		Write_Active_to_DAC( void );
	void		Set_Auto_Output_Update( bool bHold );
	bool		Get_Auto_Output_Update( void ) { return bAuto_Output_Update; };

	// DAC Access.  These are mainly for debugging
	void		Write_DAC_Channel_Unbuffered( tChannel_ID_t uChannel, tChannel_Level_t uLevel );

private:


	// Private methods
	void	Update_Channels( void );

	// private member variables
	bool	bLED_Power_Enable;
	bool	bAuto_Output_Update;
	uint16_t	uGlobal_Level;
	float	fModulation_Level;

	sSequence_Entry	asEvent_Sequence[ LED_ARRAY_NUM_SEQUENCE_BUFFER_LENGTH ];

	// Buffer variables
	tBuffer_ID_t		uActive_Buffer;
	uint16_t	uLevel_Buffer[ LED_ARRAY_NUM_BUFFERS ][ LED_ARRAY_NUM_CHANNELS ];
	uint16_t	uActive_Level[ LED_ARRAY_NUM_CHANNELS ];

	// Strobe variables
	tBuffer_ID_t	tStrobe_Buffer_ID;
	uint32_t		ulStrobe_Period_in_us;
	
	// device
	Driver_AD5066_Array_Class	Driver_AD5066_Array;		// Global instantiation of the class

};

#endif // DRIVER_LED_ARRAY_H

