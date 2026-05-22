#include "imu_sensing.cpp"
#include <AccelStepper.h>
#include <TMCStepper.h>


enum PipetteState
{
    HOME,
    SETTINGS,
    PRE_ASPIRATING,
    ASPIRATING,
    PRE_DISPENSING,
    DISPENSING,
    ERROR_STATE
};

// Initial state 
PipetteState currentState = HOME;
bool liquidEmpty = false;

// Emergency stop pin (change to your wired pin)
const int ESTOP_PIN = 27;

// Initialising emergency stop
volatile bool emergencyStopTriggered = false;
void emergencyStopISR()
{
    emergencyStopTriggered = true;
}

void initEmergencyStopISR() {
    pinMode(ESTOP_PIN, INPUT_PULLUP);
    attachInterrupt(
        digitalPinToInterrupt(ESTOP_PIN),
        emergencyStopISR,
        FALLING
    );
}

void setup()
{
    Serial.begin(115200);

    // Initialise components/processes
    initPipetteAngleSensor();
    initEmergencyStopISR();

    Serial.println("Auto Pipette System Initialised");
    Serial.println("Ready to Aspirate");
}

int liquidAspirated = 0;

int liquidDispensed = 0;

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



//---------------------------------------- CHECK LIMITS ----------------------------------------
void checkLimits() {
    if (liquidDispensed <= liquidAspirated) {
        liquidEmpty = true;
    }
}



//---------------------------------------- MAIN LOOP ----------------------------------------


void loop()
{
    if (emergencyStopTriggered)
    {
        emergencyStop();
        currentState = ERROR_STATE;
    }

    checkLimits();
    readPipetteAngle();

    switch (currentState)
    {
        case HOME:
            idleState();
            if (aspirateButtonPressed())
            {
                currentState = PRE_ASPIRATING;
            }
            break;

        case PRE_ASPIRATING:
            Serial.println("Preparing to aspirate");
            currentState = ASPIRATING;
            break;

        case ASPIRATING:
            aspirateState();
            Serial.println("Aspirate Complete");
            Serial.println("Ready to Dispense");
            currentState = PRE_DISPENSING;
            break;

        case PRE_DISPENSING:
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
                currentState = HOME;
            }
            else
            {
                currentState = PRE_DISPENSING;
            }
            break;

        case SETTINGS:
            // Optional settings mode
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
//----------------------------------------------------------------------------------------------
