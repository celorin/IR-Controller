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
  IR_post();//Herokuに送りたい(願望)
}

void sendCode(int codetype,unsigned int codeValue) {//これみて→https://qiita.com/cexen/items/5f9e7b28fe1ba4be1f50
  unsigned int codeLen = sizeof(codeValue)*8; // sizeof(unsigned int);
  if (codetype == NEC) {
    irsend.sendNEC(codeValue,codeLen);
    Serial.print("Sent NEC ");
    Serial.println(codeValue,HEX);
    Serial.println(codeLen);
  }
  else if (codetype == SONY) {
    irsend.sendSony(codeValue, codeLen);
    Serial.print("Sent Sony ");
    Serial.println(codeValue, HEX);
  }
  else if (codetype == PANASONIC) {
    irsend.sendPanasonic(codeValue, codeLen);
    Serial.print("Sent Panasonic");
    Serial.println(codeValue, HEX);
  }
  else if (codetype == JVC) {
    irsend.sendJVC(codeValue, codeLen, false);
    Serial.print("Sent JVC");
    Serial.println(codeValue, HEX);
  }
  else if (codetype == RC5 || codetype == RC6) {
    codeValue = codeValue & ~(1 << (codeLen - 1));
    codeValue = codeValue | (toggle << (codeLen - 1));
    if (codetype == RC5) {
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
  else if (codetype == UNKNOWN /* i.e. raw */) {
    // Assume 38 KHz
    Serial.println(codeValue);
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
  int cnt=0;
  WiFi.disconnect();
  irrecv.enableIRIn();
  delay(100);
  digitalWrite(BUZZER, HIGH);
  digitalWrite(LEDPin, HIGH);
  digitalWrite(IR_RCV_POW, HIGH);
  while(!(irrecv.decode(&results))) {
    Serial.print(".");
    delay(100);
    cnt++;
    if(cnt >= 100){
      break;  
    }
  }
  Serial.println(results.value, HEX);
  storeCode(&results);
  irrecv.resume();
  Serial.println("rev success!");
  irrecv.disableIRIn();
  delay(100);
  digitalWrite(BUZZER, LOW);
  digitalWrite(LEDPin, LOW);
  digitalWrite(IR_RCV_POW, LOW);
  WiFi.begin();
}

void IR_snd(String ir_code) {
  IRtype = "ここも"
  IRcode = "ここも"
  digitalWrite(BUZZER, HIGH);
  sendCode(IRtype,IRcode/*3,16711935*//*send raw code[]*/); //適当な引数入れてある　要変更　codeLenどうするか
  //irrecv.enableIRIn();
  delay(500);
  digitalWrite(BUZZER, LOW);
}
