/*
    TC4-WB for Artisan Coffee Roaster Application

    Released under BSD-NC-3.0 License

    Created by Sakunamary on 2022

    TC4-WB main.cpp

    TC4-WB is a esp32 base thermo module for Artisan .
    1)it can get two channels  temperture data by MX6675 .
    For BT channel, data updates every 750ms and ET channel data updates every 3s
    2)The temperture datas can transmit with Artisan by wifi-websocket and/or bluetooth-TC4,
      base on the version you chioced .
       fullversion : Both wifi-websocket and bluetooth ,so you can monitor the data on PC and cellphone both
       wifiversion : Only wifi-websocket,so you can monitor data on PC on tablet by wifi
       bluetoothversion : Only bluetooth ,you can monitor data by bluetooth
    3) ALL version can use OTA to update firmware by wifi
    4)Thermo compensate funciton also include

*/



#include <Arduino.h>
#include "TC4.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "WebSerial.h"
#include <ModbusIP_ESP8266.h>

// Thermo lib for MX6675
#include "max6675.h"


#include "TC4_Indicator.h"
#include "TC4_ThermalMeter.h"

#include <EEPROM.h>


// Define three tasks
extern void TaskIndicator(void *pvParameters);
extern void TaskThermalMeter(void *pvParameters);
extern void TaskBatCheck(void *pvParameters);

// define other functions
String IpAddressToString(const IPAddress &ipAddress);                         //转换IP地址格式
void notFound(AsyncWebServerRequest *request);                                // webpage function
String processor(const String &var);                                          // webpage function

// define variable
extern float BT_AvgTemp;
extern float ET_CurTemp;

String BT_EVENT;
String local_IP;
uint32_t lastTimestamp = millis();
float last_BT_temp = -273.0;
bool take_temp = true;

TaskHandle_t xHandle_indicator;

//Modbus Registers Offsets
const int BT_HREG = 3001;
const int ET_HREG = 3002;
const int AT_HREG = 3003;
const int PWR_HREG = 3004;
const int ROLL_HREG = 3005;

//ModbusIP object
ModbusIP mb;


/*
     // Create a task, storing the handle.
     xTaskCreate( vTaskCode, "NAME", STACK_SIZE, NULL, tskIDLE_PRIORITY, &xHandle );

     // ...

     // Use the handle to suspend the created task.
     vTaskSuspend( xHandle );

*/
user_wifi_t user_wifi = {" ", " ", 0.0, 0.0, 0.75, 300,true};

// object declare
AsyncWebServer server_OTA(80);


String IpAddressToString(const IPAddress &ipAddress)
{
    return String(ipAddress[0]) + String(".") +
           String(ipAddress[1]) + String(".") +
           String(ipAddress[2]) + String(".") +
           String(ipAddress[3]);
}

String processor(const String &var)
{
    Serial.println(var);
    if (var == "bt_compens")
    {
        return String(user_wifi.btemp_fix);
    }
    else if (var == "et_compens")
    {
        return String(user_wifi.etemp_fix);
    }
    else if (var == "version")
    {
        return VERSION;
    }
    else if (var == "sampling_time")
    { //
        return String(user_wifi.sampling_time);
    }
    else if (var == "sleeping_time")
    {
        return String(user_wifi.sleeping_time/60);
    }
    return String();
}

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Opps....Not found");
}

// low power mode; checks every few seconds for an event
void checkLowPowerMode(float temp_in)
{

// for debug 
//  user_wifi.Init_mode = true ;
//   user_wifi.sampling_time = 0.25; 
//   user_wifi.sleeping_time = 100;
//   EEPROM.put(0, user_wifi);
//    EEPROM.commit();

//    Serial.println("reset function done!");
//

    if (take_temp)
    {
        last_BT_temp = temp_in;   //设置第一次温度戳
        lastTimestamp = millis(); //设置第一次时间戳
        take_temp = false;
        // Serial.printf("last_BT_temp is : %f ",BT_AvgTemp);
    }

    if ((millis() - lastTimestamp) > (user_wifi.sleeping_time * 1000))
    {
        if (abs(last_BT_temp - temp_in) < 5)
        { // 60s
            take_temp = true;
            vTaskSuspend(xHandle_indicator);
            // 满足条件1:时间够60s and 条件2: 温度变化不超过5度
            // display.dim(true); //set OLED DIM
            display.clear(); // disable OLED
            
            display.display();      // disable OLE
            delay(1000);
            // set sleep mode
            esp_deep_sleep_start();
        }
        else
        {
            lastTimestamp = millis(); // update timestamp
        }
    }
}



/* Message callback of WebSerial */
void recvMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d);
}


