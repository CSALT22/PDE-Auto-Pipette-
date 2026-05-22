#include <AccelStepper.h>
#include <TMCStepper.h>

#define STEP_PIN    4
#define DIR_PIN     2
#define EN_PIN      5
#define RX_PIN      16
#define TX_PIN      17

#define R_SENSE     0.11f

// At 9V with a typical NEMA17 (2-4mH inductance):
// Max electrical step rate ≈ V / (L * steps/s) — practical limit ~20-40kHz
// With 16x microsteps and 200 steps/rev: 3200 steps/rev
// 40000 steps/s / 3200 = 12.5 rev/s → with 2mm pitch = 25mm/s
#define MICROSTEPS      8
#define STEPS_PER_REV   200
#define MAX_SPEED   6400
#define ACCELERATION 400

TMC2208Stepper driver(&Serial2, R_SENSE);
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

void setup() {
  Serial.begin(115200);
  
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH);   // disable driver while configuring

  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);



  // Driver
  driver.begin();
  driver.toff(4);               // lower toff = faster switching, better at high speeds
  driver.rms_current(600);      // ~900mA is safe for most NEMA17s at 9V continuous
  driver.microsteps(MICROSTEPS);
  driver.pwm_autoscale(true);
  driver.pwm_autograd(true);    // auto-tunes PWM gradient for better high-speed performance
  driver.en_spreadCycle(true);  // SpreadCycle gives more torque at high speeds vs StealthChop
  

  

  stepper.setMaxSpeed(MAX_SPEED);
  stepper.setAcceleration(ACCELERATION);
  stepper.move(-2500);      // changing this value to calibrate

  digitalWrite(EN_PIN, LOW);    // enable driver after config
  delay(500);
  Serial.print("TMC2208 version: 0x");
  Serial.println(driver.version(), HEX);
}

void loop() {

  stepper.run();
}
