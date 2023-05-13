/*
    TC4-WB for Artisan Coffee Roaster Application

    Released under BSD-NC-3.0 License

    Created by Sakunamary on 2022

    TC4_Indicator.cpp

    this is a FreeRTOS task for showing infomation on SSD1306 OLED monitor
    the Indicator task will reflash data every 750 ms
    using Adafruit_GFX and OLED driver

*/

#ifndef __TC4_INDICATOR_H__
#define __TC4_INDICATOR_H__

#include <Arduino.h>
#include "TC4.h"
#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include "img.h"
//#include "Fonts/FreeMonoBold9pt7b.h"
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)

#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SSD1306Wire display(SCREEN_ADDRESS, SDA, SCL);   // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h e.g. https://github.com/esp8266/Arduino/blob/master/variants/nodemcu/pins_arduino.h

//SemaphoreHandle_t xIndicatorDataMutex = NULL;

extern float BT_CurTemp;
extern float ET_CurTemp;
extern float AP_CurVal;
extern String local_IP;
extern String BT_EVENT;
extern bool WIFI_STATUS  = false ;

static char buffer[32];

//#define TASKINDICATOR_INDICATOR_INTERVEL 750 // Task re-entry intervel (ms)

void TaskIndicator(void *pvParameters)
{
     
    /* Variable Definition */
    (void)pvParameters;
    TickType_t xLastWakeTime;
    const TickType_t xIntervel = (user_wifi.sampling_time * 1000) / portTICK_PERIOD_MS;
    Serial.println("OLED started");


    String ver = VERSION;

   display.init();
   display.flipScreenVertically();
   display.setContrast(255);

    // Show initial display buffer contents on the screen --
  
   display.clear();
   display.setFont(ArialMT_Plain_10);
   display.drawString(86, 0 + 2,ver);
   display.drawXbm(17, 19,  94, 43,logo_bmp);
   display.display();

    vTaskDelay(3000 / portTICK_RATE_MS); // dealy 3s showup

    // Initial the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    for (;;) // A Task shall never return or exit.
    {
        // Wait for the next cycle
        vTaskDelayUntil(&xLastWakeTime, xIntervel);
        display.clear();

            //显示logo

            display.drawXbm(0, 0, 16, 16, BEAN_LOGO);
            display.drawXbm(0, 16, 16, 16, DRUMMER_LOGO);

#if defined(HAS_AP_INPUT)           
            display.drawXbm(0, 32, 16, 16, DRUMMER_LOGO);

            //显示IP地址和蓝牙状态
            display.drawXbm(0, 48, 16, 16, WIFI_LOGO);
            
            display.drawString(2 + 16, 54,"IP:");
            display.drawString(2 + 30, 54,local_IP);

#else 
            display.drawXbm(0, 32, 16, 16, WIFI_LOGO);
            
            display.drawString(2 + 16, 54-16,"IP:");
            display.drawString(2 + 30, 54-16,local_IP);
#endif             


            //显示温度
            if (xSemaphoreTake(xThermoDataMutex, xIntervel) == pdPASS) // Mutex to make the data more clean
            {
                display.drawStringf(2 + 16, 0 + 2,buffer,"BT:%4.2f",BT_CurTemp);
                display.drawStringf(2 + 16, 18 + 2,buffer,"ET:%4.2f",ET_CurTemp);
#if defined(HAS_AP_INPUT)           
                display.drawStringf(2 + 16, 38,buffer,"AP:%4.2f",AP_CurVal);
#endif
                xSemaphoreGive(xThermoDataMutex);
            }

            display.display();
            vTaskDelay(user_wifi.sampling_time / portTICK_RATE_MS); // dealy 1s showup
        }
    }


#endif