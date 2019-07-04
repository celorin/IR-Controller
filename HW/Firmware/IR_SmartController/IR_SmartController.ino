//Store_CodeからRawCodeだけを取り出してHerokuのDBにPost。
//GetしたRawCodeをSend_Codeに渡して発光させる処理を作る。CodeTypeをSonyとか使わないでUnknownを使えば行けるかな


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

#define HOST_NAME ""//Herokuのサーバーaddr
#define HOST_PORT 80

void IR_post(/*引数忘れないでね*/);

const int IR_SND = 2;
const int IR_RCV = 4;
const int IR_RCV_POW = 16;
const int BUTTON = 25;
const int BUZZER = 26;
const int LEDPin = 27;

String User_ID = "";

boolean serverMode = false;

IPAddress ip;

IRsend irsend;
IRrecv irrecv(IR_RCV);
decode_results results;

//学習したIRコードが保存されるファイル
const char* ircode_file = "/ir_codes.txt";

// Wi-Fi設定保存ファイル
const char* settings = "/wifi_settings.txt";

// サーバモードでのパスワード
const String pass = "TDU_SmartCon";

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
/*
  File f = SPIFFS.open(settings, "r");
  String buf = f.readStringUntil();
  f.close();
  */

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

  delay(1000);
  if (serverMode) {
    server.handleClient();
    
  } else {
    if(WiFi.status() != WL_CONNECTED) {
      //WiFi.disconnect();
      Serial.println("Reconnect...");
      ESP.restart();
      //setup_client();
    }else{
      Serial.println(getBody("http://tdu-iot.herokuapp.com/api/get"));  
    }

    //クラッシュ原因ここ
    if(digitalRead(BUTTON) == 0){
      IR_snd();
    }else{
      IR_rev();
    //User_ID +".txt"を読みに行く処理
    }
    
    
  }
  
}
