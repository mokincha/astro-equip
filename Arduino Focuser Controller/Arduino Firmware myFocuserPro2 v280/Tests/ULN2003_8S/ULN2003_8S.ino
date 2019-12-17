// myfocuser example
// ONLY FOR USE WITH 28BYJ-48 AND ULN2003 DRIVER

#include <Arduino.h>
#include <myStepper.h>              // needed for stepper motor, installed when Arduino IDE is installed
#define SerialPortSpeed 9600

// stepper connected on pins 4 (IN1), 5 (IN2), 6 (IN3), 7 (IN4)
#define IN1  4
#define IN2  5
#define IN3  6
#define IN4  7
int motorPins[] = { IN1, IN2, IN3, IN4 };  // used to disable the output so it does not overheat
int Steps;  // 8 step motor 0-7
#define CLOCKWISE 1
#define ANTICLOCKWISE 0
int Direction;

void stepper(int xw)
{
  for (int x = 0; x < xw; x++)
  {
    switch (Steps)
    {
      case 0:
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
        break;
      case 1:
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, HIGH);
        break;
      case 2:
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
        break;
      case 3:
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
        break;
      case 4:
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        break;
      case 5:
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        break;
      case 6:
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        break;
      case 7:
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
        break;
      default:
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        break;
    }
    SetDirection();
  }
}

void SetDirection()
{
  if (Direction == CLOCKWISE)
  {
    Steps++;
  }
  if (Direction == ANTICLOCKWISE)
  {
    Steps--;
  }
  if (Steps > 7)
  {
    Steps = 0;
  }
  if (Steps < 0)
  {
    Steps = 7;
  }
}

// Move stepper anticlockwise
void anticlockwise()
{
  stepper(-1);                 // step the motor one step anticlockwise
}

// Move stepper clockwise
void clockwise()
{
  stepper(1);         // step the motor one step clockwise
}

// Setup
void setup()
{
  Serial.begin(SerialPortSpeed);  // initialize serial port
  Steps = 0;
  Serial.println("Starting test");
}

// Main Loop
void loop()
{
  int lp;
  Direction = ANTICLOCKWISE;
  Serial.println("Stepping motor 200 steps anticlockwise");
  for ( lp = 0; lp < 200; lp++ )
    anticlockwise();
  Serial.println("Motor stopped");

  delay(2000);

  Direction = CLOCKWISE;
  Serial.println("Stepping motor 200 steps clockwise");
  for ( lp = 0; lp < 200; lp++ )
    clockwise();
  Serial.println("Motor stopped");

  delay(2000);
}
