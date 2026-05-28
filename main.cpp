#include <AccelStepper.h>
#include <TMCStepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <math.h>

const int MPU = 0x68; // MPU6050 I2C address

float AccX, AccY, AccZ;
float pitch, roll;

void setup() {
  Serial.begin(115200);
  delay(2000);

  Wire.begin(21, 22); // SDA, SCL for ESP32

  // Wake up MPU6050
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);      // PWR_MGMT_1 register
  Wire.write(0x00);      // wake up
  Wire.endTransmission(true);

  Serial.println("MPU6050 direct register test started");
}

void loop() {
  // Start reading from ACCEL_XOUT_H
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  // Read 6 bytes: AccX high/low, AccY high/low, AccZ high/low
  Wire.requestFrom(MPU, 6, true);

  int16_t rawAccX = Wire.read() << 8 | Wire.read();
  int16_t rawAccY = Wire.read() << 8 | Wire.read();
  int16_t rawAccZ = Wire.read() << 8 | Wire.read();

  // Convert raw values to g, assuming ±2g range
  AccX = rawAccX / 16384.0;
  AccY = rawAccY / 16384.0;
  AccZ = rawAccZ / 16384.0;

  // Calculate angles from accelerometer
  roll  = atan2(AccY, AccZ) * 180.0 / PI;
  pitch = atan2(-AccX, sqrt(AccY * AccY + AccZ * AccZ)) * 180.0 / PI;

  Serial.print("AccX: ");
  Serial.print(AccX);
  Serial.print(" | AccY: ");
  Serial.print(AccY);
  Serial.print(" | AccZ: ");
  Serial.print(AccZ);

  Serial.print(" || Pitch: ");
  Serial.print(pitch);
  Serial.print(" deg | Roll: ");
  Serial.print(roll);
  Serial.println(" deg");

  delay(100);
}


// #define STEP_PIN 19
// #define DIR_PIN 18
// #define EN_PIN      5
// #define RX_PIN      16
// #define TX_PIN      17

// #define R_SENSE     0.11f

// // At 9V with a typical NEMA17 (2-4mH inductance):
// // Max electrical step rate ≈ V / (L * steps/s) — practical limit ~20-40kHz
// // With 16x microsteps and 200 steps/rev: 3200 steps/rev
// // 40000 steps/s / 3200 = 12.5 rev/s → with 2mm pitch = 25mm/s
// #define MICROSTEPS      8
// #define STEPS_PER_REV   200
// #define MAX_SPEED   6400
// #define ACCELERATION 400

// TMC2208Stepper driver(&Serial2, R_SENSE);
// AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// void setup() {
//   Serial.begin(115200);
  
//   pinMode(EN_PIN, OUTPUT);
//   digitalWrite(EN_PIN, HIGH);   // disable driver while configuring

//   Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);



//   // Driver
//   driver.begin();
//   driver.toff(4);               // lower toff = faster switching, better at high speeds
//   driver.rms_current(900);      // ~900mA is safe for most NEMA17s at 9V continuous
//   driver.microsteps(MICROSTEPS);
//   driver.pwm_autoscale(true);
//   driver.pwm_autograd(true);    // auto-tunes PWM gradient for better high-speed performance
//   driver.en_spreadCycle(true);  // SpreadCycle gives more torque at high speeds vs StealthChop
  

  

//   stepper.setMaxSpeed(MAX_SPEED);
//   stepper.setAcceleration(ACCELERATION);
//   digitalWrite(EN_PIN, LOW);    // enable driver
//   delay(500);
//   stepper.setCurrentPosition(0);
//   stepper.moveTo(10000);        // change to -20000 for other direction
// }

// void loop() {}
