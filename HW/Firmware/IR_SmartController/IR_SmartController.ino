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

void handleRootGet() {
  String html = "";
  html += "<h1>WiFi Settings</h1>";
  html += "<form method='post'>";
  html += "  Wi-FiのSSIDを入力してください。<br>";
  html += "  <input type='text' name='ssid' placeholder='ssid'><br>";
  html += "  Wi-Fiのパスワードを入力してください。<br>";
  html += "  <input type='text' name='pass' placeholder='pass'><br>";
  html += "  登録済みのユーザー名を入力してください。<br>";
  html += "  <input type='text' name='user_id' placeholder='user_id'><br>";
  html += "  <input type='submit'><br>";
  html += "</form>";
  server.send(200, "text/html", html);
}

void handleRootPost() {
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");
  String user_id = server.arg("user_id");

  File f = SPIFFS.open(settings, "w");
  f.println(ssid);
  f.println(pass);
  f.println(user_id);
  f.close();

  String html = "";
  html += "<h1>WiFi Settings</h1>";
  html += "SSID" + ssid + "<br>";
  html += "パスワード" + pass + "<br>";
  html += "ユーザー名" + user_id + "<br>";
  html += "で設定されました。<br>これで設定は終了です。本体が再起動します。画面を閉じてください。";
  server.send(200, "text/html", html);
  delay(1000);
  ESP.restart();
}

void setup_client() {
  File f = SPIFFS.open(settings, "r");
  String ssid = f.readStringUntil('\n');
  String pass = f.readStringUntil('\n');
  User_ID = f.readStringUntil('\n');
  f.close();

  ssid.trim();
  pass.trim();
  User_ID.trim();
  delay(1000);
  Serial.println("SSID: " + ssid);
  Serial.println("PASS: " + pass);
  Serial.println("USER_ID: " + User_ID);

  WiFi.begin(ssid.c_str(), pass.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LEDPin,HIGH);
    Serial.print(".");
    delay(250);
    digitalWrite(LEDPin,LOW);
    delay(250);
  }
  Serial.println("");

  Serial.println("WiFi connected");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_server() {
  String ssid = "IRSmartCon";
  String pass = "TDU_SmartCon";
  Serial.println("SSID: " + ssid);
  Serial.println("PASS: " + pass);

  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid.c_str(), pass.c_str());

  server.on("/", HTTP_GET, handleRootGet);
  server.on("/", HTTP_POST, handleRootPost);
  server.begin();
  Serial.println("HTTP server started.");
}

void connection() {
  /*
    //WiFiManager
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    wifiManager.autoConnect("IR_SmartController");
    Serial.println("connected...");
  */
  //WiFiManagerを使わない方法
  digitalWrite(BUZZER, HIGH);
  delay(1000);
  digitalWrite(BUZZER, LOW);
  setup_server();
}


int codeType = -1; // The type of code
unsigned long RawCode; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int RawcodeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state

void storeCode(decode_results *results) {
  codeType = results->decode_type;
  int count = results->rawlen;
  if (codeType == UNKNOWN) {
    Serial.println("Received unknown code, saving as raw");
    RawcodeLen = results->rawlen - 1;
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    for (int i = 1; i <= RawcodeLen; i++) {
      if (i % 2) {
        // Mark
        rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK - MARK_EXCESS;
        Serial.print(" m");
      }
      else {
        // Space
        rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK + MARK_EXCESS;
        Serial.print(" s");
      }
      Serial.print(rawCodes[i - 1], DEC);
    }
    Serial.println("");
  }
  else {
    if (codeType == NEC) {
      Serial.print("Received NEC: ");
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        Serial.println("repeat; ignoring.");
        return;
      }
    }
    else if (codeType == SONY) {
      Serial.print("Received SONY: ");
    }
    else if (codeType == PANASONIC) {
      Serial.print("Received PANASONIC: ");
    }
    else if (codeType == JVC) {
      Serial.print("Received JVC: ");
    }
    else if (codeType == RC5) {
      Serial.print("Received RC5: ");
    }
    else if (codeType == RC6) {
      Serial.print("Received RC6: ");
    }
    else {
      Serial.print("Unexpected codeType ");
      Serial.print(codeType, DEC);
      Serial.println("");
    }
    Serial.println(results->value, HEX);
    RawCode = results->value;
    RawcodeLen = results->bits;
  }
  IR_post(/*HerokuのDBに送るRawCode*/);
}

