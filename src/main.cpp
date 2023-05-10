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




    pins useage 

    I2C SDA             GPIO21  
    I2C SCL             GPIO22
    BAT CHECK           GPIO34 
    CANTXD              GPIO5
    CANRXD              GPIO4  

    PWM HEAT            GPIO14  test OK 
    PWM FAN             GPIO26  testOK
    PWM ROLL            GPIO27  test ok option
    WIFI_SIGN           GPIO17   test OK

    ENCODER1 CLK        GPIO33
    ENCODER1 DT         GPIO32
    Roll analog         GPIO34
    Fan  analog         GPIO35

    RUN_MODE_SELECT         GPIO25

    ********** OUTPUT IO MAP **********
    1. GND
    2. 24V IN

    3. 5V out 
    4. GND
    5. PWM FAN OUT  
    6. FAN out

    7. PWM HEAT OUT
    8. PWM ROLL OUT

    9. 5V OUT
    10.GND
    11.CAN_H
    12.CAN_L

   ********** INPUT IO MAP **********

    1. GND
    2. 3.3V
    3. I2C SCL
    4. I2C SDA

    5. Fan  analog in
    6. MODE select in
    7. Roll analog in

    8.HEAT encoder pin1  
    9.HEAT encoder pin2  
    10. GND

    11.EN/RST
    12.3.3V
    13.GND
*/



#include <Arduino.h>
#include "TC4.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#include <ESP32CAN.h>
#include <CAN_config.h>

#include <ModbusIP_ESP8266.h>



#include <EEPROM.h>
#include <pwmWrite.h>
#include <ESP32Encoder.h>



#include "TC4_ThermalMeter.h"
#include "TC4_Indicator.h"



// Define three tasks
extern void TaskIndicator(void *pvParameters);
extern void TaskThermalMeter(void *pvParameters);


// define other functions
String IpAddressToString(const IPAddress &ipAddress);                         //转换IP地址格式
void notFound(AsyncWebServerRequest *request);                                // webpage function
String processor(const String &var);                                          // webpage function
bool getAutoRunMode(void);

// define variable


String BT_EVENT;
String local_IP;



char ap_name[30] ;
uint8_t macAddr[6];



uint16_t  heat_from_Hreg = 0;
uint16_t  heat_from_enc  = 0;
uint16_t  fan_from_Hreg  = 0;
uint16_t  fan_from_analog  = 0;
uint16_t  roll_from_Hreg  = 0;
uint16_t  roll_from_analog  = 0;


bool take_temp = true;
long timestamp;

//const uint32_t frequency = PWM_FREQ;
const byte resolution = PWM_RESOLUTION; //pwm -0-4096

int encoder_postion ;

TaskHandle_t xHandle_indicator;

//Modbus Registers Offsets
const int BT_HREG = 3001;
const int ET_HREG = 3002;
const int AP_HREG = 3003;
const int HEAT_HREG = 3004 ;
const int FAN_HREG  = 3005 ;
const int ROLL_HREG = 3006 ; 


//Coil Pins
const int HEAT_OUT_PIN = PWM_HEAT; //GPIO14
const int FAN_OUT_PIN = PWM_FAN;  //GPIO26
const int  ROLL_OUT_PIN = PWM_ROLL; //GPIO27




//ModbusIP object
ModbusIP mb;

//pwm object 
Pwm pwm = Pwm();

// rotary encoder object
ESP32Encoder encoder;

/*
     // Create a task, storing the handle.
     xTaskCreate( vTaskCode, "NAME", STACK_SIZE, NULL, tskIDLE_PRIORITY, &xHandle );

     // ...

     // Use the handle to suspend the created task.
     vTaskSuspend( xHandle );

*/
user_wifi_t user_wifi = {

                        " ",  //char ssid[60]; //增加到30个字符
                        " ", //char password[60]; //增加到30个字符
                        0.0, //float  btemp_fix;
                        0.0, //float  etemp_fix;
                        0.0, //float  ap_fix;
                        0x00, //uint32_t Thermo_msgID;
                        0x00, //uint32_t Airpressure_msgID;
                        0x00, //uint32_t PWMoutput_msgID;
                        PWM_FREQ, //int PWM_FREQ_HEAT;
                        PWM_FREQ, //int PWM_FREQ_FAN;
                        PWM_FREQ, //int PWM_FREQ_ROLL;
                        1.0, //double sampling_time;//采样时间   单位：s
                        false //bool   Init_mode ; //是否初始化模式
                        };
 
