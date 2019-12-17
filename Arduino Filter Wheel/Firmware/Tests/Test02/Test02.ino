
#define LED_PIN   13
#define ENABLE    4
#define STEP      2
#define DIR       7

#define MOTOR_SPEED 1

void setup() {
  // put your setup code here, to run once:

  pinMode( LED_PIN, OUTPUT );

  pinMode( ENABLE, OUTPUT );
  pinMode( STEP, OUTPUT );
  pinMode( DIR, OUTPUT );

  digitalWrite( ENABLE, LOW );
  digitalWrite( DIR, HIGH );
  
}

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite( STEP, HIGH );
  digitalWrite( LED_PIN, HIGH );
  delay( MOTOR_SPEED );

  digitalWrite( STEP, LOW );
  digitalWrite( LED_PIN, LOW );
  delay( MOTOR_SPEED );
  
}