void sendCode(unsigned long codeValue,unsigned int codeLen) {//これみて→https://qiita.com/cexen/items/5f9e7b28fe1ba4be1f50
  if (results.decode_type == NEC) {
    irsend.sendNEC(codeValue, codeLen);
    Serial.print("Sent NEC ");
    Serial.println(codeValue, HEX);
  }
  else if (results.decode_type == SONY) {
    irsend.sendSony(codeValue, codeLen);
    Serial.print("Sent Sony ");
    Serial.println(codeValue, HEX);
  }
  else if (results.decode_type == PANASONIC) {
    irsend.sendPanasonic(codeValue, codeLen);
    Serial.print("Sent Panasonic");
    Serial.println(codeValue, HEX);
  }
  else if (results.decode_type == JVC) {
    irsend.sendJVC(codeValue, codeLen, false);
    Serial.print("Sent JVC");
    Serial.println(codeValue, HEX);
  }
  else if (results.decode_type == RC5 || results.decode_type == RC6) {
    codeValue = codeValue & ~(1 << (codeLen - 1));
    codeValue = codeValue | (toggle << (codeLen - 1));
    if (results.decode_type == RC5) {
      Serial.print("Sent RC5 ");
      Serial.println(codeValue, HEX);
      irsend.sendRC5(codeValue, codeLen);
    }
    else {
      irsend.sendRC6(codeValue, codeLen);
      Serial.print("Sent RC6 ");
      Serial.println(codeValue, HEX);
    }
  }
  else if (codeType == UNKNOWN /* i.e. raw */) {
    // Assume 38 KHz
    irsend.sendRaw(rawCodes, codeLen, 38);
    Serial.print("Sent raw ");
    for(int i=0;i<=codeLen;i++)
      Serial.print(rawCodes[i]);
    Serial.println();
  }
  /*irsend.sendRaw(results.rawbuf, codeLen, 38);
  Serial.println("Sent raw");
  */
  
}

void IR_rev() {
  digitalWrite(BUZZER, HIGH);
  digitalWrite(LEDPin, HIGH);
  digitalWrite(IR_RCV_POW, HIGH);
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    storeCode(&results);
    irrecv.resume();
    Serial.println("rev success!");
    delay(500);
  }
  irrecv.disableIRIn();
  digitalWrite(BUZZER, LOW);
  digitalWrite(LEDPin, LOW);
  digitalWrite(IR_RCV_POW, LOW);
}

void IR_snd() {
  digitalWrite(BUZZER, HIGH);
  sendCode(results.value,results.bits/*send raw code[]*/); //適当な引数入れてある　要変更　codeLenどうするか
  irrecv.enableIRIn();
  delay(500);
  digitalWrite(BUZZER, LOW);
}

void IR_post(/*RawCode*/) {
  if (client.connect(HOST_NAME, HOST_PORT)) {
    Serial.println("connected to server");
    client.print("GET ここにURLを指定する（ex. /celorin.php)?val=");
    client.print("");//ここにRawCodeを入れる
    client.print(" HTTP/1.1\r\n");
    client.print("HOST: ");
    client.println(ip);
    client.println("Connection: close");
    client.println();
  }
}

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
    irrecv.enableIRIn();
  }
  
}

void loop() {
  delay(300);
  if (serverMode) {
    server.handleClient();
  }
  else {
    if(WiFi.status() != WL_CONNECTED) {
      irrecv.disableIRIn();
      Serial.println("Reconnect...");
      ESP.restart();
    }
    if(digitalRead(BUTTON) == 0){
      IR_snd();
    }else{
      irrecv.enableIRIn();
      delay(200);
      IR_rev();
    }
    //User_ID +".txt"を読みに行く処理
  }
}