// object declare
AsyncWebServer server_OTA(80);


String IpAddressToString(const IPAddress &ipAddress)
{
    return String(ipAddress[0]) + String(".") +
           String(ipAddress[1]) + String(".") +
           String(ipAddress[2]) + String(".") +
           String(ipAddress[3]);
}

String processor(const String &var) //返回当前值到html页面
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
    else if (var == "AP_compens")
    {
        return String(user_wifi.ap_fix);
    }
    else if (var == "version")
    {
        return VERSION;
    }
    else if (var == "sampling_time")
    { //
        return String(user_wifi.sampling_time);
    }
    else if (var == "thermo_msgID_compens")
    { //
        return String(user_wifi.Thermo_msgID);
    }
     else if (var == "air_msgID_compens")
    { //
        return String(user_wifi.Airpressure_msgID);
    }
     else if (var == "PWMOUT_msgID_compens")
    { //
        return String(user_wifi.PWMoutput_msgID);
    }
     else if (var == "HEAT_PWM_compens")
    { //
        return String(user_wifi.PWM_FREQ_HEAT);
    }
     else if (var == "FAN_PWM_compens")
    { //
        return String(user_wifi.PWM_FREQ_FAN);
    }    
     else if (var == "ROLL_PWM_compens")
    { //
        return String(user_wifi.PWM_FREQ_ROLL);
    }

    return String();
}

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Opps....Not found");
}



