// Astronomy Filter Wheel Controller
// (c) 2019 M. Okincha

/*  Controller for a motorized telescope filter wheel
        * stepper motor driven by DRV8825 driver IC
        * 10 positions
        * home and position photosensors
        * ASCOM-compatible interface
        * rotary encoder with button used for manual control
        * I2C LCD for status and position display
*/

#include <avr/power.h> // Comment out this line for non-AVR boards (Arduino Due, etc.)

