#include <Servo.h>

Servo myServo;

const int btn15 = 2;   // 15 min
const int btn30 = 3;   // 30 min
const int btn60 = 4;   // 60 min
const int btnRun = 5;

const int servoPin = 6;
const int buzzerPin = 7;
const int greenLed = 8;
const int redLed = 9;

int currentAngle = 0;

unsigned long selectedTime = 0;
unsigned long startTime = 0;

bool running = false;
unsigned long lockUntil = 0;

// update LEDs based on servo position
void updateLEDs() {
  if (currentAngle == 90) {
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);
  } else {
    digitalWrite(redLed, LOW);
    digitalWrite(greenLed, HIGH);
  }
}

// smooth servo movement
void moveServoSmooth(int targetAngle)
{
  if (targetAngle > currentAngle)
  {
    for (int pos = currentAngle; pos <= targetAngle; pos++)
    {
      myServo.write(pos);
      currentAngle = pos;
      updateLEDs();
      delay(10);
    }
  }
  else
  {
    for (int pos = currentAngle; pos >= targetAngle; pos--)
    {
      myServo.write(pos);
      currentAngle = pos;
      updateLEDs();
      delay(10);
    }
  }
}

void setup()
{
  pinMode(btn15, INPUT);
  pinMode(btn30, INPUT);
  pinMode(btn60, INPUT);
  pinMode(btnRun, INPUT);

  pinMode(buzzerPin, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);

  myServo.attach(servoPin);

  moveServoSmooth(0);  // start at 0°
}

void loop()
{
  // select time
  if (digitalRead(btn15) == HIGH) selectedTime = 900;   // 15 min
  if (digitalRead(btn30) == HIGH) selectedTime = 1800;  // 30 min
  if (digitalRead(btn60) == HIGH) selectedTime = 3600;  // 60 min

  // start servo
  if (digitalRead(btnRun) == HIGH && millis() > lockUntil && !running && selectedTime > 0)
  {
    moveServoSmooth(90);
    startTime = millis();
    running = true;
    lockUntil = millis() + 2000;
  }

  // timer finished
  if (running && millis() - startTime >= selectedTime)
  {
    moveServoSmooth(0);

    tone(buzzerPin, 500, 500);  // 2 kHz
    //delay(2000);
    //noTone(buzzerPin);

    running = false;
    selectedTime = 0;
  }
}