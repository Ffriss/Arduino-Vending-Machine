#include <Servo.h>

Servo servoA;
Servo servoB;
Servo servoC;
Servo servoD;

const byte servoAPin = A0;
const byte servoBPin = A1;
const byte servoCPin = A2;
const byte servoDPin = A3;

const byte irPin = 2;


// ---------------- SERVO DIRECTION ----------------

const int FORWARD_A = 60;
const int FORWARD_B = 60;
const int FORWARD_C = 60;
const int FORWARD_D = 60;


// ---------------- PULSE ----------------

const unsigned long PULSE_ON_MS = 50;
const unsigned long PULSE_OFF_MS = 150;


// ---------------- TIMEOUT ----------------

const unsigned long DISPENSE_TIMEOUT_MS = 15000;


volatile bool irDetected = false;
bool dispensing = false;


// ---------------- IR INTERRUPT ----------------

void irInterrupt() {

  if (dispensing) {
    irDetected = true;
  }
}


// ---------------- DISPENSE ----------------

void dispense(Servo &servo, byte pin, int forwardValue) {

  dispensing = true;
  irDetected = false;

  unsigned long startTime = millis();

  while (!irDetected) {

    if (millis() - startTime > DISPENSE_TIMEOUT_MS) {
      break;
    }


    // หมุน
    servo.attach(pin);
    servo.write(forwardValue);

    unsigned long onStart = millis();

    while (!irDetected &&
           millis() - onStart < PULSE_ON_MS) {

      delay(1);
    }


    // หยุด
    servo.detach();

    unsigned long offStart = millis();

    while (!irDetected &&
           millis() - offStart < PULSE_OFF_MS) {

      delay(1);
    }
  }


  // หยุดสนิท
  servo.detach();

  dispensing = false;


  // ส่งผลกลับ Arduino #1

  if (irDetected) {
    Serial.println("Y");
  }
  else {
    Serial.println("N");
  }


  delay(300);

  irDetected = false;


  // เคลียร์ Serial ที่ค้าง

  while (Serial.available()) {
    Serial.read();
  }
}


// ---------------- SETUP ----------------

void setup() {

  Serial.begin(9600);

  pinMode(irPin, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(irPin),
    irInterrupt,
    FALLING
  );

  delay(500);
}


// ---------------- LOOP ----------------

void loop() {

  if (dispensing) {
    return;
  }


  if (Serial.available()) {

    char command = Serial.read();


    if (command == 'A') {

      dispense(
        servoA,
        servoAPin,
        FORWARD_A
      );
    }


    else if (command == 'B') {

      dispense(
        servoB,
        servoBPin,
        FORWARD_B
      );
    }


    else if (command == 'C') {

      dispense(
        servoC,
        servoCPin,
        FORWARD_C
      );
    }


    else if (command == 'D') {

      dispense(
        servoD,
        servoDPin,
        FORWARD_D
      );
    }
  }
}