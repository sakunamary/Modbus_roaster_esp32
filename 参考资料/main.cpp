
#include <Arduino.h>
#include <max6675.h>
#include <ModbusRtu.h>

// data array for modbus network sharing
uint16_t au16data[16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1};

/**
 *  Modbus object declaration
 *  u8id : node id = 0 for master, = 1..247 for slave
 *  u8serno : serial port (use 0 for Serial)
 *  u8txenpin : 0 for RS-232 and USB-FTDI 
 *               or any pin number > 1 for RS-485
 */
Modbus slave(1,Serial,0); // this is slave @1 and RS-232 or USB-FTDI

int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

int relay = 9;  
  
void setup() {
  slave.begin(9600); // 19200 baud, 8-bits, even, 1-bit stop
  slave.start();
  // use Arduino pins 
  pinMode(relay, OUTPUT);
 delay(500);
  
}

void loop() {
   //write current thermocouple value
   au16data[2] = 1340;
   //au16data[2] = ((uint16_t) thermocouple.readCelsius()*100);
   au16data[3] = 1230;
 
   //poll modbus registers
   slave.poll( au16data, 16 );

   //write relay value using pwm
  // analogWrite(relay, (au16data[4]/100.0)*255);
    au16data[5] = au16data[4]*100;
   delay(200);
}




//******************************第二个方法**************************/

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
    Serial.begin(115200);
 
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
   if (millis() > ts + 2000) {
       ts = millis();
       //Setting raw value (0-1024)
       mb.Hreg(SENSOR_IREG,23500);
   }
   delay(10);
}
