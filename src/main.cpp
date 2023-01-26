/*
  Modbus-Arduino Example - Test Analog Input (Modbus IP ESP8266)
  Read Analog sensor on Pin ADC (ADC input between 0 ... 1V)
  Original library
  Copyright by André Sarmento Barbosa
  http://github.com/andresarmento/modbus-arduino

  Current version
  (c)2017 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266


  https://github.com/lukeinator42/coffee-roaster
*/

#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else //ESP32
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>

//Modbus Registers Offsets
const int SENSOR_IREG = 100;

//ModbusIP object
ModbusIP mb;

long ts;

void setup() {
    Serial.begin(57600);
 
    WiFi.begin("rainly", "xnloveyyl");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    mb.server();		//Start Modbus IP
    // Add SENSOR_IREG register - Use addIreg() for analog Inputs
    mb.addHreg(SENSOR_IREG);

    ts = millis();
}

void loop() {
   //Call once inside loop() - all magic here
   mb.task();

   //Read each two seconds
   if (millis() > ts + 500) {
       ts = millis();
       //Setting raw value (0-1024)
       //mb.Ireg(SENSOR_IREG, analogRead(A0));
       mb.Hreg(SENSOR_IREG,1500);
   }
   delay(10);
}
