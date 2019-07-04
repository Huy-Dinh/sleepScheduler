/*
    Brute force hardcoded, only works if SAMPLING_INTEVAL >= LIGHT_OFF_PULSE_MS
    only syncs up if LIGHT_XX_LED_INTERVAL_MS % SAMPLING_INTEVAL == 0
 */
#include <stdio.h>
#include "header\evtGenerator.h"

#define SAMPLING_INTERVAL_MS        (100)

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
#define MOCK_VALUE_COUNT (600)
unsigned int mockSensorValues[MOCK_VALUE_COUNT] = 
{
    61,63,61,56,54,47,43,41,39,48,41,48,51,46,61,50,37,39,42,32,38,35,44,36,34,44,55,66,54,58,45,44,33,25,30,33,32,24,21,34,40,45,52,55,54,60,46,40,42,44,41,46,48,55,62,60,62,63,66,51,56,50,55,54,46,35,49,49,46,43,55,57,59,52,55,55,58,44,33,37,28,43,50,55,57,66,68,71,71,70,68,68,63,62,59,44,34,41,46,41,48,55,50,55,57,59,54,50,51,47,51,65,70,59,44,33,30,24,20,18,24,19,14,18,16,18,19,14,18,16,13,20,15,11,15,17,13,17,12,9,9,13,13,13,18,13,19,14,11,13,10,10,7,6,11,8,9,7,13,16,17,13,18,20,15,11,11,14,11,16,17,15,11,8,15,16,19,20,22,16,12,11,14,13,16,19,19,20,17,19,21,24,18,16,13,10,7,7,5,7,14,10,9,9,13,11,14,17,15,11,16,19,14,11,10,13,14,16,16,12,10,17,20,15,12,10,10,9,11,14,16,13,22,23,20,23,17,13,15,14,18,14,16,19,14,16,13,12,10,8,12,9,9,9,14,13,14,13,14,14,17,13,12,9,16,18,23,22,19,23,19,20,15,17,13,16,17,20,15,19,17,21,18,14,21,22,23,17,17,14,15,11,18,18,17,13,10,7,13,15,20,17,13,18,18,20,17,15,16,12,15,12,17,15,20,15,11,21,19,24,24,21,16,18,16,22,17,18,17,22,22,22,17,13,13,10,8,12,18,30,39,32,43,55,63,64,67,50,50,55,49,56,51,54,56,62,63,51,55,55,47,49,41,49,53,60,67,70,57,58,53,51,41,33,32,28,21,21,21,20,15,12,13,9,14,16,12,12,16,16,17,21,18,14,12,16,18,20,22,20,18,13,13,9,7,12,18,17,30,35,46,43,51,59,60,57,48,49,48,38,48,52,58,62,60,54,50,52,63,78,79,84,79,82,63,60,65,48,37,28,28,28,23,18,16,14,13,18,19,19,22,26,20,20,15,17,22,28,34,27,27,27,24,26,26,20,20,17,19,21,21,16,19,17,12,9,14,20,17,16,17,13,13,10,9,10,8,8,6,14,23,32,24,31,34,26,35,31,43,53,60,52,57,60,58,53,49,41,48,65,53,61,61,55,48,54,41,32,32,32,26,26,20,27,27,27,22,19,21,20,17,21,16,18,20,21,21,25,19,20,21,21,20,25,21,29,26,20,21,18,21,16,16,19,23,23,19,19,17,16,14,11,13,10,16,15,21,24,18,15,11,18,20,17,15,11,15,14,13,10,10,9,15,11,14,16,14,14,15,15,12,9,16,12,16,12,11,8,8,6
};

unsigned int nextMockADCIndex = 0;
unsigned int mockGetADC()
{
    //printf("-- ADC sample\n");
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

int main_test()
{
    unsigned int thisADCValue;
    LightEvent_t currentEvent;
    seedFirstValue(60);
    while (thisADCValue != 0)
    {
        thisADCValue = mockGetADC();
        currentEvent = getEvent(thisADCValue);
        if (currentEvent == LIGHT_EVT_RISING_EDGE)
        {
            printf("RISING EDGE");
            printf("\n");
        }
        else if (currentEvent == LIGHT_EVT_FALLING_EDGE)
        {
            printf("FALLING EDGE");
            printf("\n");
        }
    }
    return 0;
}

int main()
{
    mainState_t currentState = STATE_IDLE;
    LightEvent_t currentEvent;

    resetEventGenerator();
    
    unsigned int thisADCValue = 61;
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
                    printf("%u LIGHT SWITCHED ON\n", nextMockADCIndex);
                    ledOn();
                    mockSleep(LIGHT_ON_PULSE_MS);
                    ledOff();
                    sampleCount = 0;
                    currentState = STATE_LIGHT_ON;
                    mockSleep(SAMPLING_INTERVAL_MS - LIGHT_ON_PULSE_MS);
                }
                else if (currentEvent == LIGHT_EVT_FALLING_EDGE)
                {
                    printf("%u LIGHT SWITCHED OFF\n", nextMockADCIndex);
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
                    printf("%d LIGHT SWITCHED OFF\n", nextMockADCIndex);
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
                        currentState = STATE_LIGHT_ON;
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
                    printf("%d LIGHT SWITCHED ON\n", nextMockADCIndex);
                    ledOn();
                    mockSleep(LIGHT_ON_PULSE_MS);
                    ledOff();
                    currentState = STATE_LIGHT_ON;
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