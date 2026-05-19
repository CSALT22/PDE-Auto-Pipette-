#include <Wire.h>
#include <math.h>

const int MPU_ADDR = 0x68;

const int GREEN_LED = 25;

const int ASPIRATE_BUTTON = 32;
const int DISPENSE_BUTTON = 33;

bool aspirating = true;
bool dispensing = false;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(GREEN_LED, OUTPUT);

  pinMode(ASPIRATE_BUTTON, INPUT_PULLUP);
  pinMode(DISPENSE_BUTTON, INPUT_PULLUP);

  Wire.begin(21, 22);

  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  Serial.println("Pipette angle detection started");
}

void loop() {
  updatePipetteState();

  float angle = getPipetteAngle();

  if (aspirating) {
    checkAspirationAngle(angle);
  }

  if (dispensing) {
    checkDispensingAngle(angle);
  }

  delay(150);
}

void updatePipetteState() {
  if (digitalRead(ASPIRATE_BUTTON) == LOW) {
    aspirating = true;
    dispensing = false;
  }

  if (digitalRead(DISPENSE_BUTTON) == LOW) {
    aspirating = false;
    dispensing = true;
  }
}

float getPipetteAngle() {
  int16_t accX, accY, accZ;

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR, 6, true);

  accX = Wire.read() << 8 | Wire.read();
  accY = Wire.read() << 8 | Wire.read();
  accZ = Wire.read() << 8 | Wire.read();

  // Angle from vertical using X/Z plane
  float angle = atan2(abs(accX), abs(accZ)) * 180.0 / PI;

  Serial.print("Angle: ");
  Serial.print(angle);

  if (aspirating) Serial.print(" | Mode: ASPIRATING");
  if (dispensing) Serial.print(" | Mode: DISPENSING");

  return angle;
}

void checkAspirationAngle(float angle) {
  // Target = 90° ± 5°
  if (angle >= 85 && angle <= 95) {
    digitalWrite(GREEN_LED, HIGH);
    Serial.println(" | Correct aspiration angle");
  } else {
    digitalWrite(GREEN_LED, LOW);
    Serial.println(" | Wrong aspiration angle");
  }
}

void checkDispensingAngle(float angle) {
  // Target = 45° ± 15°
  if (angle >= 30 && angle <= 60) {
    digitalWrite(GREEN_LED, HIGH);
    Serial.println(" | Correct dispensing angle");
  } else {
    digitalWrite(GREEN_LED, LOW);
    Serial.println(" | Wrong dispensing angle");
  }
}
