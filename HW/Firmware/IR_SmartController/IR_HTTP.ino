String getBody(String url){  
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  Serial.printf("Response: %d", httpCode);
  Serial.println();
  if (httpCode == HTTP_CODE_OK) {
    String body = http.getString();
    /*Serial.print("Response Body: ");
    Serial.println(body);*/
    return body;
  }
  return "";
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
