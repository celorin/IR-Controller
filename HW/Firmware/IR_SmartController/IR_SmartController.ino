#include <Arduino.h>
#include <EEPROM.h>
#include <FS.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <WiFiClient.h> 
#include <WebServer.h>
//#include <WiFi.h>          //https://github.com/esp8266/Arduino
//#include <DNSServer.h>
//#include <WebServer.h>
//#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <IRremote.h>

const int IR_SND = 2;
const int IR_RCV = 4;
const int BUTTON = 25;
const int BUZZER = 26;
const int LEDPin = 27;

String User_ID ="";

IRsend irsend;
IRrecv irrecv(IR_RCV);
decode_results results;

// Wi-Fi設定保存ファイル
const char* settings = "/wifi_settings.txt";

// サーバモードでのパスワード
const String pass = "TDU_SmartCon";

WebServer server(80);

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
  html += "で設定されました。<br>これで設定は終了です。画面を閉じてください。";
  server.send(200, "text/html", html);
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
  
  Serial.println("SSID: " + ssid);
  Serial.println("PASS: " + pass);
  Serial.println("USER_ID: " + User_ID);

  WiFi.begin(ssid.c_str(), pass.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
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

void connection(){
  /*
  //WiFiManager
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();
  wifiManager.autoConnect("IR_SmartController");
  Serial.println("connected...");
  */
  //WiFiManagerを使わない方法
  delay(1000);
  setup_server();
}

void setup() {
 Serial.begin(9600); // シリアルモニタとの接続レート9600kbps
 pinMode(IR_SND, OUTPUT); // 出力ピンの設定
 pinMode(BUZZER, OUTPUT);
 pinMode(IR_RCV, INPUT); // 入力ピンの設定
 pinMode(BUTTON, INPUT);
 
 digitalWrite(IR_SND, LOW); // 初期状態をLOWにセット
 digitalWrite(BUZZER, LOW);
 
 SPIFFS.begin();
 
 if(digitalRead(BUTTON) == 0)
    connection();
 delay(1000);
 setup_client();
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
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
        Serial.print(" m");
      } 
      else {
        // Space
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
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
}

void sendCode(unsigned long codeValue, int codeLen) {
  if (codeType == NEC) {
      irsend.sendNEC(codeValue, RawcodeLen);
      Serial.print("Sent NEC ");
      Serial.println(codeValue, HEX);
  } 
  else if (codeType == SONY) {
    irsend.sendSony(codeValue, codeLen);
    Serial.print("Sent Sony ");
    Serial.println(codeValue, HEX);
  } 
  else if (codeType == PANASONIC) {
    irsend.sendPanasonic(codeValue, codeLen);
    Serial.print("Sent Panasonic");
    Serial.println(codeValue, HEX);
  }
  else if (codeType == JVC) {
    irsend.sendJVC(codeValue, codeLen, false);
    Serial.print("Sent JVC");
    Serial.println(codeValue, HEX);
  }
  else if (codeType == RC5 || codeType == RC6) {
    codeValue = codeValue & ~(1 << (codeLen - 1));
    codeValue = codeValue | (toggle << (codeLen - 1));
    if (codeType == RC5) {
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
    Serial.println("Sent raw");
  }
}

void IR_rev(){
  digitalWrite(BUZZER,HIGH);
  digitalWrite(LEDPin,HIGH);
  irrecv.enableIRIn();
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    storeCode(&results);
    irrecv.resume();
  }
  digitalWrite(BUZZER,LOW);
  digitalWrite(LEDPin,LOW);
}

void IR_snd(/*int num*/){
  digitalWrite(BUZZER,HIGH);
  sendCode(/*send raw code[num]*/);
  digitalWrite(BUZZER,LOW);
}

void loop() {
  server.handleClient();
}
