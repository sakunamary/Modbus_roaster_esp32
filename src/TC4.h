/*
    TC4-WB for Artisan Coffee Roaster Application

    Released under BSD-NC-3.0 License

    Created by Sakunamary on 2022

    define and web page raw data 
*/    

#ifndef __TC4_H__
#define __TC4_H__

#define PRINT_ARTISAN_WHOLE_MESSAGE 1     // set, to print Artisan commands on serial debug port
#define PRINT_TEAMPERATURE_EACH_READING 1 // Set, to print temperature vaule on serial debug port


#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define BAUDRATE 115200  //serial port baudrate


#define VERSION "1.0.0m"
#define LED_WIFI 4

// 网页设置的参数
 typedef struct eeprom_settings 
{
  char ssid[60]; //增加到30个字符
  char password[60]; //增加到30个字符
  float  btemp_fix;
  float  etemp_fix;
  double sampling_time;//采样时间   单位：s
  int    sleeping_time ;//休眠时间  单位：s
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
    alert("VALUE SAVED  数据已保存");
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
            <h1 class=''>Arti-Mod SETTING </h1>
            <h2 class=''>WIFI SETUP </h2>
            <div class='form-floating'>
            <label>SSID/WIFI名字</label>
            <input type='text' class='form-control' name='ssid'> 
            </div>
            <div class='form-floating'>
            <br/>
            <label>PASSWORD</label>
            <input type='password' class='form-control' name='password'>
            </div>
            <p>NOTICE:INPUT NULL will set back to AP mode <br/>
            提示:输入空白即恢复AP模式直链模式
            </p>
            <br/>
            <button type='submit'>SAVE WIFI INFO </button>
        </form>     
        <form action='/compens' method='get'>                
            <br/>
            <br/>
            <h2 class=''>THERMO COMPENSATE SETUP <br/>电偶温度补偿设置</h2>
            <div class='form-floating'>
            <label>Bean Temp/豆温 (current: %bt_compens%) </label>
            <input type='number' step = '0.01' max = '20' min='-20' class='form-control'  name='Btemp_fix'> 
            </div>
            <br/>
            <div class='form-floating'>
            <label>Env  Temp/炉温 (current:%et_compens%)</label>
            <input type='number' step = '0.01' max = '20' min='-20' class='form-control' name='Etemp_fix'> 
            </div>
            <br/>
            <button type='submit'onclick="submitMessage()">SAVE</button>
        </form> 
        <form action='/other' method='get'>   
            <br/>
            <br/>
            <div class='form-floating'>
            <h2 class=''>OTHER SETTING <br/>杂项</h2>  
            <label>Sampling 采样时间 (current: %sampling_time%) s</label>
            <input type='number' step = '0.25' max = '4' min='0.75' class='form-control'  name='sampling_time'> 
            </div>
            <br/>
            <div class='form-floating'>
            <label>Sleeping 休眠 (current:%sleeping_time%) Mins</label>
            <input type='number' step = '1' max = '30' min='5' class='form-control' name='sleeping_time'> 
            </div>
            <br/>
            <button type='submit'onclick="submitMessage()">SAVE</button>
        </form> 
            <p>
            <a href='/update' target='_blank'>FIRMWARE UPDATE verison:%version%</a>
            </p>
            <br/>
    </main> 
</body></html>
)rawliteral";

#endif /*__TC4_H__*/
