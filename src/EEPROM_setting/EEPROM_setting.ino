/*
   EEPROM Write

   Stores random values into the EEPROM.
   These values will stay in the EEPROM when the board is
   turned off and may be retrieved later by another sketch.
*/

#include "EEPROM.h"

#define PWM_FREQ 2500
#define BAUDRATE 115200 

// 网页设置的参数
 typedef struct eeprom_settings 
{
    char ssid[60]; //增加到30个字符
    char password[60]; //增加到30个字符
    float  btemp_fix;
    float  etemp_fix;
    float  ap_fix;
    uint32_t Thermo_canID;
    uint32_t AP_canID;
    int PWM_FREQ_HEAT;
    int PWM_FREQ_FAN;
    int PWM_FREQ_ROLL;
    double sampling_time;//采样时间   单位：s
     bool   Init_mode ; //是否初始化模式
} user_wifi_t;

extern user_wifi_t  user_wifi ;


void setup()
{
  Serial.begin(BAUDRATE);
  Serial.println("start...");
  if (!EEPROM.begin(sizeof(user_wifi)))
  {
    Serial.println("failed to initialise EEPROM"); 
    delay(1000000);
  } else {
    Serial.println("Initialed EEPROM,data will be writen after 3s..."); 
    delay(3000);
    EEPROM.get(0, user_wifi);
    
    strcat(user_wifi.ssid,"");
    strcat(user_wifi.password,"");
    user_wifi.Init_mode = false ;
    user_wifi.sampling_time = 0.75; 
    user_wifi.btemp_fix = 0;
    user_wifi.etemp_fix = 0;
    user_wifi.Thermo_canID = 0x0A6 ;
    user_wifi.AP_canID = 0x0C6 ;
    user_wifi.PWM_FREQ_HEAT = PWM_FREQ;
    user_wifi.PWM_FREQ_FAN = PWM_FREQ ;
    user_wifi.PWM_FREQ_ROLL = PWM_FREQ;


    EEPROM.put(0, user_wifi);
    EEPROM.commit();
  }

  Serial.println(" bytes read from Flash . Values are:");

  for (int i = 0; i < sizeof(user_wifi); i++)
  {
    Serial.print(byte(EEPROM.read(i))); Serial.print(" ");
  }

}


void loop()
{
  
}
