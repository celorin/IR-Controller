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
