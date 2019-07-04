#include "evtGenerator.h"

#define SECONDARY_FILTER_WEIGHT (0.25)
#define BASE_UPDATE_WEIGHT (0.05)

typedef enum
{
    EVT_GEN_STABLE,
    EVT_GEN_CHANGE_DETECTED
} EventGeneratorState_t;

/* Utility functions */
unsigned int absoluteValue(unsigned int firstNumber, unsigned int secondNumber)
{
    if (firstNumber > secondNumber)
    {
        return firstNumber - secondNumber;
    }
    else
    {
        return secondNumber - firstNumber;
    }
}

EventGeneratorState_t eventGenState = EVT_GEN_STABLE;
//Basically counts if the new value stays within the threshold for LIGHT_STABLE_COUNT_THRESHOLD times
unsigned char newValueStability = 0; 
float lastStableSensorValue = 0;
float lastNewSensorValue = 0;
float lastSensorReading = 0;

void seedFirstValue(unsigned int firstSensorReading)
{
    lastStableSensorValue = firstSensorReading;
    lastSensorReading = firstSensorReading;
}

void resetEventGenerator()
{
    eventGenState = EVT_GEN_STABLE;
    newValueStability = 0;
    lastStableSensorValue = 0;
    lastNewSensorValue = 0;
}

LightEvent_t getEvent(unsigned int sensorReading)
{
    LightEvent_t returnEvent = LIGHT_EVT_NO_CHANGE;
    unsigned int filteredSensorReading = lastSensorReading * (1 - SECONDARY_FILTER_WEIGHT) + sensorReading * SECONDARY_FILTER_WEIGHT;
    switch (eventGenState)
    {
        case EVT_GEN_STABLE:
            // New sensor reading is within threshold --> slow change due to sun light
            if (absoluteValue(filteredSensorReading, lastStableSensorValue) < LIGHT_STABLE_THRESHOLD)
            {
                lastStableSensorValue = lastStableSensorValue * (1 - BASE_UPDATE_WEIGHT) + filteredSensorReading * BASE_UPDATE_WEIGHT;
            }
            // A considerable change is detected
            else
            {
                eventGenState = EVT_GEN_CHANGE_DETECTED;
                // update the new "unstable value"
                lastNewSensorValue = filteredSensorReading;
                // give the change some creditability
                ++newValueStability;
            }
            returnEvent = LIGHT_EVT_NO_CHANGE;
            break;

        case EVT_GEN_CHANGE_DETECTED:
            // If the new sensor reading does not go too far from the last new value
            if ((absoluteValue(filteredSensorReading, lastNewSensorValue) < LIGHT_STABLE_THRESHOLD))
            {
                lastNewSensorValue = lastNewSensorValue * (1 - BASE_UPDATE_WEIGHT) + filteredSensorReading * BASE_UPDATE_WEIGHT;;
                // If the new value has been constant for enough samples
                if (++newValueStability >= LIGHT_STABLE_COUNT_THRESHOLD)
                {
                    // It's considered a stable signal and an edge event is triggered
                    eventGenState = EVT_GEN_STABLE;
                    if (lastNewSensorValue > lastStableSensorValue)
                        returnEvent = LIGHT_EVT_RISING_EDGE;
                    else
                        returnEvent = LIGHT_EVT_FALLING_EDGE;
                    lastStableSensorValue = lastNewSensorValue;
                }
            }
            // If the value is close to the last stable value again
            else if ((absoluteValue(filteredSensorReading, lastStableSensorValue)))
            {
                eventGenState = EVT_GEN_STABLE;
                newValueStability = 0;
                returnEvent = LIGHT_EVT_NO_CHANGE;
            }
            // This means there is another "new" value
            else
            {
                lastNewSensorValue = filteredSensorReading;
                // Sets the stability counter to 1
                // because exactly 1 value of this kind is detected
                newValueStability = 1;
                returnEvent = LIGHT_EVT_NO_CHANGE;
            }
            break;
    }
    lastSensorReading = sensorReading;
    return returnEvent;
}
