myFocuserPro2
(c) Copyright Robert Brown 2014-2019. All Rights Reserved.

YOU MAY NOT SELL CONTROLLERS OR PCB'S BASED ON THIS PROJECT
without express written permission granted from the author. 
Schematics, Code, Firmware, Ideas, Applications, Layout are protected by International Copyright Law.

Permission is NOT granted to any person to redistribute, market, manufacture or sell for commercial gain the myFocuser products, ideas, PCB's, circuits, builds, variations and units described or discussed herein or on this site. Permission is granted for personal and Academic/Educational use only.

THIS MEANS THAT YOU CANNOT RE-DESIGN AND MAKE AVAILABLE KITS OR PCB BASED ON THIS PROJECT AND
OFFER THOSE KITS FOR SALE TO OTHERS. THAT IS A BREACH OF COPYRIGHT.
==============================================================================================================
The firmware zip file contains
 - firmware for all driver boards
 - test programs
 - library files (each in its own folder under "myFocuserPro2libraries" - read how to use these with Arduino IDE
==============================================================================================================
Professional PCB: MYFOCUSERPRO2 PCB CAN BE ORDERED ONLINE

DRV8825HW203-M-MT-F-BT		https://aisler.net/p/DWEURMAC
DRV8825HW203_FIRBT 		https://aisler.net/p/UGHLNFJC	for infra red controller
DRV8825HW203_FRE 		https://aisler.net/p/KMHRZAON	for rotary encoder

A4998-M-MT-F-BT			https://aisler.net/p/RLQDYNPU
DRV8825-M-MT-F-BT		https://aisler.net/p/QVXMBSWW
EASYDRIVER-HW203-M-MT-F-BT	https://aisler.net/p/NCMLHIHM 		
EASYDRIVER-HW203_FRE 		https://aisler.net/p/ISHXZBYS
L293DMINI-M-MT-F-BT		https://aisler.net/p/WXCWWRJH	(not TFT)
L298N-M-MT-F-BT			https://aisler.net/p/QWESFIOS		
L9110S-M-MT-F-BT		https://aisler.net/p/ZSXWHVTG
ULN2003-M-MT-F	 		https://aisler.net/p/DEYGMRQC
TB6612FNG-M-MT-F-BT		https://aisler.net/p/XGJMJQGP

==============================================================================================================
General changes across all firmware versions are listed here

// 2.80
// Add 12V stepper power detect circuit, if 12V power not present stepper will not move when asked to step
// Requires additional circuitry. All Aisler boards changed to add this support dated from 10-Jan-2019
// Serial comms fix for Home Position

// 2.79
// Controller remembers motorspeed
// Please run the ClearEEPROM.ino file on the controller before programming with v279
// Note that running the ClearEEPROM will erase all focuser settings

// 2.78
// Major update - please use with Windows App 2.4.1.9 or higher
This is a MAJOR update.
It is recommended that you follow this procedure
1. Save your focuser settings using the windows application to a file
2. Run the ClearEEPROM ino file in the firmware zip file - \Tests\FocuserUtilPrograms\ClearEEPROM
3. Program the controller with firmware 278
4. Run the windows app, and restore the focuser settings from the file generated in 1. above

What are the changes
Speed improvement.
Better response
Lower memory (ram and eeprom) usage allowing new features to be added later
Add temp compensation direction for some takahaski tubes which require a negative temperature coefficient

As usual, report any issues to me,

// 2.77
// Fixes for speeds s/m/f
// Fix for delay after move

// 2.76 
// Fix bug in tempmode C/F

// 2.75
// Add call to get temperature mode setting, Celsius or Fahrenheight
// Add call to get if there is a temperature probe
// Requires WinApp 2.4.1.1 or higher

// 2.74
// Revert back to move restrictions when in TC mode enabled

// 2.73 
// test more changes to Push Button code
// Added NOKIA 5110 LCD option to L293DMini, ULN2003 and L9110S

// 2.72
// Another change in PB code
// Remove move restrictions when in TC mode enabled

// 2.71
// Small change in push button code

// 2.70
// Combine Move and Home code together, bug fix for backlash issue when Home, and Home followed by Move

// 2.69
// Changes related to compatibility with recent updates to specific client apps

// 2.68 
// Fix backlash issue
// Requires update of Windows application to latest version 2395+ and 2336+

// v2.67
// Rewrite temp routines to remove wait delay
// Remove backlash maxsteps
// changes to push button code
// eeprom updates consolidated into a method
// remove eeprom writes and jogging speed commands
// changes to jogging, infrared and rotary encoder

// v2.66
// Rewrite using defines to make more configurable
// Add checks to prevent home position switch never ending if not detected
// Reformat display output, add LCD1604, display types now LCD1602, LCD1604 and LCD2004
// Allow focuser moves when temperature compensation is enabled to conform to recent ASCOM standard changes

// v2.65
// Ensure firmware matches new Rev3 PCB boards

// v2.64
// Changes to processCommand()
// Changes to clockwise() and anticlockwise()
// Changes to serial events code incl bluetooth
// Fix error in delayaftermove for pushbuttons, ir, jogging, re
// Implement queue for received serial commands
// Changes to LCD Display output
// + Bug fixes to ULN2003 and ULN2003_8S code

// v2.63
// Add Arduino Micro controller support: IL
// Add defines for BUZZER and INOUT LEDS
// Wrap include of OneWire into #define as only use for I2C
// Fix error for nearinghomepos

// v2.62
// Backlash routines added for both IN and OUT independently, either can be enabled/disabled
// Backlash steps are in range 0-50 and specified seperately for IN and OUT
// Backlash settings are remembered by the controller (saved in EEPROM)
// Backlash is NOT applied to push buttons or jogging
// Backlash is not applied when using HOME POSITION switch

// v2.61
// Consolidate check code routines
// Consolidate serial response routines
// myXXXX libraries must be placed in myDocuments/Arduino/libraries
// Complier Define Options now used to generate different firmware versions
// Enable defines for BLUETOOTH, TEMPERATUREPROBE, PUSHBUTTONS, DISPLAY, HOMEPOSITION, ROTARYENCODER, INFRARED
// Reduce memory footprint
// Backlash routines added for both IN and OUT independently, either can be enabled/disabled
// Backlash steps are in range 0-200 and specified seperately for IN and OUT
// Backlash settings are remembered by the controller (saved in EEPROM)
// Backlash is NOT applied to push buttons or jogging
// Backlash is not applied when using HOME POSITION switch

// v2.59 18112017
// Implement DelayAfterMove, provides 0-500ms delay after move and before image is taken so to prevent any blurring of star
// caused by any mechanical residual movement or vibration of the stepper or focuser during focusing

// v2.58
// Fix errors in timer overflows
// Change in temp comp

// v2.57 20072017
// Changes for compatibility with IDE v1.8.3
// Changes to halfstepper code for compatibility with IDE v1.8.3
// Changes to EEPROM.h code for compatibility with IDE v1.8.3
// L293D_F, L293D_M and L293D_MT discontinued as AFMotor Library unsupported now and does not work with IDE v1.8.3
// Suggest users migrate to L293DMINI

// v2.55 23042017
// bug fix for setting maxIncrement

// v2.54 28032017
// Code changes to reduce memory footprint
// Changes to setup() code and add validation on myfocuser EEPROM parameters

// v2.51 31122016
// Fix for bug in home position switch code

// v2.50 ALL VERSIONS 02112016
// Changes to code for Home Position switch (use define's which makes it easier to change code later)
// Changes to Case 3: programShortVersion
// Changed to LCD display page1 (PW and RD moved 1 space to right to accomodate larger values)
// Fix issue with saving and restoring motor speed when jogging
// Fix errors related to motorspeed setting and motor delays

// v2.49_ALL_VERSIONS 29092016
// Code improvements for MaxStep, MaxIncrement and focuser limits, improved exception handling
// Boundary range checks on focuser position, maxstep and maxIncrement on initialization

// v2.48_ALL_VERSIONS 28092016
// Fix for small values of stepsize between 0-1

// v2.47_ALL_VERSIONS 20092016
// Increase maxSteps to a huge number

// v2.46_XXXX_F and _MT 01092016 - All Full versions and versions with Temperature Probe
// Prevent push buttons working when temperature compensation is ON

// v2.45_F FULL VERSIONS ONLY 11082016
// Fix issue of speed of motor - caused by changes in v2.44 to PushButtons

// v2.44_ ALL VERSIONS 01072016
// Small number of fixes related to LED's and PB code

// v2.43_ ALL VERSIONS 01072016
// Fix bug in Coil Power not disabling

// v2.42_ ALL VERSIONS 21062016
// Fix for jogging which is not allowed if in temperature compensation mode

// v2.41_ ALL VERSIONS 16062016
// Added EEPROM_WRITES which keeps a running total for writes to EEPROM

// v2.40_ ALL VERSIONS 01062016
// Fix for temperature precision state not being remembered (init of temp probe must come after init of focuser settings)
// Fix for invalid maxsteps value of 33-------

// v2.39_ ALL VERSIONS 31052016
// Bug fix for fahrenheit (thanks BJ)

// v2.38_ULN2003_F 25052016
// Add commands  for jogging control
// Add jogging code, modify HALT
// Fix for LCD not updating

// v2.37_ULN2003_F 25052016
// New command :64# to step a number of steps (for jog shuttle control)
// Reduce some of delays from home methods to be more responsive and quicker
// fix for timeout when homing
// fix for motorspeed not changing when homing, when begin set from menu, when motorspeedchange is enabled

// v2.36_ULN2003_F 22052016
// Try to compensate for positional error in home position switch - settohome()
// Also modified HOME call to set 0 if a move to home does not actually hit the mechanical switch

// v2.35_ULN2003_F 18052016
// Rewrite of some common code to reduce memory footprint
// Fix fault in Home Position, would not move OUT after homing (workbench was fine, under the stars no
// v2.34 ALL VERSIONS  13052016
// Add lcdupdateonmove, :61, :62 set and get
// Add check status of home switch :63
// Removed numwrites
// Removed :41 to free up space

// v2.32 ALL VERSIONS 09052016
// Fix reset to position 0 on moves

// v2.31 ALL VERSIONS 07052016
// Fixed Home Position switch bug
// Fixed EEPROM write bug

// v2.30 ALL VERSIONS 04052016
// Complete protocol change - only works with Arduino Firmware 230 or higher and ASCOM driver 230 or higher and Windows App 230 or higher
// Fixed error in not saving EEPROM after moves
// Update of focuser position during move deleted (will rewrite this later)
// Fix outliers in temperature compensation in case of huge jump in temperature change (or error in reading)
// Added new features
//    Implement home position switch, on D12, switch sets pin 12 low when activated, pin 12 held high by 470K resistor
//    Get motorspeed
//    Set motorspeedchange value (threshold) when moving - switches to slowspeed when nearing destination
//    Return motorspeedchange value for which stepper slows down at end of its move
//    Enable/Disable motorspeedchange when moving
//    Get status of motorspeedchange enabled/disabled
//    Save settings to EEPROM
