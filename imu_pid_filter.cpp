#include <Wire.h>
#include <math.h>

// MPU6050
const int MPU_ADDR = 0x68;

// PINS
const int GREEN_LED = 25;
const int ASPIRATE_BUTTON = 32;
const int DISPENSE_BUTTON = 33;

// STATES
bool aspirating = true;
bool dispensing = false;

// COMPLEMENTARY FILTER
float filteredAngle = 0.0;
unsigned long lastTime = 0;

// Filter coefficient
// Higher = smoother but slower
const float alpha = 0.98;

// SETUP
void setup() {

  Serial.begin(115200);
  delay(1000);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(ASPIRATE_BUTTON, INPUT_PULLUP);
  pinMode(DISPENSE_BUTTON, INPUT_PULLUP);

  // ESP32 I2C pins
  Wire.begin(21, 22);

  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  lastTime = millis();

  Serial.println("Semi-automatic pipette IMU system started");
}

// MAIN LOOP
void loop() {

  updatePipetteState();
  float angle = getPipetteAngle();
  if (aspirating) {
    checkAspirationAngle(angle);
  }

  if (dispensing) {
    checkDispensingAngle(angle);
  }

  delay(20);
}

// UPDATE STATES
void updatePipetteState() {

  if (digitalRead(ASPIRATE_BUTTON) == LOW) {

    aspirating = true;
    dispensing = false;
    Serial.println("Mode changed -> ASPIRATING");
    delay(250);
  }

  if (digitalRead(DISPENSE_BUTTON) == LOW) {
    aspirating = false;
    dispensing = true;
    Serial.println("Mode changed -> DISPENSING");
    delay(250);
  }
}

// GET FILTERED ANGLE
float getPipetteAngle() {
  
  int16_t accX, accY, accZ;
  int16_t gyroY;
  
  // READ ACCELEROMETER
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR, 6, true);

  accX = Wire.read() << 8 | Wire.read();
  accY = Wire.read() << 8 | Wire.read();
  accZ = Wire.read() << 8 | Wire.read();


  // READ GYROSCOPE
  // Gyro Y register = 0x45
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x45);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR, 2, true);

  gyroY = Wire.read() << 8 | Wire.read();

  // TIMING
  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;

  // Prevent invalid dt
  if (dt <= 0 || dt > 1) {
    dt = 0.01;
  }

  // Horizontal = 0°
  // 45° tilt = 45°
  // Vertical = 90°
  
  float accelAngle =
      atan2((float)abs(accZ), (float)abs(accX))
      * 180.0 / PI;

  // MPU6050 sensitivity:
  // 131 LSB = 1 deg/s
  float gyroRate = gyroY / 131.0;


  // COMPLEMENTARY FILTER
  filteredAngle =
      alpha * (filteredAngle + gyroRate * dt)
      + (1 - alpha) * accelAngle;

  // SERIAL MONITOR
  Serial.print("Filtered Angle: ");
  Serial.print(filteredAngle);

  if (aspirating) {
    Serial.print(" | ASPIRATING");
  }

  if (dispensing) {
    Serial.print(" | DISPENSING");
  }

  return filteredAngle;
}


// ASPIRATION CHECK : Target = 90° ± 5°
void checkAspirationAngle(float angle) {

  if (angle >= 85 && angle <= 95) {

    digitalWrite(GREEN_LED, HIGH);

    Serial.println(" | Correct aspiration angle");

  } else {

    digitalWrite(GREEN_LED, LOW);

    Serial.println(" | Wrong aspiration angle");
  }
}

// DISPENSING CHECK : Target = 45° ± 15°
void checkDispensingAngle(float angle) {

  if (angle >= 30 && angle <= 60) {

    digitalWrite(GREEN_LED, HIGH);

    Serial.println(" | Correct dispensing angle");

  } else {

    digitalWrite(GREEN_LED, LOW);

    Serial.println(" | Wrong dispensing angle");
  }
} 
