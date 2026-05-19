#include "motor_control.h"
#include "sensors.h"
#include "buttons.h"
#include "states.h"
#include "safety.h"
#include "imu.h"

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
    Serial.begin(9600);

    initialiseSystem();
    initialiseMotor();
    initialiseSensors();
    initialiseButtons();
    initialiseIMU();

    attachInterrupt(
        digitalPinToInterrupt(ESTOP_PIN),
        emergencyStopISR,
        FALLING
    );

    Serial.println("Auto Pipette System Initialised");
    Serial.println("Ready to Aspirate");
}

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