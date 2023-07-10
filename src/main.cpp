/* *****************************************************************
 * Blinker 是一套跨硬件、跨平台的物联网解决方案，提供APP端、设备端、
 * 服务器端支持，使用公有云服务进行数据传输存储。可用于智能家居、
 * 数据监测等领域，可以帮助用户更好更快地搭建物联网项目。
 * *****************************************************************/

/* *****************************************************************
点灯APP界面配置
{¨config¨{¨headerColor¨¨transparent¨¨headerStyle¨¨light¨¨background¨{¨img¨¨assets/img/headerbg.jpg¨¨isFull¨«}}¨dashboard¨|{¨type¨¨deb¨¨mode¨É¨bg¨É¨cols¨Ñ¨rows¨Ì¨key¨¨debug¨´x´É´y´¤C}{ßA¨tex¨¨t0¨´´¨t1¨¨操作提示¨¨size¨¤EßDÉ¨ico¨¨fad fa-hand-point-right¨ßEÍßFËßG¨TextKey¨´x´Í´y´Ì¨speech¨|÷¨lstyle¨Ì¨clr¨¨#076EEF¨}{ßA¨btn¨ßN¨fal fa-power-off¨ßCÉßJ¨开关机¨ßK¨文本2¨ßDÉßEËßFËßG¨ButtonKey1¨´x´Ê´y´ÏßQ|÷}{ßAßUßNßVßCÉßJ¨重启¨ßKßXßDÉßEËßFËßG¨ButtonKey2¨´x´Î´y´ÏßQ|÷}{ßAßIßJ´´ßK¨电脑状态¨ßM¤EßDÉßN¨fad fa-tv-retro¨ßEÍßFËßG¨TextKey2¨´x´É´y´ÌßQ|÷ßRÌßSßT}÷¨actions¨|¦¨cmd¨¦¨switch¨‡¨text¨‡¨on¨¨打开?name¨¨off¨¨关闭?name¨—÷¨triggers¨|{¨source¨ßg¨source_zh¨¨开关状态¨¨state¨|ßißk÷¨state_zh¨|¨打开¨¨关闭¨÷}÷} * *****************************************************************/
#define BLINKER_WIFI
#define BLINKER_ALIGENIE_OUTLET

#include <Arduino.h>
#include <Blinker.h>
#include <ESP8266WiFi.h> //智能配网
#include <main.h>

#define SwitchKey 4        // 开关控制引脚
#define RestartKey 5       // 重启控制引脚
#define statusdetectKey 13 // 状态检测引脚
WiFiManager wifiManager;
Ticker timer2;                    // 建立Ticker用于实现定时功能

/***************************手机APP接口***********************/
// 创建文本对象
#define TEXTE1 "TextKey"
BlinkerText Text1(TEXTE1);
#define TEXTE2 "TextKey2"
BlinkerText Text2(TEXTE2);

// 按钮1初始化, 创建对象，用于正常开关机及强制关机
#define BUTTON1 "ButtonKey1"
BlinkerButton Button1(BUTTON1);

// 按钮2初始化, 创建对象,用于电脑重启
#define BUTTON2 "ButtonKey2"
BlinkerButton Button2(BUTTON2);

// 按钮1数据处理回调函数-电脑开关机控制
void Button1Callback(const String &state)
{
  BLINKER_LOG("get button state: ", state);
  // 电脑开关机操作
  if (state == "tap")
  {
    // 检测电脑处于开机还是关机状态
    if (digitalRead(statusdetectKey) == HIGH) // 开机状态
    {
      digitalWrite(SwitchKey, HIGH); // 执行电脑关机
      delay(500);
      digitalWrite(SwitchKey, LOW);
      delay(15000); // 等待关机
      if (digitalRead(statusdetectKey) == LOW)
        Text1.print("关机成功");
      else
        Text1.print("关机失败");
    }
    else // 关机状态
    {
      digitalWrite(SwitchKey, HIGH); // 执行电脑开机
      delay(500);
      digitalWrite(SwitchKey, LOW);
      delay(15000); // 等待开机
      if (digitalRead(statusdetectKey) == LOW)
        Text1.print("开机成功");
      else
        Text1.print("开机失败");
    }
  }
  // 电脑强制关机操作
  if (state == "press") // 长按按键进行强制关机
  {
    if (digitalRead(statusdetectKey) == HIGH) // 检测电脑是否处于开机状态
    {
      Text1.print("强制关机中"); // 开机状态就执行电脑强制关机
      digitalWrite(SwitchKey, HIGH);
    }
  }
  if (state == "pressup") // 长按按键松开后检测是否关机
  {
    digitalWrite(SwitchKey, LOW);
    delay(7000);
    if (digitalRead(statusdetectKey) == HIGH)
      Text1.print("关机成功");
    else
      Text1.print("关机失败");
  }
}

