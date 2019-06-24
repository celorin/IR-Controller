#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <IRremote.h>

const int IR_SND = ;
const int IR_RCV = ;
const int BUTTON = ;
const int BUZZER = ;


void setup() {
 Serial.begin(9600); // シリアルモニタとの接続レート9600kbps
 pinMode(IR_SND, OUTPUT); // 出力ピンの設定
 pinMode(BUZZER, OUTPUT);
 pinMode(IR_RCV, INPUT); // 入力ピンの設定
 pinMode(BUTTON, INPUT);
 
 digitalWrite(IR_SND, LOW); // 初期状態をLOWにセット
 digitalWrite(BUZZER, LOW);

 connection();
}

void connection(){
  //WiFiManager
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();
  wifiManager.autoConnect("IR_SmartController");
  Serial.println("connected...");

  //WiFiManagerを使わない方法　https://qiita.com/exabugs/items/2f67ae363a1387c8967c
}

void 

void loop() {
 
}