void setup()
{
 Serial.begin(BAUDRATE);
    while (!Serial)
    {
        ; // wait for serial port ready
    }

    Serial.printf("\nArti-Mod  STARTING...\n");
    
    Serial.printf("\nSW version:%s\n",VERSION);

    Serial2.begin(BAUDRATE) ;

    while (!Serial2)
    {
        vTaskDelay(1000); // wait for serial port ready
        Serial.printf("\nSerial2 STARTING...\n");
    }
        Serial.printf("\nSerial2 OK!\n");

    Serial.printf("\nSetting INIT PINOUT ...\n");

    xThermoDataMutex = xSemaphoreCreateMutex();


    pinMode(LED_WIFI,OUTPUT);
    pinMode(HEAT_OUT_PIN, OUTPUT);
    pinMode(FAN_OUT_PIN, OUTPUT);   
    pinMode(RUN_MODE_SELECT,INPUT);
    pinMode(FAN_IN,INPUT);

#if defined(ROLL_CONTROL)
    pinMode(ROLL_OUT_PIN, OUTPUT);   
    pinMode(ROLL_IN,INPUT);
#endif     
    digitalWrite(LED_WIFI,LOW);

    Serial.printf("\nREAD data from EEPROM...\n");
    // set up eeprom data
    EEPROM.begin(sizeof(user_wifi));
    EEPROM.get(0, user_wifi);


   // user_wifi.Init_mode = true ;

if (user_wifi.Init_mode) 
{
    user_wifi.Init_mode = false ;
    user_wifi.sampling_time = 0.75; 
    user_wifi.btemp_fix = 0;
    user_wifi.etemp_fix = 0;
    user_wifi.ap_fix = 0 ;
    user_wifi.Thermo_msgID =0x0A6;
    user_wifi.Airpressure_msgID=0x0C6;
    user_wifi.PWMoutput_msgID=0x0B6;
    user_wifi.PWM_FREQ_HEAT = PWM_FREQ;
    user_wifi.PWM_FREQ_ROLL = PWM_FREQ;
    user_wifi.PWM_FREQ_FAN =PWM_FREQ;
    EEPROM.put(0, user_wifi);
    EEPROM.commit();
}
    Serial.printf("\nStart WIFI service...\n");

    //初始化网络服务
    WiFi.mode(WIFI_STA);
    WiFi.begin(user_wifi.ssid, user_wifi.password);



    byte tries = 0;
    while (WiFi.status() != WL_CONNECTED)
    {

        delay(1000);
        Serial.printf("\nWiFi connecting...\n");
        if (tries++ > 7)
        {
            // Serial_debug.println("WiFi.mode(AP):");
            WiFi.mode(WIFI_AP);
            sprintf( ap_name ,"MODBUS-%02X%02X%02X",macAddr[2],macAddr[1],macAddr[0]);
            WiFi.softAP(ap_name, "12345678"); // defualt IP address :192.168.4.1 password min 8 digis
            
            break;
        }
        // show AP's IP
    }


    Serial.print("MODBUS_CONTROL IP:");

    if (WiFi.getMode() == 2) // 1:STA mode 2:AP mode
    {
        Serial.println(IpAddressToString(WiFi.softAPIP()));
        local_IP = IpAddressToString(WiFi.softAPIP());
        WIFI_STATUS=true;
        digitalWrite(LED_WIFI,HIGH);
    }
    else
    {
        Serial.println(IpAddressToString(WiFi.localIP()));
        local_IP = IpAddressToString(WiFi.localIP());
        WIFI_STATUS=true;
        digitalWrite(LED_WIFI,HIGH);
    }

    
    Serial.println("");
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
#if defined(HAS_AP_INPUT)   
                      if (request->getParam("Ap_fix")->value() != "")
                      {
                          user_wifi.ap_fix = request->getParam("Ap_fix")->value().toFloat();
                      }
#endif 
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
                      // Svae EEPROM
                      EEPROM.put(0, user_wifi);
                      EEPROM.commit();
                  });

    server_OTA.on("/canbus", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      // get value form webpage
                      if (request->getParam("HEAT_PWM")->value() != "")
                      {
                          user_wifi.PWM_FREQ_HEAT = request->getParam("HEAT_PWM")->value().toInt();
                      }
                      if (request->getParam("FAN_PWM")->value() != "")
                      {
                          user_wifi.PWM_FREQ_FAN = request->getParam("FAN_PWM")->value().toInt();
                      }
                      if (request->getParam("ROLL_PWM")->value() != "")
                      {
                          user_wifi.PWM_FREQ_ROLL = request->getParam("ROLL_PWM")->value().toInt();
                      }

                      if (request->getParam("thermo_msgID")->value() != "")
                      {
                         user_wifi.Thermo_msgID = request->getParam("thermo_msgID")->value().toInt();
                         Serial.printf("\nSet user_wifi.Thermo_msgID:%d" ,user_wifi.Thermo_msgID);
                      }
                      
                      if (request->getParam("air_msgID")->value() != "")
                      {
                          user_wifi.Airpressure_msgID = request->getParam("air_msgID")->value().toInt();
                          Serial.printf("\nSet user_wifi.Airpressure_msgID:%d" ,user_wifi.Airpressure_msgID);
                      }
                      if (request->getParam("PWMOUT_msgID")->value() != "")
                      {
                          user_wifi.PWMoutput_msgID = request->getParam("PWMOUT_msgID")->value().toInt();
                      }

                      
                      // Svae EEPROM
                      EEPROM.put(0, user_wifi);
                      EEPROM.commit();
                  });



    server_OTA.onNotFound(notFound); // 404 page seems not necessary...


    Serial.printf("\nStart OTA service...\n");
    AsyncElegantOTA.begin(&server_OTA); // Start ElegantOTA

    Serial.printf("\nStart WEB  service...\n");
    server_OTA.begin();
   // WebSerial.println("HTTP server started");
    Serial.println("HTTP server started");




    Serial.printf("\nStart tasks service...\n");

    /*---------- Task Definition ---------------------*/
    // Setup tasks to run independently.

    xTaskCreatePinnedToCore(
        TaskThermalMeter, "ThermalMeter" // MAX6675 thermal task to read Bean-Temperature (BT)
        ,
        1024*2 // Stack size
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




    Serial.printf("\nStart CANBUS  service...\n");
//Init CANBUS 
    CAN_cfg.speed=CAN_SPEED_125KBPS;
    CAN_cfg.tx_pin_id = GPIO_NUM_5;
    CAN_cfg.rx_pin_id = GPIO_NUM_4;
    CAN_cfg.rx_queue = xQueueCreate(10,sizeof(CAN_frame_t));
    //start CAN Module
    ESP32Can.CANInit();


    Serial.printf("\nStart OUTPUT PWM  service...\n");
//Init pwm output
    pwm.pause();
    pwm.write(HEAT_OUT_PIN, 0, user_wifi.PWM_FREQ_HEAT, resolution);
    pwm.write(FAN_OUT_PIN, 0, user_wifi.PWM_FREQ_FAN, resolution);
#if defined(ROLL_CONTROL)    
    pwm.write(ROLL_OUT_PIN, 0, user_wifi.PWM_FREQ_ROLL, resolution);
#endif    
    pwm.resume();
    //pwm.printDebug();
    Serial.println("PWM started");  
    analogReadResolution(10); //0-1024


    Serial.printf("\nStart INPUT ENCODER  service...\n");
//init ENCODER
  encoder.attachHalfQuad( ENC_CLK,ENC_DT);
  encoder.setCount ( 0 );
  Serial.println("Encoder started");  

    Serial.printf("\nStart Modbus-TCP  service...\n");
//Init Modbus-TCP 
    mb.server(502);		//Start Modbus IP //default port :502
    // Add SENSOR_IREG register - Use addIreg() for analog Inputs
    mb.addHreg(BT_HREG);
    mb.addHreg(ET_HREG);


#if defined(HAS_AP_INPUT)   
    mb.addHreg(AP_HREG);
#endif

    mb.addHreg(HEAT_HREG);
    mb.addHreg(FAN_HREG);
    mb.Hreg(HEAT_HREG,0); //初始化赋值
    mb.Hreg(FAN_HREG,0);  //初始化赋值

#if defined(ROLL_CONTROL)   
    mb.addHreg(ROLL_HREG);   
    mb.Hreg(ROLL_HREG,0);  //初始化赋值
#endif    

   Serial.println("Modbus-TCP  started");  


timestamp=millis();


    //check  analog input ,keep in low 
    Serial.printf("\nCheck FAN and ROLL input ,keep them low ...\n");
    fan_from_analog = analogRead(FAN_IN);
    roll_from_analog =0;
#if defined(ROLL_CONTROL) 
    roll_from_analog =  analogRead(ROLL_IN) ;
#endif 


    while (fan_from_analog > 5 or roll_from_analog > 5) {

        vTaskSuspend(xHandle_indicator); //停止显示
        Serial.printf("\nFAN or ROLL input ERROR: \n");

        display.clear();
        display.setFont(ArialMT_Plain_16);
        display.drawString(30, 14-4 + 4,"INPUT ERROR");
        display.drawString(18, 30-4 + 4,"CHECK FAN &ROLL");
        display.drawRect(2, 2, 128-2, 64-2);
        display.display();
        vTaskDelay(1000);

        fan_from_analog = analogRead(FAN_IN);
#if defined(ROLL_CONTROL) 
        roll_from_analog =  analogRead(ROLL_IN) ;
#endif         

        Serial.printf("\nFAN_IN:%d,ROLL_IN:%d\n",fan_from_analog,roll_from_analog);
    }



}

