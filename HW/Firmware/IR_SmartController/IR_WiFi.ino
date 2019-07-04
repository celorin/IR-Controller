
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

void handleRootGet() {
  String html = "";
  html += "<h1>WiFi Settings</h1>";
  html += "<form method='post'>";
  html += "  Wi-FiのSSIDを入力してください。<br>";
  html += "  <input type='text' name='ssid' placeholder='ssid' required><br>";
  html += "  Wi-Fiのパスワードを入力してください。<br>";
  html += "  <input type='text' name='pass' placeholder='pass' required><br>";
  html += "  登録済みのユーザー名を入力してください。<br>";
  html += "  <input type='text' name='user_id' placeholder='user_id' required><br>";
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
