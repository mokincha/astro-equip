# astro-equip
Astronomy equipment designs

## Dew Cap Heater Controller
This is a two-channel controller for dew cap heaters.  The controller uses an ambient temperature and humidyt sensor to compute the dew point temperature and automatically set the channels dew-free temperature.  Each channel used PWM to drive a 12V heater element.  A seperate temperature sensor for each channel and PID loop set the heater power level and regulate the len's temperature.

## Focuser Controller
This is a general-purpose Arduino-based focus motor controller.  The focuser is based on someone else's project.  I modified the source code for my command language, pinouts, and features.  The project also includes  source code for an ASCOM-compatible Windows driver.

## Focuser for Celestron 8SE
This is a mech design for a stepper motor-based focus motor controller for the Celestron 8SE telescope. All parts are 3D-printable.

## Filter Wheel Controller
This is an Arduino-based filter wheel controller.  There are mech designs for a 6-position wheel for 2" filters and a 10-position wheel for 1.25" filters.  The controller uses home and position sensors and drivers a stepper motor to turn the wheel.