void setup()
{

    xThermoDataMutex = xSemaphoreCreateMutex();
    xIndicatorDataMutex = xSemaphoreCreateMutex();

    // Initialize serial communication at 115200 bits per second:
    Serial.begin(BAUDRATE);
    while (!Serial)
    {
        ; // wait for serial port ready
    }

    Serial.printf("\nArti-Mod  STARTING...\n");

    // set up eeprom data
    EEPROM.begin(sizeof(user_wifi));
    EEPROM.get(0, user_wifi);


   // user_wifi.Init_mode = true ;

if (user_wifi.Init_mode) 
{
    user_wifi.Init_mode = false ;
    user_wifi.sampling_time = 0.75; 
    user_wifi.sleeping_time = 300;
    user_wifi.btemp_fix = 0;
    user_wifi.etemp_fix = 0;
    

    EEPROM.put(0, user_wifi);
    EEPROM.commit();
}

    /*---------- Task Definition ---------------------*/
    // Setup tasks to run independently.
    xTaskCreatePinnedToCore(
        TaskBatCheck, "bat_check" // 测量电池电源数据，每分钟测量一次
        ,
        1024 // This stack size can be checked & adjusted by reading the Stack Highwater
        ,
        NULL, 1 // Priority, with 1 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,
        NULL,  1 // Running Core decided by FreeRTOS,let core0 run wifi and BT
    );

    xTaskCreatePinnedToCore(
        TaskThermalMeter, "ThermalMeter" // MAX6675 thermal task to read Bean-Temperature (BT)
        ,
        1024 // Stack size
        ,
        NULL, 3 // Priority
        ,
        NULL, 
        1 // Running Core decided by FreeRTOS,let core0 run wifi and BT
    );

    xTaskCreatePinnedToCore(
        TaskIndicator, "IndicatorTask" // 128*64 SSD1306 OLED 显示参数
        ,
        1024*6 // This stack size can be checked & adjusted by reading the Stack Highwater
        ,
        NULL, 2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,
        &xHandle_indicator, 1 // Running Core decided by FreeRTOS , let core0 run wifi and BT
    );

  //初始化网络服务
    WiFi.mode(WIFI_STA);
    WiFi.begin(user_wifi.ssid, user_wifi.password);

    byte tries = 0;
    while (WiFi.status() != WL_CONNECTED)
    {

        delay(1000);

        if (tries++ > 5)
        {

            // Serial_debug.println("WiFi.mode(AP):");
            WiFi.mode(WIFI_AP);
            WiFi.softAP("ARTIMOD_THRMO", "12345678"); // defualt IP address :192.168.4.1 password min 8 digis
            break;
        }
        // show AP's IP
    }


    Serial.print("Arti-Mod's IP:");

    if (WiFi.getMode() == 2) // 1:STA mode 2:AP mode
    {
        Serial.println(IpAddressToString(WiFi.softAPIP()));
        local_IP = IpAddressToString(WiFi.softAPIP());
    }
    else
    {
        Serial.println(IpAddressToString(WiFi.localIP()));
        local_IP = IpAddressToString(WiFi.localIP());
    }

    // for index.html
    server_OTA.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send_P(200, "text/html", index_html, processor); });

    // get the value from index.html
    server_OTA.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
//get value form webpage      
    strncpy(user_wifi.ssid,request->getParam("ssid")->value().c_str(), sizeof(user_wifi.ssid) );
    strncpy(user_wifi.password,request->getParam("password")->value().c_str(), sizeof(user_wifi.password) );
    user_wifi.ssid[request->getParam("ssid")->value().length()] = user_wifi.password[request->getParam("password")->value().length()] = '\0';  
//Svae EEPROM 
    EEPROM.put(0, user_wifi);
    EEPROM.commit();
//output wifi_sussce html;
    request->send_P(200, "text/html", wifi_sussce_html); });

    server_OTA.on("/compens", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      // get value form webpage
                      if (request->getParam("Btemp_fix")->value() != "")
                      {
                          user_wifi.btemp_fix = request->getParam("Btemp_fix")->value().toFloat();
                      }
                      if (request->getParam("Etemp_fix")->value() != "")
                      {
                          user_wifi.etemp_fix = request->getParam("Etemp_fix")->value().toFloat();
                      }
                      // Svae EEPROM
                      EEPROM.put(0, user_wifi);
                      EEPROM.commit();
                  });

    server_OTA.on("/other", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      // get value form webpage
                      if (request->getParam("sampling_time")->value() != "")
                      {
                          user_wifi.sampling_time = request->getParam("sampling_time")->value().toFloat();
                      }
                      if (request->getParam("sleeping_time")->value() != "")
                      {
                          user_wifi.sleeping_time = request->getParam("sleeping_time")->value().toInt() * 60; // input in MINUTES covernet to seconds
                      }
                      // Svae EEPROM
                      EEPROM.put(0, user_wifi);
                      EEPROM.commit();
                  });

    server_OTA.onNotFound(notFound); // 404 page seems not necessary...


    WebSerial.begin(&server_OTA);
    WebSerial.msgCallback(recvMsg);

    AsyncElegantOTA.begin(&server_OTA); // Start ElegantOTA


    server_OTA.begin();
   // WebSerial.println("HTTP server started");
    Serial.println("HTTP server started");

    mb.server();		//Start Modbus IP
    // Add SENSOR_IREG register - Use addIreg() for analog Inputs
    mb.addHreg(SENSOR_IREG);
}

void loop()
{



    checkLowPowerMode(BT_AvgTemp); //测量是否进入睡眠模式



}