void createFile(fs::FS& fs, const char* path) {
  Serial.printf("Creating file: %s\n", path);

  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to create fille");
    return;
  }
  file.close();
}

void deleteFile(fs::FS& fs, const char* path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

// This is needed because WIFI API is ass
void beginWifiSTR(String ssid, String pw) {
  std::unique_ptr<char[]> cptr1_(new char[ssid.length() + 1]);
  ssid.toCharArray(cptr1_.get(), (ssid.length() + 1));

  std::unique_ptr<char[]> cptr2_(new char[pw.length() + 1]);
  pw.toCharArray(cptr2_.get(), (pw.length() + 1));

  WiFi.begin(cptr1_.get(), cptr2_.get());
}

void setBacklightLevel(uint8_t Value) {
  if (Value < minBrightness || Value > maxBrightness) {
    //printf("TFT_SET_BL Error \r\n");
  } else {
    analogWrite(TFT_BL, Value * 2.55);
  }
}

//void listDirNth(fs::FS& fs, const char* dirname, uint8_t levels, uint16_t nth, char* fileName);
//uint16_t countDir(fs::FS& fs, const char* dirname, uint8_t levels);

/**************************************GIF HELPERS****************************************/
void* fileOpenGif(const char* filename, int32_t* pFileSize) {
  File* f = new File();
  *f = SD.open(filename, FILE_READ);
  if (*f) {
    *pFileSize = f->size();
    return (void*)f;
  }
  delete f;
  return NULL;
}

void fileCloseGIF(void* pHandle) {
  File* f = static_cast<File*>(pHandle);
  if (f != NULL) {
    f->close();
    delete f;
  }
}

int32_t fileReadGIF(GIFFILE* pFile, uint8_t* pBuf, int32_t iLen) {
  File* f = static_cast<File*>(pFile->fHandle);
  if (f == NULL) return 0;
  int32_t iBytesRead = f->read(pBuf, iLen);
  pFile->iPos = f->position();
  return iBytesRead;
}

int32_t fileSeekGIF(GIFFILE* pFile, int32_t iPosition) {
  File* f = static_cast<File*>(pFile->fHandle);
  if (f == NULL) return 0;
  f->seek(iPosition);
  pFile->iPos = f->position();
  return pFile->iPos;
}


void listDirNth(fs::FS& fs, const char* dirname, uint8_t levels, uint16_t nth, char* fileName) {
  File root = fs.open(dirname);
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {

    } else {
      if (nth == 1) {
        strlcpy(fileName, file.name(), 32);
        return;

      } else {
        nth--;
      }
    }
    file = root.openNextFile();
  }
}

uint16_t countDir(fs::FS& fs, const char* dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);
  int n = 0;
  File root = fs.open(dirname);
  if (!root) {
    return 0;
  }
  if (!root.isDirectory()) {
    return 0;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
      n++;
    }
    file = root.openNextFile();
  }
  Serial.println("Found files" + String(n));
  return n;
}

void readFileConf(fs::FS& fs, const char* path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  //Serial.print("Read from file:");
  while (file.available()) {
    String temp = file.readString();
    //Serial.println(temp.c_str());
    //Get SSID, so second line, meaning we get substring from first \n to second \name
    char delimiter = '\n';
    int startIndex = 0;
    int currentLine = 0;
    // Iterate through the string
    for (int i = 0; i < temp.length(); i++) {
      if (temp[i] == delimiter) {
        // Extract substring
        String substring = temp.substring(startIndex, i);
        //Serial.println(substring);
        if (currentLine == 1) {
          // copy like cavemen
          strncpy(ssid, substring.c_str(), substring.length());

          ssid[substring.length() - 1] = 0;
          //ssid=substring.c_str();
        }
        // Update the start index
        startIndex = i + 1;
        currentLine++;
      }
    }

    // Print the last substring
    strncpy(password, temp.substring(startIndex).c_str(), temp.substring(startIndex).length());
    //password[temp.substring(startIndex).length()-1]=0;
    //password = temp.substring(startIndex).c_str();

    file.close();
  }
  Serial.println("Acquired SSID");
  Serial.println(ssid);
  Serial.println("Acquired pass");
  Serial.println(password);
}

void HTTPSreadString(const String url, String& result) {

  WiFiClientSecure client;

  // set secure client without certificate
  client.setInsecure();
  //create an HTTPClient instance
  HTTPClient https;

  //Initializing an HTTPS communication using the secure client
  //Serial.print("[HTTPS] begin...\n");
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
          //Serial.write(buff, c);
          file.write(buff, c);
          tft.drawString(String((float(lenFull)-float(len))/float(lenFull)*100)+"%", 0, 24);
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
}
else {
  Serial.printf("[HTTP] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
}
https.end();
}