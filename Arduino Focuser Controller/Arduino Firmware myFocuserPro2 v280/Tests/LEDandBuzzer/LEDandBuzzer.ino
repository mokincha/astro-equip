#include <Arduino.h>

#define SerialPortSpeed 9600

// watch the output of this program on the serial port monitor of the arduino IDE

#define bledIN A1                 // define IN and OUT LEDS, associated with PB and stepper moves
#define gledOUT A2
#define Buzzer A3                 // define Buzzer

// Setup
void setup()
{
  Serial.begin(SerialPortSpeed);      // initialize serial port

  pinMode(Buzzer, OUTPUT);            // turn ON the Buzzer - provide power ON beep
  digitalWrite( Buzzer, 1);
  pinMode( bledIN, OUTPUT);           // turn ON both LEDS as power on cycle indicator
  pinMode( gledOUT, OUTPUT);
  digitalWrite( bledIN, 1 );
  digitalWrite( gledOUT, 1 );

  Serial.println("Blue LED ON - A1");
  Serial.println("GREEN LED ON - A2");
  Serial.println("Buzzer ON - A3");

  delay(2000);
}

void loop()
{
  // TURN OFF
  digitalWrite( Buzzer, 0);
  Serial.println("Buzzer OFF - A3");
  delay(3000);

  digitalWrite( bledIN, 0 );
  Serial.println("Blue LED OFF - A1");
  delay(3000);
  
  digitalWrite( gledOUT, 0 );
  Serial.println("Green LED OFF - A2");
  delay(3000);

  // TURN ON
  digitalWrite( Buzzer, 1);
  Serial.println("Buzzer ON - A3");
  delay(3000);

  digitalWrite( bledIN, 1 );
  Serial.println("Blue LED ON - A1");
  delay(3000);
  
  digitalWrite( gledOUT, 1 );
  Serial.println("Green LED ON - A2");
  delay(3000);
}

