
void fetchUpdates {
  if (WiFi.status() == WL_CONNECTED) {
    tft.drawString("Checking for new gifs", 0, 312);
    //HTTPSreadString("https://raynoxu.neocities.org/esp32-test.txt", downloadedString);
    String gifIndex;
    HTTPSreadString("https://raynoxu.neocities.org/gifPlayer-raynoxu/filesIndex.json", gifIndex);
    if (gifIndex.length() > 0) {
      JsonDocument responseJson;
      DeserializationError error = deserializeJson(responseJson, gifIndex);

      // Test if parsing succeeds.
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      const char* timestampFromInternets = responseJson["Timestamp"];
      Serial.println(timestampFromInternets);
    }
  }
}

void downloadHTTPS(const String url, const String path) {
  WiFiClientSecure client;
  tft.drawString("Downloading:", 0, 0);
  tft.drawString(url.c_str(), 0, 8);
  // set secure client without certificate
  client.setInsecure();
  //create an HTTPClient instance
  HTTPClient https;
  https.begin(client, url);
  int httpCode = https.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      tft.drawString("Downloading:", 0, 0);
      tft.drawString(url.c_str(), 0, 8);
      tft.drawString("Progress:", 0, 16);
      createFile(SD, path.c_str());
      File file = SD.open(path.c_str(), FILE_APPEND);
      int len = https.getSize();
      const int lenFull = len;
      uint8_t buff[128] = { 0 };
      NetworkClient* stream = https.getStreamPtr();
      // read all data from server
      while (https.connected() && (len > 0 || len == -1)) {
        // get available data size
        size_t size = stream->available();

        if (size) {
          // read up to 128 byte
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          // write it to Serial
          file.write(buff, c);
          tft.drawString(String((float(lenFull) - float(len)) / float(lenFull) * 100) + "%", 0, 24);
          if (len > 0) {
            len -= c;
          }
        }
        delay(1);
      }
      Serial.println();
      Serial.print("[HTTP] connection closed or file end.\n");
      file.close();
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
  }
  https.end();
}

void HTTPSreadString(const String url, String& result) {

  WiFiClientSecure client;

  // set secure client without certificate
  client.setInsecure();
  //create an HTTPClient instance
  HTTPClient https;

  if (https.begin(client, url)) {  // HTTPS
    //Serial.print("[HTTPS] GET...\n");
    // start connection and send HTTP header
    int httpCode = https.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      //Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        result = https.getString();
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  }
}