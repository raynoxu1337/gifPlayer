// Networking
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
// IO
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
// Support
#include <AnimatedGIF.h>
#include <Arduino.h>
#include "time.h"

//https://arduinojson.org/
#include <ArduinoJson.h>


// Haram, should move to read from SD card
char ssid[32] = "";
char password[32] = "";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

static int nthFrame = 0;

#define SD_CS_PIN 5
#define LCD_BACK_LIGHT_PIN 21

String uri = "raynoxu.neocities.org";

TFT_eSPI tft = TFT_eSPI();
AnimatedGIF gif;

// Playing files in folder
int nfiles = 0;

// State
//bool backlightOn = true;
const int minBrightness = 0;
const int maxBrightness = 100;
int brightness = 90;
String downloadedString = "";
bool SDerror = false;
struct tm timeinfo;


/**************************************SETUP****************************************/
void setup() {
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(0);
  tft.invertDisplay(0);
  tft.fillScreen(TFT_BLACK);

  setBacklightLevel(50);

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Error: Unable to initialize SD card!");
    tft.fillScreen(TFT_RED);
    tft.drawString("SD Error", 10, 10);
    tft.drawString("If SD is inside then restart device", 20, 10);
    while (1)
      ;
  }
  Serial.println("SD card initialized successfully.");

  readFileConf(SD, "/conf/wifi.txt");

  // Since we have Wifi credentials now we can connect to wifi
  WiFi.mode(WIFI_STA);
  beginWifiSTR(ssid, password);
  // Debug Wifi
  //while (WiFi.status() != WL_CONNECTED) {
  // Serial.print('.');
  // delay(1000);
  //}


  // could get time like this
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  //if(!getLocalTime(&timeinfo)){
  //  Serial.println("Failed to obtain time");
  //  return;
  //}
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  //deleteFile(SD, "/downloaded.gif");
  //downloadHTTPS("https://raynoxu.neocities.org/notilt.gif", "/downloaded.gif");

  nfiles = countDir(SD, "/", 0);
  gif.begin(GIF_PALETTE_RGB565_BE);
}
/************************************** MAIN LOOP ****************************************/
void loop() {

  uint16_t nthFile = nfiles + 1;
  while (nthFile > 1) {
    nthFile--;

    char fileNameOld[32] = "";
    listDirNth(SD, "/", 0, nthFile, fileNameOld);
    char fileName[32] = "/";
    strlcat(fileName, fileNameOld, 32);
    Serial.printf("Idx %u, fn %s \n", nthFile, fileName);

    time_t tick = millis();
    time_t fetchUpdateTimer = millis();
    while (millis() - tick < 10000) {
      nthFrame = 0;
      gif.setDrawType(GIF_DRAW_COOKED);
      if (gif.open(fileName, fileOpenGif, fileCloseGIF, fileReadGIF, fileSeekGIF, GIFDraw)) {
        tft.startWrite();
        while (gif.playFrame(true, NULL)) {
          nthFrame++;
          yield();
          // works kenough
          //tft.drawString(String(nthFrame), 0, 232);
          if (downloadedString.length() > 0) {
            tft.drawString(downloadedString, 0, 310);
          }
        }
        gif.close();
        tft.endWrite();
      }
    }
    if (nthFile == 1) { nthFile = nfiles + 1; }
    // Feth updates once a minute
    if (milis() - fetchUpdateTimer < 60000) {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Wifi gucci");
        HTTPSreadString("https://raynoxu.neocities.org/esp32-test.txt", downloadedString);
        // no point to use without https
      }
    }
  }
}
/******************************************************************************/
