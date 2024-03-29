#include <Arduino.h>
#include <EEPROM.h>
#include <FS.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESP.h>
#include <HTTPClient.h>
//#include <WiFi.h>          //https://github.com/esp8266/Arduino
//#include <DNSServer.h>
//#include <WebServer.h>
//#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <IRremote.h>

#define HOST_NAME "http://tdu-iot.herokuapp.com/" //Herokuのサーバーaddr
#define HOST_PORT 80

void IR_post(/*引数忘れないでね*/);

const int IR_SND = 2;
const int IR_RCV = 19;
const int IR_RCV_POW = 16;
const int BUTTON = 22;
const int BUZZER = 32;
const int LEDPin = 21;
const int TMPPin = 34;

String User_ID = "";
String Recv_Data = "";
boolean serverMode = false;

IPAddress ip;

IRsend irsend;
IRrecv irrecv(IR_RCV);
decode_results results;

//学習したIRコードが保存されるファイル
const char* ircode_file = "/ir_codes.txt";

// Wi-Fi設定保存ファイル
const char* settings = "/wifi_settings.txt";

//サーバからGETしたbodyを保存しておくファイル
const char* rcvbody = "/rcv_body.txt";

// サーバモードでのパスワード
const String pass = "TDU_SmartCon";
//IPアドレスは 192.168.4.1

WebServer server(80);
WiFiClient client;


int codeType = -1; // The type of code
unsigned long RawCode; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int RawcodeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state


void setup() {
  Serial.begin(9600); // シリアルモニタとの接続レート9600kbps
  pinMode(BUZZER, OUTPUT);// 出力ピンの設定
  pinMode(LEDPin, OUTPUT);
  pinMode(IR_RCV_POW, OUTPUT);
  pinMode(BUTTON, INPUT);// 入力ピンの設定

  digitalWrite(BUZZER, LOW);// 初期状態をLOWにセット

  
  SPIFFS.begin();

  if (digitalRead(BUTTON) == 0) {
    serverMode = true;
    connection();
  }
  else {
    serverMode = false;
    delay(1000);
    setup_client();
    ip = WiFi.localIP();
  }
  
}

void loop() {
  static unsigned int cnt=0;
  delay(300);
  
  if (serverMode) {
    server.handleClient();
  }else {
    if(WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnect...");
      delay(100);
    }else{
      if(digitalRead(BUTTON) == 0){
        IR_rev();
        //IR_snd();
      }else{
        getBody();
        delay(100);
        Serial.println("送信中...");
      }
      if(cnt == 10){  //とりあえず10回ごと．0.8*10=8秒ごとくらい？
        Serial.println(getTmp());
        TMP_post();
        delay(100);
        cnt = 0;
      }
      cnt++;
    }
  }
}