// 按钮2数据处理回调函数-电脑重启控制及开关机状态查询
void Button2Callback(const String &state)
{
  BLINKER_LOG("get button state: ", state);

  if (state == "tap") // 电脑重启
  {
    digitalWrite(RestartKey, HIGH);
    delay(500);
    digitalWrite(RestartKey, LOW);
    Text1.print("电脑重启中");
  }
  if (state == "press") // 电脑状态查询
  {
    if (digitalRead(statusdetectKey) == HIGH)
      Text1.print("电脑已开机");
    else
      Text1.print("电脑已关机");
  }
}

// 自定义心跳函数，返回电脑状态
void heartbeat()
{
  if (digitalRead(statusdetectKey) == HIGH)
    Text2.print("电脑已开机");
  else
    Text2.print("电脑已关机");
}

/***********************天猫精灵接口************************/

// 天猫精灵开关类的操作接口
void aligeniePowerState(const String &state)
{
  BLINKER_LOG("need set power state: ", state);
  // 天猫精灵开机指令
  if (state == BLINKER_CMD_ON)
  {
    if (digitalRead(statusdetectKey) == LOW) // 检测电脑是否处于关机状态
    {
      digitalWrite(SwitchKey, HIGH); // 关机状态就执行电脑开机
      delay(500);
      digitalWrite(SwitchKey, LOW);
      BlinkerAliGenie.powerState("on");
      BlinkerAliGenie.print();
      delay(15000);                             // 等待开机
      if (digitalRead(statusdetectKey) == HIGH) // 检测是否开机成功
      {
        BlinkerAliGenie.powerState("on");
        BlinkerAliGenie.print();
      }
      else
      {
        BlinkerAliGenie.powerState("off");
        BlinkerAliGenie.print();
      }
    }
    else // 开机状态不执行动作
    {
      BlinkerAliGenie.powerState("on");
      BlinkerAliGenie.print();
    }
  }
  // 天猫精灵关机指令
  else if (state == BLINKER_CMD_OFF)
  {
    if (digitalRead(statusdetectKey) == HIGH) // 检测电脑是否处于开机状态
    {
      digitalWrite(SwitchKey, HIGH); // 开机状态就执行电脑关机
      delay(500);
      digitalWrite(SwitchKey, LOW);
      BlinkerAliGenie.powerState("off");
      BlinkerAliGenie.print();
      delay(15000);                            // 等待关机
      if (digitalRead(statusdetectKey) == LOW) // 检测是否关机成功
      {
        BlinkerAliGenie.powerState("off");
        BlinkerAliGenie.print();
      }
      else
      {
        BlinkerAliGenie.powerState("on");
        BlinkerAliGenie.print();
      }
    }
    else
    {
      BlinkerAliGenie.powerState("off");
      BlinkerAliGenie.print();
    }
  }
}

// 天猫精灵设备查询接口
void aligenieQuery(int32_t queryCode)
{
  BLINKER_LOG("AliGenie Query codes: ", queryCode);

  switch (queryCode)
  {
  case BLINKER_CMD_QUERY_ALL_NUMBER: // 所有属性查询
    BLINKER_LOG("AliGenie Query All");
    BlinkerAliGenie.powerState(digitalRead(13) == LOW ? "on" : "off");
    BlinkerAliGenie.print();
    break;
  case BLINKER_CMD_QUERY_POWERSTATE_NUMBER: // 查询电源状态
    BLINKER_LOG("AliGenie Query Power State");
    BlinkerAliGenie.powerState(digitalRead(13) == LOW ? "on" : "off");
    BlinkerAliGenie.print();
    break;
  default:
    BlinkerAliGenie.powerState("off");
    BlinkerAliGenie.print();
    break;
  }
}

