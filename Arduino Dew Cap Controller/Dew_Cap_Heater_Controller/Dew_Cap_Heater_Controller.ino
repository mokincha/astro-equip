// Astronomy Dew Cap HEater Controller
// (c) 2019 M. Okincha

/*  Controller for multiple dew cap heaters
        * Support for two heaters
		* Measures ambient temperature and humidity and computes dew point temp
        * Monitors temperature at each heater
        * PI loop control of heaters
        * Rotary encoder with button used for manual control
        * I2C LCD for status display
*/

#include <avr/power.h> // Comment out this line for non-AVR boards (Arduino Due, etc.)

