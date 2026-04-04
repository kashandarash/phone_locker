#include <Servo.h>
#include <EEPROM.h>

Servo myServo;

const int btnOne = 2;
const int btnTwo = 3;
const int btnThree = 4;
const int btnRun = 5;

// 30 min.
const unsigned long timeOne = 18000;
// 45 min.
const unsigned long timeTwo = 27000;
// 60 min.
const unsigned long timeThree = 36000;

// Prevents a button press from registering multiple times in consecutive loop cycles.
const unsigned long debounceDelay = 1000;

// Duration reset button must be held to trigger an unlock.
const unsigned long resetHoldDuration = 5000;

// EEPROM address where selected time is stored to survive power loss.
const int eepromAddr = 0;

const int servoLocked = 90;
const int servoUnlocked = 0;

const int servoPin = 6;
const int buzzer = 7;
const int greenLed = 8;
const int redLed = 9;
const int blueLed = 10;
const int reset = 11;

int currentAngle = 0;

unsigned long selectedTime = 0;
unsigned long startTime = 0;

bool locked = false;
unsigned long lockUntil = 0;
unsigned long resetPressedAt = 0;
bool resetHolding = false;

// Update LEDs based on servo position.
void updateLEDs() {
  if (currentAngle == servoLocked) {
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);
  } else {
    digitalWrite(redLed, LOW);
    digitalWrite(greenLed, HIGH);
  }
}

// Smooth servo movement.
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

// Initialize pins, attach servo and move it to unlocked position.
void setup()
{
  pinMode(btnOne, INPUT);
  pinMode(btnTwo, INPUT);
  pinMode(btnThree, INPUT);
  pinMode(btnRun, INPUT);
  pinMode(reset, INPUT);

  pinMode(buzzer, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(blueLed, OUTPUT);

  myServo.attach(servoPin);

  // Read saved time from EEPROM to restore locked state after power loss.
  EEPROM.get(eepromAddr, selectedTime);
  if (selectedTime > 0 && selectedTime != 0xFFFFFFFF)
  {
    moveServoSmooth(servoLocked);
    startTime = millis();
    locked = true;
  }
  else
  {
    moveServoSmooth(servoUnlocked);
  }
}

// Handle time selection, locking on button press, and unlocking when timer expires.
void loop()
{
  // Select time only when not locked.
  if (!locked)
  {
    if (digitalRead(btnOne) == HIGH && millis() > lockUntil)
    {
      selectedTime = timeOne;
      lockUntil = millis() + debounceDelay;
    }
    if (digitalRead(btnTwo) == HIGH && millis() > lockUntil)
    {
      selectedTime = timeTwo;
      lockUntil = millis() + debounceDelay;
    }
    if (digitalRead(btnThree) == HIGH && millis() > lockUntil)
    {
      selectedTime = timeThree;
      lockUntil = millis() + debounceDelay;
    }
  }

  // Start servo to lock.
  if (digitalRead(btnRun) == HIGH && millis() > lockUntil && !locked && selectedTime > 0)
  {
    moveServoSmooth(servoLocked);
    // Save selected time to EEPROM so lock survives power loss.
    EEPROM.put(eepromAddr, selectedTime);
    startTime = millis();
    locked = true;
    lockUntil = millis() + debounceDelay;
  }

  // Reset button pressed — start tracking hold duration and turn on blue LED.
  if (digitalRead(reset) == HIGH && !resetHolding && locked)
  {
    resetHolding = true;
    resetPressedAt = millis();
    digitalWrite(blueLed, HIGH);
  }

  // Reset button released before 5 seconds — cancel, turn off blue LED.
  if (digitalRead(reset) == LOW && resetHolding)
  {
    resetHolding = false;
    digitalWrite(blueLed, LOW);
  }

  // Reset button held for 5 seconds — force unlock.
  if (resetHolding && millis() - resetPressedAt >= resetHoldDuration)
  {
    resetHolding = false;
    digitalWrite(blueLed, LOW);
    moveServoSmooth(servoUnlocked);
    locked = false;
    selectedTime = 0;
    // Clear EEPROM so device starts unlocked after next power on.
    EEPROM.put(eepromAddr, selectedTime);
  }

  // Timer finished.
  if (locked && millis() - startTime >= selectedTime)
  {
    moveServoSmooth(servoUnlocked);
    // 500 Hz.
    tone(buzzer, 800, 500);
    locked = false;
    selectedTime = 0;
    // Clear EEPROM so device starts unlocked after next power on.
    EEPROM.put(eepromAddr, selectedTime);
  }
}