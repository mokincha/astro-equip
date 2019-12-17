myFocuserPro2 Arduino Firmware Changes

// (c) Copyright Robert Brown 2014-2016. All Rights Reserved.
// The schematic, code and ideas for myFocuser are released into the public domain. Users are free to implement
// these but may NOT sell this project or projects based on this project for commercial gain without express 
// written permission granted from the author. Schematics, Code, Firmware, Ideas, Applications, Layout are 
// protected by Copyright Law.
// Permission is NOT granted to any person to redistribute, market, manufacture or sell for commercial gain 
// the myFocuserPro2 products, ideas, circuits, builds, variations and units described or discussed herein or 
// on this site.
// Permission is granted for personal and Academic/Educational use only.

This folder contains a number of Utility programs used for testing purposes

ClearEEPROM
Clears (writes to) all of the EEPROM locations. Useful if having strange errors of values not making an sense.
Do not run this repeatedly, only when there is an issue or asked to do this.
Remember that EEPROM writes are specified as about 10,000 writes.  Use the Arduino Serial monitor program to view the program output.

LCD2004Test1 - LCD2004Test6
Various LCD test programs for use with LCD2004.  Use the Arduino Serial monitor program to view the program output.

Sizeof
Returns the size of the myfocuser struct variable (Full, Minimal and MT). Use the Arduino Serial monitor program to view the program output.
Also makes prediction on how long the EEPROM might last.
