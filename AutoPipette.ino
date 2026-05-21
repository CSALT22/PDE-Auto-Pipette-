
// #include "motor_control.h"
// #include "sensors.h"
// #include "buttons.h"
// #include "states.h"
// #include "safety.h"

#include "imu_sensing.cpp"
#include <AccelStepper.h>
#include <TMCStepper.h>
#include <Wire.h>
#include <math.h>

enum PipetteState
{
    WAIT_FOR_ASPIRATE,
    ASPIRATING,
    WAIT_FOR_DISPENSE,
    DISPENSING,
    ERROR_STATE
};

PipetteState currentState = WAIT_FOR_ASPIRATE;
bool liquidEmpty = false;
volatile bool emergencyStopTriggered = false;
void emergencyStopISR()
{
    emergencyStopTriggered = true;
}

void setup()
{
    Serial.begin(115200);

    // initialiseSystem();
    // initialiseMotor();
    // initialiseSensors();
    // initialiseButtons();
    initPipetteAngleSensor();

    // Interrupt service routine for emergency 
    attachInterrupt(
        digitalPinToInterrupt(ESTOP_PIN),
        emergencyStopISR,
        FALLING
    );

    Serial.println("Auto Pipette System Initialised");
    Serial.println("Ready to Aspirate");
}




//---------------------------------------- ANGLE SENSING ----------------------------------------
const int MPU_ADDR = 0x68;

void initPipetteAngleSensor() {
  Wire.begin(21, 22);

  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
}

float readPipetteAngle() {
  int16_t accX, accY, accZ;

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR, 6, true);

  accX = Wire.read() << 8 | Wire.read();
  accY = Wire.read() << 8 | Wire.read();
  accZ = Wire.read() << 8 | Wire.read();

  // Angle from vertical using X/Z plane
  return atan2(abs(accX), abs(accZ)) * 180.0 / PI;
}
//----------------------------------------------------------------------------------------------

void loop()
{
    if (emergencyStopTriggered)
    {
        emergencyStop();
        currentState = ERROR_STATE;
    }

    checkLimits();
    checkPipetteAngle(currentState);

    switch (currentState)
    {
        case WAIT_FOR_ASPIRATE:
            idleState();
            if (aspirateButtonPressed())
            {
                currentState = ASPIRATING;
            }
            break;

        case ASPIRATING:
            aspirateState();
            Serial.println("Aspirate Complete");
            Serial.println("Ready to Dispense");
            currentState = WAIT_FOR_DISPENSE;
            break;

        case WAIT_FOR_DISPENSE:
            idleState();
            if (dispenseButtonPressed())
            {
                currentState = DISPENSING;
            }
            break;
        case DISPENSING:
            dispenseState();
            if (liquidEmpty)
            {
                Serial.println("Liquid Empty");
                Serial.println("Return to Aspirate");
                currentState = WAIT_FOR_ASPIRATE;
            }
            else
            {
                currentState = WAIT_FOR_DISPENSE;
            }
            break;

        case ERROR_STATE:
            stopMotor();
            while (1)
            {
            }
            break;
        default:
            emergencyStop();
            break;
    }
}