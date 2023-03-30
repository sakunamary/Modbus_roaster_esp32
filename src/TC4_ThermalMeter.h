/*  TC4-WB for Artisan Coffee Roaster Application

    Released under BSD-NC-3.0 License

    Created by Sakunamary on 2022


    ThermalMeter Task
    The default sample rate of Artisan is 3 seconds, although the setting value can be modified by user.
    I think this value is generated from lots of experimental, so keeps polling-time as 3 seconds.
    Thus, In the ThermalMeter Task will (750ms x 4 = 3s)
    (1) read MAX6675 temperature evey 750ms, and
    (2) Average-Array length is 4.
    That is, the measured temperature is avaraged from 4 times of MAX6675 temperature reading.

*/
#ifndef __TC4_THERMALMETER_H__
#define __TC4_THERMALMETER_H__

#include <Arduino.h>
#include "TC4.h"

SemaphoreHandle_t xThermoDataMutex = NULL;

void TaskThermalMeter(void *pvParameters)
{

    /* Variable Definition */
    (void)pvParameters;
    TickType_t xLastWakeTime;

    const TickType_t xIntervel = (user_wifi.sampling_time * 1000) / portTICK_PERIOD_MS;
    Serial.println("Thermo Task started");
    /* Task Setup and Initialize */
    // Initial the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    for (;;) // A Task shall never return or exit.
    {

        vTaskDelayUntil(&xLastWakeTime, xIntervel);

        if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE){
            

            if (xSemaphoreTake(xThermoDataMutex, xIntervel) == pdPASS)
                {


            xSemaphoreGive(xThermoDataMutex);
            }

        }

    




#endif
