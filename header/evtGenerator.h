/* ==========================================================
   Event generator, filters the low and high frequency noises
   the poor man's way 
   =========================================================*/

#ifndef EVENT_GEN_H
#define EVENT_GEN_H

#define LIGHT_STABLE_THRESHOLD          (200)
#define LIGHT_STABLE_COUNT_THRESHOLD    (2)

typedef enum
{
    LIGHT_EVT_RISING_EDGE,
    LIGHT_EVT_FALLING_EDGE,
    LIGHT_EVT_NO_CHANGE
} LightEvent_t;

void seedFirstValue(unsigned int firstSensorReading);
void resetEventGenerator();
LightEvent_t getEvent(unsigned int sensorReading);
#endif