// 检测收到未解析数据时的回调函数
void dataRead(const String &data)
{
  BLINKER_LOG("Blinker readString: ", data);
  int flag = data.toInt();
  if (flag == 1)
  {
    wifiManager.resetSettings();
    ESP.reset();
    Blinker.print("resetSettings and reset ESP");
  }
  Blinker.vibrate();
  uint32_t BlinkerTime = millis();
  Blinker.print("millis", BlinkerTime);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void setup()
{
  // 初始化串口服务
  Serial.begin(115200);
  BLINKER_DEBUG.stream(Serial);
  // 设置引脚模式并初始化引脚
  pinMode(SwitchKey, OUTPUT); // 开关机控制引脚
  digitalWrite(SwitchKey, LOW);
  pinMode(RestartKey, OUTPUT); // 重启控制引脚
  digitalWrite(RestartKey, LOW);
  pinMode(statusdetectKey, INPUT); // 电脑状态检测引脚
  init_littlefs();

  WiFiManagerParameter blinker_auth("auth", "blinker_auth(12位)", "", 13);
  WiFiManagerParameter host_hint("<small> Blinker_auth <br></small><br><br>");
  WiFiManagerParameter p_lineBreak_notext("<p></p>");
  wifiManager.setSaveConfigCallback(STACallback);
  wifiManager.setAPCallback(APCallback);
  // 配置连接超时，单位秒
  wifiManager.setConnectTimeout(10);
  // 设置 如果配置错误的ssid或者密码 退出配置模式
  wifiManager.setBreakAfterConfig(true);
  wifiManager.addParameter(&p_lineBreak_notext);
  // wifiManager.addParameter(&host_hint);
  wifiManager.addParameter(&blinker_auth);

  // wifiManager.resetSettings(); //reset saved settings
  if (!wifiManager.autoConnect("远程开机卡"))
  {
    // reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  if (shouldSaveConfig)
  {
    int ssid_len = wifiManager.getWiFiSSID().length() + 1;
    int pswd_len = wifiManager.getWiFiPass().length() + 1;
    wifiManager.getWiFiSSID().toCharArray(ssid, ssid_len);
    wifiManager.getWiFiPass().toCharArray(pswd, pswd_len);
    Serial.println(wifiManager.getWiFiSSID());
    Serial.println(wifiManager.getWiFiPass());
    Serial.println(ssid);
    Serial.println(pswd);
    strcpy(auth, blinker_auth.getValue());
    saveConfig();
    ESP.reset();
  }

  // Binker设备配置
  Serial.println(auth);
  Serial.println(ssid);
  Serial.println(pswd);
  Blinker.begin(auth, ssid, pswd);

  // 注册Blinker APP命令的回调函数
  Blinker.attachData(dataRead);
  Button1.attach(Button1Callback); // 按钮1回调函数注册
  Button2.attach(Button2Callback); // 按钮2回调函数注册
  Blinker.attachHeartbeat(heartbeat);
  timer2.attach(10, dht_flag_change);

  // 注册天猫精灵控制命令的回调函数
  BlinkerAliGenie.attachQuery(aligenieQuery);           // 注册设备查询函数
  BlinkerAliGenie.attachPowerState(aligeniePowerState); // 注册电源开关函数
}

void loop()
{
  if (dht_flag)
  {
    Serial.println(auth);
    Serial.println(ssid);
    Serial.println(pswd);
    // 检测blinker的auth值是否正确，正确blinker初始化成功，Blinker.init()值不会在一个循环内刷新，故放置在10s循环内
    if (!Blinker.init())
    {
      wifiManager.resetSettings(); // reset saved settings
      ESP.reset();
    }
    // 检测运行是否断网，WiFi.status()值不会立即刷新
    if (WiFi.status() == WL_CONNECTED)
      digitalWrite(LED_BUILTIN, HIGH);
    else
    {
      digitalWrite(LED_BUILTIN, LOW);
      // wifiManager.resetSettings(); // reset saved settings
      // ESP.reset();
    }
    dht_flag = 0;
  }
  Blinker.run();
}
