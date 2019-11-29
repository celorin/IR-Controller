int getTmp(){
  int analog , tmp, vt;
  analog = analogRead(TMPPin); 
  vt  = map(analog,0,4095,0,4095);    
  tmp = map(vt,300,1600,-30,100); 
  return tmp;
}

void TMP_post() {
  String send_url = "temp/";
  String send_val = "?temperature=";
  int tmp = getTmp();
  String post_url = HOST_NAME + send_url + User_ID + send_val + String(tmp);
  //String post_url=""; 
  //sprintf(post_url,"%s%s%s%s%s%s",HOST_NAME,"send/",User_ID,"?val=",String(results.decode_type),String(results.value));
  Serial.println(post_url);
  HTTPClient http;
  http.begin(post_url);
  int httpCode = http.GET();
  Serial.printf("Response: %d", httpCode);
  Serial.println();
}