void loop()
{
   //Call once inside loop() - all magic here
   mb.task();
   //Read each two seconds
   if (millis() > timestamp + 250) {
       timestamp = millis();
    mb.Hreg(BT_HREG,BT_CurTemp);
    mb.Hreg(ET_HREG,ET_CurTemp);
#if defined(HAS_AP_INPUT)   
    mb.Hreg(AP_HREG,AP_CurVal);
#endif

   }
   
   //Serial.printf("HEAT value : %f\n",mb.Hreg(HEAT_IREG));
    if (digitalRead(RUN_MODE_SELECT) == HIGH){ //自动模式
    
       //HEAT 控制部分 
       if (mb.Hreg(HEAT_HREG) <= 0 ) { //如果输入小于0值，自动限制在0
            heat_from_Hreg = 0;
            mb.Hreg(HEAT_HREG,heat_from_Hreg) ; //寄存器重新赋值为0

       } else if (mb.Hreg(HEAT_HREG) >= 100 ){//如果输入大于100值，自动限制在100
            heat_from_Hreg = 100;
            mb.Hreg(HEAT_HREG,heat_from_Hreg) ;//寄存器重新赋值为100

       } else {
            heat_from_Hreg = mb.Hreg(HEAT_HREG); //自动模式下，从寄存器获取heat的数值
            encoder.setCount(heat_from_Hreg); //同步encoder的步进数。
       }

       heat_from_enc = heat_from_Hreg ; //自动模式下，同步数据到encoder
       pwm.write(HEAT_OUT_PIN, map(heat_from_Hreg,0,100,0,4096), user_wifi.PWM_FREQ_HEAT, resolution); //自动模式下，将heat数值转换后输出到pwm


       //FAN  控制部分 
       if(mb.Hreg(FAN_HREG) <= 0){ //自动模式下，寄存器值限制在0
            fan_from_Hreg = 0;
            mb.Hreg(FAN_HREG,fan_from_Hreg); //寄存器重新赋值为0

       }else if (mb.Hreg(FAN_HREG) >= 100){ //自动模式下，寄存器值限制在0
            fan_from_Hreg =100 ;
            mb.Hreg(FAN_HREG,fan_from_Hreg);
       } else {
            fan_from_Hreg = mb.Hreg(FAN_HREG); //自动模式下，从寄存器读取fan的数值
       }
        pwm.write(FAN_OUT_PIN,map(fan_from_Hreg,0,100,0,4096),user_wifi.PWM_FREQ_FAN, resolution) ; //自动模式下，将fan数值转换后输出到pwm

#if defined(ROLL_CONTROL)   
       //ROLL  控制部分 
       if(mb.Hreg(ROLL_HREG) <= 0){ //自动模式下，寄存器值限制在0
            roll_from_Hreg = 0;
            mb.Hreg(ROLL_HREG,roll_from_Hreg); //寄存器重新赋值为0

       }else if (mb.Hreg(ROLL_HREG) >= 100){ //自动模式下，寄存器值限制在0
            roll_from_Hreg =100 ;
            mb.Hreg(ROLL_HREG,roll_from_Hreg);
       } else {
            roll_from_Hreg = mb.Hreg(ROLL_HREG); //自动模式下，从寄存器读取fan的数值
       }
        pwm.write(ROLL_OUT_PIN,map(roll_from_Hreg,0,100,0,4096),user_wifi.PWM_FREQ_ROLL, resolution) ; //自动模式下，将fan数值转换后输出到pwm
#endif


    } else {//手动模式
       //HEAT 控制部分 
       if (encoder.getCount() <0 ) {//如果输入小于0值，自动限制在0
            heat_from_enc = 0 ;
            encoder.setCount ( 0 ); //设置counter为0
       } else if (encoder.getCount() >100)
        {
            heat_from_enc = 100 ;//如果输入大于100值，自动限制在100
            encoder.setCount ( 100 );//设置counter为0
       } else {
            heat_from_enc = encoder.getCount();//设置counter为赋值
        }

       heat_from_Hreg = heat_from_enc; //手动模式下，同步数据到寄存器
       mb.Hreg(HEAT_HREG,heat_from_Hreg); //手动模式下，写入寄存器
       pwm.write(HEAT_OUT_PIN, map(heat_from_enc,0,100,0,4096), user_wifi.PWM_FREQ_HEAT, resolution); //自动模式下，将heat数值转换后输出到pwm


       //FAN  控制部分 
        fan_from_analog = analogRead(FAN_IN);   //获取模拟量信息
        fan_from_Hreg = map(fan_from_analog,0,1024,0,100); // 模拟量 1024 转为 100 
        mb.Hreg(FAN_HREG,fan_from_Hreg);//手动模式下，写入寄存器
        pwm.write(FAN_OUT_PIN,map(fan_from_Hreg,0,100,0,4096), user_wifi.PWM_FREQ_FAN, resolution);//手动模式下，将fan数值输出到pwm     

#if defined(ROLL_CONTROL)   
       //ROLL  控制部分 
        roll_from_analog = analogRead(ROLL_IN);   //获取模拟量信息
        roll_from_Hreg = map(roll_from_analog,0,1024,0,100); // 模拟量 1024 转为 100 
        mb.Hreg(ROLL_HREG,roll_from_Hreg);//手动模式下，写入寄存器
        pwm.write(ROLL_OUT_PIN,map(roll_from_Hreg,0,100,0,4096), user_wifi.PWM_FREQ_ROLL, resolution);//手动模式下，将fan数值输出到pwm     
#endif

    }
}