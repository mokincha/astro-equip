/*
 * Simple demo, should work with any driver board
 *
 * Connect STEP, DIR as indicated
 *
 * Copyright (C)2015-2017 Laurentiu Badea
 *
 * This file may be redistributed under the terms of the MIT license.
 * A copy of this license has been included with this distribution in the file LICENSE.
 */
#include <Arduino.h>


// All the wires needed for full functionality
#define PIN_SENSOR_LED          A2
#define PIN_SENSOR_POSITION     A0
#define PIN_SENSOR_HOME         A1

#define SENSOR_HOME_THRESHOLD   167
#define SENSOR_POSITION_THRESHOLD   170


void setup() {

  Serial.begin(9600);
  
  pinMode( PIN_SENSOR_LED, OUTPUT );
  digitalWrite( PIN_SENSOR_LED, HIGH );
  
  }

void loop() {

  uint16_t  uHome_Sensor_Reading;
  uint16_t  uPosition_Sensor_Reading;

  uHome_Sensor_Reading = analogRead( PIN_SENSOR_HOME );
  uPosition_Sensor_Reading = analogRead( PIN_SENSOR_POSITION );

  Serial.print( "Home: " );
  if ( uHome_Sensor_Reading < SENSOR_HOME_THRESHOLD ) {
    Serial.print( "* " );
  } else {
    Serial.print( "  " );
  }
  Serial.print( uHome_Sensor_Reading );
  
  Serial.print( "\tPos: " );
  if ( uPosition_Sensor_Reading < SENSOR_POSITION_THRESHOLD ) {
    Serial.print( "* " );
  } else {
    Serial.print( "  " );
  }
  Serial.println( uPosition_Sensor_Reading );
  
}
