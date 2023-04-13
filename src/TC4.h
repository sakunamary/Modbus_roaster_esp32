/*
    TC4-WB for Artisan Coffee Roaster Application

    Released under BSD-NC-3.0 License

    Created by Sakunamary on 2022

    define and web page raw data 
*/    

#ifndef __TC4_H__
#define __TC4_H__


//modules settings
#define ROLL_CONTROL
#define HAS_AP_INPUT


#if defined(ROLL_CONTROL)
    #define VERSION "1.0.0r"
#endif    


//
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define BAUDRATE 115200  //serial port baudrate

//pinout setting
#define LED_WIFI 18
#define RUN_MODE_SELECT 25
#define ROLL_IN  34
#define FAN_IN   35
#define ENC_CLK  33
#define ENC_DT   32
#define PWM_ROLL 27
#define PWM_FAN  26
#define PWM_HEAT 14
#define RXD2     16
#define TXD      17


//pwm setting 
#define PWM_FREQ 2000
#define PWM_RESOLUTION 12 //0-4096

// 网页设置的参数
 typedef struct eeprom_settings 
{
  char ssid[60]; //增加到30个字符
  char password[60]; //增加到30个字符
  float  btemp_fix;
  float  etemp_fix;
  float  ap_fix;
  uint32_t Thermo_msgID;
  uint32_t Airpressure_msgID;
  uint32_t PWMoutput_msgID;
   int PWM_FREQ_HEAT;
   int PWM_FREQ_FAN;
   int PWM_FREQ_ROLL;
  double sampling_time;//采样时间   单位：s
  bool   Init_mode ; //是否初始化模式
} user_wifi_t;

extern user_wifi_t  user_wifi ;


////////////////////////////////////////////////////////////////
//
//  web page raw data 
//
////////////////////////////////////////////////////////////////
const char wifi_sussce_html[] PROGMEM = R"rawliteral(
<!doctype html><html lang='cn'><head>
    <meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'>
        <title>Arti_Mod Wifi Setup</title>
        <style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}</style> 
        </head> 
<body>
    <main class='form-signin'> 
        <h1>Arti-Mod Setup OK</h1> <br/> 
        <p>Your settings have been saved successfully!<br />
        IF settings not working, Please do it again.<br />
        Please RESTART the device.<br />
        </p>
        <p>设置成功!<br />
        重启生效<br />
        如不成功请重复操作<<br />
        </p>
    </main>
</body></html>
)rawliteral";

const char index_html[] PROGMEM = R"rawliteral(
<!doctype html><html lang='cn'>
<head>
<script>
  function submitMessage() {
    alert("数据已保存");
    setTimeout(function(){ document.location.reload(false); }, 500);
  }
</script>
    <meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Arti-Mod Setup</title>
    <style>*,::after,::before{box-sizing:border-box;}
    body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';
    font-size:1rem;
    font-weight:400;
    line-height:1.5;color:#212529;
    background-color:#f5f5f5;}
    .form-control{
    display:block;
    width: 400px;
    height:calc(1.5em + .75rem + 2px);
    border:1px solid #ced4da;}
    button{border:1px solid transparent;
    color:#fff;
    background-color:#007bff;
    border-color:#007bff;
    padding:.5rem 1rem;
    font-size:1.25rem;
    line-height:1.5;
    border-radius:.3rem;
    width:400px}
    .form-signin{
    width: 400px;
    padding:15px;
    margin:auto;}
    h1,p{text-align:center}</style> 
</head> 
<body>
    <main class='form-signin'> 
        <form action='/get' method='get'>
            <h1 class=''>MODBUS CONTROLER SETTING </h1>
            <h2 class=''>WIFI设置 </h2>
            <div class='form-floating'>
            <label>SSID/WIFI名字</label>
            <input type='text' class='form-control' name='ssid'> 
            </div>
            <div class='form-floating'>
            <br/>
            <label>PASSWORD</label>
            <input type='password' class='form-control' name='password'>
            </div>
            <p>
            提示:输入空白即恢复AP模式直链模式
            </p>
            <br/>
            <button type='submit'>保存</button>
        </form>     
        <form action='/compens' method='get'>                
            <br/>
            <br/>
            <h2 class=''>热电偶温度补偿设置</h2>
            <div class='form-floating'>
            <label>Bean Temp/豆温 (当前值: %bt_compens%) </label>
            <input type='number' step = '0.01' max = '20' min='-20' class='form-control'  name='Btemp_fix'> 
            </div>
            <br/>
            <div class='form-floating'>
            <label>Env  Temp/炉温 (当前值:%et_compens%)</label>
            <input type='number' step = '0.01' max = '20' min='-20' class='form-control' name='Etemp_fix'> 
            </div>
            <br/>
            <div class='form-floating'>
            <label>Air Presure/风压差 (当前值:%AP_compens%)</label>
            <input type='number' step = '0.1' max = '20' min='-20' class='form-control' name='Ap_fix'> 
            </div>
            <br/>
            <button type='submit'onclick="submitMessage()">保存</button>
        </form> 
        <form action='/canbus' method='get'> 
            <br/>
            <br/>
            <h2 class=''>传感器模块设置</h2>
            <div class='form-floating'>
                <label>温度传感器CanID (当前ID: %thermo_msgID_compens%) </label>
                <input type='text'  class='form-control'  name='thermo_msgID'> 
                </div>
                <br/>
                <div class='form-floating'>
                <label>风压传感器CanID (当前ID:%air_msgID_compens%)</label>
                <input type='text'  class='form-control' name='air_msgID'> 
                </div>
                <br/>
                <div class='form-floating'>
                <label>PWM输出模块CanID (当前ID:%PWMOUT_msgID_compens%)</label>
                <input type='text'  class='form-control' name='PWMOUT_msgID'> 
                </div>
                <br/>
                <div class='form-floating'>
                    <label>热源PWM频率 (当前Hz:%HEAT_PWM_compens%)</label>
                    <input type='number' step = '100' max = '3000' min='0' class='form-control' name='HEAT_PWM'> 
                    </div>
                    <br/>
                <div class='form-floating'>
                    <label>风门PWM频率 (当前Hz:%FAN_PWM_compens%)</label>
                    <input type='number'  step = '100' max = '3000' min='0'class='form-control' name='FAN_PWM'> 
                    </div>
                    <br/>
                <div class='form-floating'>
                    <label>滚桶PWM频率 (当前Hz:%ROLL_PWM_compens%)</label>
                    <input type='number'  step = '100' max = '3000' min='0' class='form-control' name='ROLL_PWM'> 
                    </div>
                    <br/>                                            
                <button type='submit'onclick="submitMessage()">保存</button>

        </form>              
        <form action='/other' method='get'>   
            <br/>
            <br/>
            <div class='form-floating'>
            <h2 class=''>其他设置</h2>  
            <label>采样时间 (当前值: %sampling_time%) s</label>
            <input type='number' step = '0.25' max = '4' min='0.75' class='form-control'  name='sampling_time'> 
            </div>
            <br/>
  
            <button type='submit'onclick="submitMessage()">保存</button>
        </form> 
            <p>
            <a href='/update' target='_blank'>FIRMWARE UPDATE verison:%version%</a>
            </p>
            <br/>
    </main> 
</body></html>
)rawliteral";

#endif /*__TC4_H__*/
