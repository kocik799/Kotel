/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Social networks:            http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on NodeMCU.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right NodeMCU module
  in the Tools -> Board menu!

  For advanced settings please follow ESP examples :
   - ESP8266_Standalone_Manual_IP.ino
   - ESP8266_Standalone_SmartConfig.ino
   - ESP8266_Standalone_SSL.ino

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS D2       // пин подключения датчика температуры
float TEMP;                   // температура котла
int TEMP_INSTALL = 60;        // установка температуры
int MANUALSTATE = 0;          // ручной режим, начально автоматический режим
int UGOL_PODDUV_1 = 179;      //закрыта поддувало
int UGOL_PODDUV_2 = 90;       // открыто поддувало
int UGOL_PODDUV_3 = 152;      // поддержание поддувало
int UGOL_TYAGA_1 = 1;         // Открыт дымоход
int UGOL_TYAGA_2 = 45;        //закрыт на половину
int UGOL_TYAGA_3 = 90;        // закрыт полностью

int prevServo1 = 1;
int prevServo2 = 1;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

char auth[] = "YOU token";

// Your WiFi credentials.
// Set password to "" for open networks.
// You can also specify server:
//Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
//Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8442);
char ssid[] = "you ssid wifi";
char pass[] = "password";

Servo servo1;
Servo servo2;
int ReleyCool = D6;
//int  RelayState = 0;


void setup()
{
  // Debug console
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  sensors.begin();
  servo1.write(1); //начальное положение при загрузки
  servo2.write(1);  //начальное положение при загрузки
  servo1.attach(D0);  // поддувало
  servo2.attach(D1);  // дымоход
  pinMode(ReleyCool, OUTPUT); // реле вентилятора поддува
  digitalWrite(ReleyCool, LOW);
}

BLYNK_CONNECTED() {
  Blynk.syncAll();
}

BLYNK_WRITE(V5) {
  TEMP_INSTALL = param.asInt();
}

BLYNK_WRITE(V0) {
  if (MANUALSTATE == 1){
    setServopodduv(param.asInt());
  }
}

BLYNK_WRITE(V1) {
  if (MANUALSTATE == 1){
    setServotyaga(param.asInt());
  }
}

BLYNK_WRITE(V2) {
  if(MANUALSTATE != 0){
    int buttonState = param.asInt();
    digitalWrite(ReleyCool, buttonState);
  } else {
    Blynk.virtualWrite(V2, 0);
  }
}

BLYNK_WRITE(V3) {
   MANUALSTATE = param.asInt();
}

void loop()
{
  Blynk.run();
  sendTemps();   //измерение температуры котла
  if(MANUALSTATE == 0){
    termoStat();
  }
}

void sendTemps()
{
  sensors.requestTemperatures();
  //delay(500);
  TEMP = sensors.getTempCByIndex(0)+ 10;  //коллибровка температуры
 // Serial.println(TEMP);
  Blynk.virtualWrite(V4, TEMP);
  
}

void termoStat() {
    if (TEMP <= 50){
    setServopodduv(UGOL_PODDUV_2);
    setServotyaga(UGOL_TYAGA_3);
    Blynk.notify("Остываем  "+String(TEMP));
    
  }else if (TEMP < (TEMP_INSTALL - 7)&& TEMP > 50) {
    setServopodduv(UGOL_PODDUV_2);
    setServotyaga(UGOL_TYAGA_1);
    digitalWrite(ReleyCool, LOW);
    Blynk.virtualWrite(V2, 1);
    
  }else if (TEMP >= (TEMP_INSTALL -7) && TEMP < (TEMP_INSTALL + 5)) {
    setServopodduv(UGOL_PODDUV_3);
    digitalWrite(ReleyCool, HIGH); 
    Blynk.virtualWrite(V2, 0);
    
  } else if (TEMP >= (TEMP_INSTALL + 5) && TEMP < 115) {
    setServopodduv(UGOL_PODDUV_1);
    setServotyaga(UGOL_TYAGA_2);
    digitalWrite(ReleyCool, HIGH); 
    Blynk.virtualWrite(V2, 0);
    
  }else if (TEMP >= 115) {
    Blynk.notify("АХТУНГ!!!_ "+String(TEMP));
    setServopodduv(UGOL_PODDUV_1);
    setServotyaga(UGOL_TYAGA_3);
  }
}

void setServopodduv (byte newStateservo ){
  if (newStateservo != prevServo1){
    servo1.write(newStateservo);
    prevServo1 = newStateservo;
    Blynk.virtualWrite(V0,newStateservo);
  }else {
    Blynk.virtualWrite(V0,newStateservo);
  }
}

void setServotyaga (byte val ){
  if (val != prevServo2){
    servo2.write(val);
    prevServo2 = val;
    Blynk.virtualWrite(V1,val);
  }else{
    Blynk.virtualWrite(V1,val);
  }
}

