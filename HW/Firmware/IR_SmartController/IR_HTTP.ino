void split(String body,String split_body[]){
  int len = body.length() + 1;
  char p[len];
  body.toCharArray(p,len);
  int i=0,j=0;
  while(p[i] != '\0' && j<=2){
    if(p[i] == '\n' || p[i] == '?'){
      j++;
    }else{
      split_body[j] = split_body[j] + p[i];  
    }
    i++;
  }
  Serial.println(split_body[0]);
  Serial.println(split_body[1]);
  Serial.println(split_body[2]);
}

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
      
      File f = SPIFFS.open(rcvbody, "w");
      f.println(body);
      f.close();
      
      String body_split[5];
      split(body,body_split);
      if(body_split[0].equals(String("Learn_IR")))
        IR_rev();
      Serial.println("oppai");
      if(body_split[0].equals(String("Send_IR"))){
        IR_snd(body_split[1],body_split[2]);
      }
      Serial.println("tioppai");
    }
  }
}

void IR_post() {
  String send_url = "send/";
  String send_val = "?code=";
  String post_url = HOST_NAME + send_url + User_ID + send_val + String(results.decode_type) + String("?") + String(results.value);
  //String post_url=""; 
  //sprintf(post_url,"%s%s%s%s%s%s",HOST_NAME,"send/",User_ID,"?val=",String(results.decode_type),String(results.value));
  Serial.println(post_url);
  HTTPClient http;
  http.begin(post_url);
  int httpCode = http.GET();
  Serial.printf("Response: %d", httpCode);
  Serial.println();
}
