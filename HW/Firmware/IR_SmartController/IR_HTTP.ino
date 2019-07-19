void getBody(){ 
  String url = HOST_NAME + String("api/") + User_ID;
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  Serial.printf("Response: %d", httpCode);
  Serial.println();
  if (httpCode == HTTP_CODE_OK) {
    String body = http.getString();
    if(!Recv_Data.equals(body)){
      Recv_Data = body;
      String meirei = "Stringをなんとかして\nで分割して"
      if(meirei.equals("Lean_IR"))IR_rev();
      if(meirei == "Send_IR"){
        String ir_code = "これもよろしく"
        IR_snd(ir_code);
      }
    }
  }
}

void IR_post() {
  String send_url = "send/";
  String send_val = "?val=";
  String post_url = HOST_NAME + send_url + User_ID + send_val + String(results.decode_type) + String("!") + String(results.value);
  //String post_url=""; 
  //sprintf(post_url,"%s%s%s%s%s%s",HOST_NAME,"send/",User_ID,"?val=",String(results.decode_type),String(results.value));
  Serial.println(post_url);
  /*HTTPClient http;
  http.begin(post_url);
  int httpCode = http.GET();
  Serial.printf("Response: %d", httpCode);
  Serial.println();*/
}
