/*
    Brute force hardcoded, only works if SAMPLING_INTEVAL >= LIGHT_OFF_PULSE_MS
    only syncs up if LIGHT_XX_LED_INTERVAL_MS % SAMPLING_INTEVAL == 0
 */
#include <stdio.h>
#include "header\evtGenerator.h"

#define SAMPLING_INTERVAL_MS        (10000)

#define LIGHT_OFF_PULSE_MS          (100)
#define LIGHT_OFF_LED_INTERVAL_MS   (60000)
#define LIGHT_OFF_SAMPLE_COUNT      (LIGHT_OFF_LED_INTERVAL_MS/SAMPLING_INTERVAL_MS)

#define LIGHT_ON_PULSE_MS           (50)
#define LIGHT_ON_LED_INTERVAL_MS    (10000)
#define LIGHT_ON_SAMPLE_COUNT       (LIGHT_ON_LED_INTERVAL_MS/SAMPLING_INTERVAL_MS)

typedef enum
{
    STATE_IDLE,
    STATE_LIGHT_ON,
    STATE_LIGHT_OFF
} mainState_t;

/* Mock ADC */
#define MOCK_VALUE_COUNT (10)
unsigned int mockSensorValues[MOCK_VALUE_COUNT] = {500, 500, 500, 1000, 1000, 8000, 1000, 5, 500, 500};

unsigned int nextMockADCIndex = 0;
unsigned int mockGetADC()
{
    printf("-- ADC sample\n");
    if (nextMockADCIndex < MOCK_VALUE_COUNT)
    {
        return mockSensorValues[nextMockADCIndex++];
    }
    return 0;
}

/* Mock sleep */
unsigned long timeStamp = 0;
void mockSleep(unsigned long sleepPeriod)
{
    timeStamp += sleepPeriod;
    if (sleepPeriod > 0)
        printf("Timestamp = %d:\n", timeStamp);
}

/* Mock led control */
void ledOn()
{
    printf("-- led ON\n");
}

void ledOff()
{
    printf("-- led OFF\n");
}

int main()
{
    mainState_t currentState = STATE_IDLE;
    LightEvent_t currentEvent;

    resetEventGenerator();
    
    unsigned int thisADCValue = 500;
    seedFirstValue(thisADCValue);
    unsigned int sampleCount = 0;
    while (thisADCValue != 0)
    {
        thisADCValue = mockGetADC();
        currentEvent = getEvent(thisADCValue);
        switch (currentState)
        {
            case STATE_IDLE:
                if (currentEvent == LIGHT_EVT_RISING_EDGE)
                {
                    printf("LIGHT SWITCHED ON\n");
                    ledOn();
                    mockSleep(LIGHT_ON_PULSE_MS);
                    ledOff();
                    sampleCount = 0;
                    currentState = STATE_LIGHT_ON;
                    mockSleep(SAMPLING_INTERVAL_MS - LIGHT_ON_PULSE_MS);
                }
                else if (currentEvent == LIGHT_EVT_FALLING_EDGE)
                {
                    printf("LIGHT SWITCHED OFF\n");
                    ledOn();
                    mockSleep(LIGHT_OFF_PULSE_MS);
                    ledOff();
                    sampleCount = 0;
                    currentState = STATE_LIGHT_OFF;
                    mockSleep(SAMPLING_INTERVAL_MS - LIGHT_OFF_PULSE_MS);
                }
                else
                {
                    mockSleep(SAMPLING_INTERVAL_MS);
                }
                
                break;
            case STATE_LIGHT_ON:
                if (currentEvent == LIGHT_EVT_FALLING_EDGE)
                {
                    printf("LIGHT SWITCHED OFF\n");
                    ledOn();
                    mockSleep(LIGHT_OFF_PULSE_MS);
                    ledOff();
                    currentState = STATE_LIGHT_OFF;
                    sampleCount = 0;
                    mockSleep(SAMPLING_INTERVAL_MS - LIGHT_OFF_PULSE_MS);
                }
                else
                {
                    sampleCount = (sampleCount + 1) % LIGHT_ON_SAMPLE_COUNT;
                    if (sampleCount == 0)
                    {
                        ledOn();
                        mockSleep(LIGHT_ON_PULSE_MS);
                        ledOff();
                        currentState = STATE_LIGHT_OFF;
                        sampleCount = 0;
                        mockSleep(SAMPLING_INTERVAL_MS - LIGHT_ON_PULSE_MS);                       
                    }
                    else
                    {
                        mockSleep(SAMPLING_INTERVAL_MS);
                    }
                }
                break;
            case STATE_LIGHT_OFF:
                if (currentEvent == LIGHT_EVT_RISING_EDGE)
                {
                    printf("LIGHT SWITCHED ON\n");
                    ledOn();
                    mockSleep(LIGHT_ON_PULSE_MS);
                    ledOff();
                    currentState = STATE_LIGHT_OFF;
                    sampleCount = 0;
                    mockSleep(SAMPLING_INTERVAL_MS - LIGHT_ON_PULSE_MS);
                }
                else
                {
                    sampleCount = (sampleCount + 1) % LIGHT_OFF_SAMPLE_COUNT;
                    if (sampleCount == 0)
                    {
                        ledOn();
                        mockSleep(LIGHT_OFF_PULSE_MS);
                        ledOff();
                        currentState = STATE_LIGHT_OFF;
                        sampleCount = 0;
                        mockSleep(SAMPLING_INTERVAL_MS - LIGHT_OFF_PULSE_MS);                       
                    }
                    else
                    {
                        mockSleep(SAMPLING_INTERVAL_MS);
                    }
                }
                break;
        }
    }
}