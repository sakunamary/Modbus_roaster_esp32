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

#include <ESP32CAN.h>
#include <CAN_config.h>
float BT_CurTemp;
float ET_CurTemp;
float AP_CurVal;


CAN_frame_t rx_frame;
CAN_device_t CAN_cfg;

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

            if (rx_frame.MsgID == 0x0F6 && rx_frame.FIR.B.DLC == 8){
                if (xSemaphoreTake(xThermoDataMutex, xIntervel) == pdPASS)
                {
                    BT_CurTemp=(rx_frame.data.u32[0]+  user_wifi.btemp_fix)*100 ;
                    ET_CurTemp=(rx_frame.data.u32[1]+  user_wifi.etemp_fix)*100;
                    xSemaphoreGive(xThermoDataMutex);
            
                 } 
            }
#if defined(HAS_AP_INPUT)
            if (rx_frame.MsgID == 0xE6 && rx_frame.FIR.B.DLC == 8){ //airpress 
                if (xSemaphoreTake(xThermoDataMutex, xIntervel) == pdPASS)
                {
                    AP_CurVal=(rx_frame.data.u32[0]+  user_wifi.ap_fix)*100 ;
                    xSemaphoreGive(xThermoDataMutex);
            
                 } 
            }
#endif            
        }
    }
}    




#endif
