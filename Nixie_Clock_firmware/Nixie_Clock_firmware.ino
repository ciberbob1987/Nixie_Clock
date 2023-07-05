#include "StaticConfig.h"
#include "NixieTubesDigitMap.h"

#include <FFat.h>          // ESP32 core
#include <WiFi.h>          // ESP32 core

// NTP request
#include <WiFiUdp.h>           // ESP32 core

// DAC communication
#include <SPI.h>           // ESP32 core

// RTC DS3231 handling
#include <RTClib.h>        // https://github.com/adafruit/RTClib

// Time and date handling with DST and timezone 
#include <Timezone.h>      // https://github.com/JChristensen/Timezone
                           // require TimeLib https://github.com/PaulStoffregen/Time

// Web server
#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer
                               // require AsyncTCP https://github.com/me-no-dev/AsyncTCP

// JSON
#include <ArduinoJson.h>       // https://github.com/bblanchon/ArduinoJson

/*** MASTER CORE GLOBAL VARIABLES ***/
GlobalConfig  currConf;
QueueHandle_t masterQueue, slaveQueue;

RTC_DS3231        rtc;
SemaphoreHandle_t rtcSemaphore;

TimeChangeRule dstStart = {"DST", 0, 0, 0, 0, 0};
TimeChangeRule dstEnd = {"STD", 0, 0, 0, 0, 0};
Timezone       timeZoneObj(dstEnd);

TaskHandle_t   slaveCoreTaskHandler;

AsyncWebServer webServer(80);

uint8_t  masterCore, slaveCore;
bool     numOnTubes = false;

uint8_t  wifiTimeCounter;

uint32_t lastNTPSync;
bool     lastNTPSyncSuccess;
uint32_t currTime;

void setup() {
  
  masterCore = xPortGetCoreID();
  if (masterCore == 0)
    slaveCore = 1;
  else
    slaveCore = 0;
  
  Serial.begin(115200);
  Serial.printf("\n\nNixie Clock v%s running on core %u at %u MHz\n\n", FIRMWARE_V, masterCore, getCpuFrequencyMhz());
  
  if (!mainBootSequence()) {
    Serial.println();
    Serial.println("> Boot failed. Restarting");
    //ESP.restart();
  }
  
  Serial.println();
  Serial.println("> Boot complete. Starting loop");
  Serial.println();
  Serial.println();
}

bool mainBootSequence() {
  /* --------------------------------------------- */
  /* --- SPI bus, DAC and tubes inizialization --- */
  /* --------------------------------------------- */
  Serial.println("\n> DACs and tubes initialization");
  for (uint8_t nDac=0; nDac<DAC_NUM; nDac++) {
    pinMode(PIN_DAC_CS[nDac], OUTPUT);
    digitalWrite(PIN_DAC_CS[nDac], HIGH);
  }
  
  // Buffee enable
  pinMode(PIN_BUFF_EN, OUTPUT);
  digitalWrite(PIN_BUFF_EN, HIGH);
  
  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_SPI_SS);
  
  // Setting all DACs channels to 0V
  for (uint8_t nDac=0; nDac<DAC_NUM; nDac++)
    for (uint8_t nCh=0; nCh<DAC_CHANNELS; nCh++)
      writeDAC(nDac, nCh, 0);
  
  
  /* ---------------------------- */
  /* --- Queue initialization --- */
  /* ---------------------------- */
  Serial.println("\n> Initializing queues");
  masterQueue = xQueueCreate(MASTER_QUEUE_SIZE, sizeof(MASTER_QUEUE_EVENT));
  slaveQueue = xQueueCreate(SLAVE_QUEUE_SIZE, sizeof(SLAVE_QUEUE_EVENT));
  
  if (!(masterQueue && slaveQueue)) {
    Serial.println("Error initializing queues");
    // TODO: Print error code on tubes
    return false;
  }
  
  /* ------------------------------ */
  /* --- Storage initialization --- */
  /* ------------------------------ */
  for(uint8_t i=0; i<=1; i++) {
    Serial.println("\n> Mounting FAT volume");  
    if (FFat.begin()) break;
    else {
      Serial.println("failed. Webserver not available. Formatting");
      FFat.format();
      // TODO: Print error code on tubes
    }
  }
  
  
  /* -------------------- */
  /* --- Config files --- */
  /* -------------------- */
  Serial.println("\n> Reading config files");
  if (!readBinaryData(&currConf, sizeof(GlobalConfig), CURR_CONFIG_FILE)) {
    Serial.println("Using default config");
    saveBinaryData(&currConf, sizeof(GlobalConfig), CURR_CONFIG_FILE);
  }
  
  if (!readBinaryData(&lastNTPSync, sizeof(uint32_t), LAST_NTP_SYNC_FILE)) {
    Serial.println("Unable to read last NTP sync");
    lastNTPSync = 0;
    saveBinaryData(&lastNTPSync, sizeof(uint32_t), LAST_NTP_SYNC_FILE);
  }
  
  
  /* --------------------- */
  /* --- RGB LED setup --- */
  /* --------------------- */
  Serial.println("\n> Setting RGB LEDs channels");
  ledcSetup(PWM_CH_LED_R, LED_PWM_FREQ, 8);
  ledcAttachPin(PIN_LED_R, PWM_CH_LED_R);
  ledcWrite(PWM_CH_LED_R, 0);
  
  ledcSetup(PWM_CH_LED_G, LED_PWM_FREQ, 8);
  ledcAttachPin(PIN_LED_G, PWM_CH_LED_G);
  ledcWrite(PWM_CH_LED_G, 0);
  
  ledcSetup(PWM_CH_LED_B, LED_PWM_FREQ, 8);
  ledcAttachPin(PIN_LED_B, PWM_CH_LED_B);
  ledcWrite(PWM_CH_LED_B, 0);  
  
  setRGBLEDColor(currConf.rgbLedColor, currConf.rgbLedBri, currConf.rgbLedOn);
  
  
  /* ------------------------------- */
  /* --- RTC and time zone setup --- */
  /* ------------------------------- */
  Serial.println("\n> Setting time zone and RTC");
  setTimeZone();
  
  if(!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    // TODO: Print error code on tubes
    return false;
  }
  
  if(rtc.lostPower()) {
    Serial.println("RTC lost power, setting to 01/01/2000 00:00:00 local time");
    
    DateTime newTimeLocal(2000, 1, 1, 0, 0, 0);
    DateTime newTimeUTC(timeZoneObj.toUTC(newTimeLocal.unixtime()));
    
    rtc.adjust(DateTime(newTimeUTC));
    // TODO: Print error code on tubes
  }
  //we don't need the 32K Pin, so disable it
  rtc.disable32K();
  // set alarm 1, 2 flag to false (so alarm 1, 2 didn't happen so far)
  // if not done, this easily leads to problems, as both register aren't reset on reboot/recompile
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  // stop oscillating signals at SQW Pin
  // otherwise setAlarm1 will fail
  rtc.writeSqwPinMode(DS3231_OFF);

  // turn off alarm 2 (in case it isn't off already)
  // again, this isn't done at reboot, so a previously set alarm could easily go overlooked
  rtc.disableAlarm(2);
  
  // Setting Alarm1 to rise when seconds match 00
  if(!rtc.setAlarm1(DateTime(2000, 1, 1, 0, 0, 0), DS3231_A1_Second)) {
    Serial.println("Error, alarm wasn't set");
    // TODO: Print error code on tubes
    return false;
  }
  
  // rtc semaphore
  rtcSemaphore = xSemaphoreCreateMutex();
  
  /* ------------------------------- */
  /* --- Slave core regular task --- */
  /* ------------------------------- */
  Serial.println("\n> Starting loop on slave core");
  
  xTaskCreatePinnedToCore(
  coreSlaveTask, // Function to implement the task
  "coreSlaveT",  // Name of the task
  10000,         // Stack size in words
  NULL,          // Task input parameter
  1,             // Priority of the task
  &slaveCoreTaskHandler, // Task handle
  slaveCore);    // Core
  
  
  /* --------------- */
  /* --- Network --- */
  /* --------------- */
  Serial.println("\n> Initializing Wifi");
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.mode(WIFI_MODE_NULL);
  WiFi.setHostname(HOST_NAME);
  WiFi.onEvent(wifiEventHandler);
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  if (currConf.netAPMode) Serial.println("Mode: AP");
  else                    Serial.println("Mode: Client");
  
  Serial.printf("SSID: %s\n", currConf.netSSID);
  Serial.printf("Pass: %s\n", currConf.netKey);
  
  
  /* ------------------ */
  /* --- Web server --- */
  /* ------------------ */
  Serial.println("\n> Initializing web server");
  
  webServer.on("/json/version.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonStr = "{\"version\": \"";
    jsonStr = jsonStr + FIRMWARE_V;
    jsonStr = jsonStr + "\"}";
    
    sendJsonResponse(request, 200, jsonStr);
  });
  
  webServer.on("/json/time.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<256> doc;
    
    DateTime currTimeLocal(timeZoneObj.toLocal(currTime));
    DateTime lastSyncLocal(timeZoneObj.toLocal(lastNTPSync));
    
    doc["now"]["h"]  = currTimeLocal.hour();
    doc["now"]["m"]  = currTimeLocal.minute();
    doc["now"]["s"]  = currTimeLocal.second();
    doc["now"]["d"]  = currTimeLocal.day();
    doc["now"]["mt"] = currTimeLocal.month();
    doc["now"]["y"]  = currTimeLocal.year();
    
    if (lastNTPSync) {
      doc["last_sync"]["h"]  = lastSyncLocal.hour();
      doc["last_sync"]["m"]  = lastSyncLocal.minute();
      doc["last_sync"]["s"]  = lastSyncLocal.second();
      doc["last_sync"]["d"]  = lastSyncLocal.day();
      doc["last_sync"]["mt"] = lastSyncLocal.month();
      doc["last_sync"]["y"]  = lastSyncLocal.year();
    }
    else {
      doc["last_sync"]["h"]  = -1;
      doc["last_sync"]["m"]  = -1; 
      doc["last_sync"]["s"]  = -1;
      doc["last_sync"]["d"]  = -1;
      doc["last_sync"]["mt"] = -1;
      doc["last_sync"]["y"]  = -1;
    }
    
    String jsonStr;
    size_t error = serializeJson(doc, jsonStr);
    
    if (!error) sendPlainTextResponse(request, 500, "Json serialization error");
    else        sendJsonResponse(request, 200, jsonStr);
  });
  
  webServer.on("/json/net.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<256> doc;
    
    doc["ssid"]            = currConf.netSSID;
    doc["key"]             = HIDDEN_WIFI_KEY;
    doc["first_loading"]   = true;
    doc["ap_mode"]         = currConf.netAPMode;
    doc["ap_default_ssid"] = DEFAULT_AP_SSID;
    
    String jsonStr;
    size_t error = serializeJson(doc, jsonStr);
    
    if (!error) sendPlainTextResponse(request, 500, "Json serialization error");
    else        sendJsonResponse(request, 200, jsonStr);
  });
  
  webServer.on("/json/general.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<768> doc;
    
    doc["ntp_addr"] = currConf.ntpAddr;
    doc["ntp_sync"] = currConf.ntpSync;
    doc["ntp_sync_int_h"] = currConf.ntpSyncIntH;
    
    doc["time_zone_enable"] = currConf.timeZoneEnable;
    doc["time_zone_id"] = currConf.timeZoneId;
    doc["time_zone_offset"] = currConf.timeZoneOffset;
    
    doc["dst_enable"] = currConf.dstEnable;
    doc["dst_offset"] = currConf.dstOffset;
    
    doc["dst_start"]["week"] = currConf.dstStart.week;
    doc["dst_start"]["week_day"] = currConf.dstStart.weekDay;
    doc["dst_start"]["month"] = currConf.dstStart.month;
    doc["dst_start"]["h"] = currConf.dstStart.h;
    
    doc["dst_end"]["week"] = currConf.dstEnd.week;
    doc["dst_end"]["week_day"] = currConf.dstEnd.weekDay;
    doc["dst_end"]["month"] = currConf.dstEnd.month;
    doc["dst_end"]["h"] = currConf.dstEnd.h;
    
    for (uint8_t i=0; i<=3; i++) doc["tube_bri"][i] = currConf.nixieBri[i];
    doc["tube_bri"][4] = currConf.dotLampBri;
    
    doc["rgb_on"] = currConf.rgbLedOn;
    doc["rgb_bri"] = currConf.rgbLedBri;
    
    doc["rgb_color"][0] = (currConf.rgbLedColor & 0xFF0000) >> 16;
    doc["rgb_color"][1] = (currConf.rgbLedColor & 0x00FF00) >> 8;
    doc["rgb_color"][2] = (currConf.rgbLedColor & 0x0000FF);
    
    doc["cath_p_on"] = currConf.catPoiOn;
    doc["format_12_h"] = currConf.timeFormat12;
    
    String jsonStr;
    size_t error = serializeJson(doc, jsonStr);
    
    if (!error) sendPlainTextResponse(request, 500, "Json serialization error");
    else        sendJsonResponse(request, 200, jsonStr);
  });
  
  webServer.on("/update", HTTP_POST, onRequestEmptyHandler, onUploadEmptyHandler, execCmdHandler);
  
  webServer.serveStatic("/", FFat, "/www/").setDefaultFile("index.html");
  
  webServer.onNotFound([](AsyncWebServerRequest *request) {
    sendPlainTextResponse(request, 404, "Page not found");
  });
  
  webServer.begin();
  
  
  
  
  
  
  
  
  /* ---------------------------------- */
  /* --- Jobs to do just after boot --- */
  /* ---------------------------------- */
  {
    MASTER_QUEUE_EVENT qItem;
    
    if (currConf.ntpSync) {
      qItem = MASTER_QUEUE_EVENT::NTP_SYNC;
      xQueueSendToBack(masterQueue, &qItem, NULL);
    }
    
    qItem = MASTER_QUEUE_EVENT::CONNECT_WIFI;
    xQueueSendToFront(masterQueue, &qItem, NULL);
  }
  
  return true;
}

void sendJsonResponse(AsyncWebServerRequest *request, uint16_t code, const String& jsonSrt) {
  request->send(code, "application/json", jsonSrt);
}

void sendPlainTextResponse(AsyncWebServerRequest *request, uint16_t code, const String& message) {
  request->send(code, "text/plain", message);
}

void execCmdHandler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  StaticJsonDocument<UPD_JSON_MAX_SIZE> doc;
  
  DeserializationError error = deserializeJson(doc, data, len);
  
  if (doc["cmd"].isNull()) {
    sendPlainTextResponse(request, 500, "Bad command");
    return;
  }
  
  bool newRGBLedVal   = false;
  bool newTubesVal    = false;
  bool newTimeZoneVal = false;
  
  GlobalConfig newConf;
  
  memcpy(&newConf, &currConf, sizeof(GlobalConfig));
  
  uint16_t cmdCodeInt = doc["cmd"];
  COMMAND_CODE cmdCode = static_cast<COMMAND_CODE>(cmdCodeInt);
  
  switch (cmdCode) {
    case COMMAND_CODE::RGB_LEDS_CHANGE:
      newConf.rgbLedOn    = doc["rgb_on"];
      newConf.rgbLedBri   = doc["rgb_bri"];
      newConf.rgbLedColor = doc["rgb_color"];
      newRGBLedVal = true;
      break;
    
    case COMMAND_CODE::CATH_POI:
      newConf.catPoiOn = doc["cath_p_on"];
      break;
    
    case COMMAND_CODE::TUBES_BRIG:
      for (uint8_t i=0; i<4; i++) newConf.nixieBri[i] = doc["tube_bri"][i];
      newConf.dotLampBri = doc["tube_bri"][4];
      newTubesVal = true;
      break;
    
    case COMMAND_CODE::DST_UPD:
      newConf.dstEnable        = doc["dst_enable"];
      newConf.dstOffset        = doc["dst_offset"];
      newConf.dstStart.week    = doc["dst_start"]["week"];
      newConf.dstStart.weekDay = doc["dst_start"]["week_day"];
      newConf.dstStart.month   = doc["dst_start"]["month"];
      newConf.dstStart.h       = doc["dst_start"]["h"];
      newConf.dstEnd.week      = doc["dst_end"]["week"];
      newConf.dstEnd.weekDay   = doc["dst_end"]["week_day"];
      newConf.dstEnd.month     = doc["dst_end"]["month"];
      newConf.dstEnd.h         = doc["dst_end"]["h"];
      
      newTubesVal = true;
      newTimeZoneVal = true;
      break;
    
    case COMMAND_CODE::TIME_UPD:
      strncpy(newConf.ntpAddr, doc["ntp_addr"], CONF_STR_LEN);
      newConf.ntpSync     = doc["ntp_sync"];
      newConf.ntpSyncIntH = doc["ntp_sync_int_h"];
      
      newConf.timeFormat12   = doc["format_12_h"];
      newConf.timeZoneEnable = doc["time_zone_enable"];
      newConf.timeZoneId     = doc["time_zone_id"];
      newConf.timeZoneOffset = doc["time_zone_offset"];
      newTubesVal = true;
      newTimeZoneVal = true;
      
      if (!currConf.ntpSync && newConf.ntpSync) syncNTP();
      break;
    
    case COMMAND_CODE::NTP_SYNC:
      if (syncNTP()) {
        newTubesVal = true;
      }
      else {
        sendPlainTextResponse(request, 500, "NTP sync failed");
        return;
      }
      break;
    
    case COMMAND_CODE::TIME_SYNC:
      if (currConf.ntpSync) {
        sendPlainTextResponse(request, 500, "Manual sync failed. Clock set to sync from internet (NTP).");
        return;
      }
      else {
        uint8_t  day   = doc["d"];
        uint8_t  month = doc["mt"];
        uint16_t year  = doc["y"];
        
        uint8_t  hour = doc["h"];
        uint8_t  min  = doc["m"];
        uint8_t  sec  = doc["s"];
        
        DateTime newTimeLocal(year, month, day, hour, min, sec);
        DateTime newTimeUTC(timeZoneObj.toUTC(newTimeLocal.unixtime()));
        
        rtc.adjust(newTimeUTC);
        
        currTime = newTimeUTC.unixtime();
        
        newTubesVal = true;
      }
      break;
    
    case COMMAND_CODE::WIFI_UPD:
      newConf.netAPMode = doc["ap_mode"];
      if (!newConf.netAPMode) {
        strncpy(newConf.netSSID, doc["ssid"], CONF_STR_LEN);
        if (strcmp(HIDDEN_WIFI_KEY, doc["key"])) {
          strncpy(newConf.netKey, doc["key"], CONF_STR_LEN);
          Serial.println("\n\nDIVERSA\n\n");
        }
        else Serial.println("\n\nUGUALE\n\n");
      }
      break;
    
    default:
      sendPlainTextResponse(request, 500, "Bad command code");
      return;
      break;
  }
  
  if (cmdCode != COMMAND_CODE::NTP_SYNC &&
      cmdCode != COMMAND_CODE::TIME_SYNC &&
      !saveBinaryData(&newConf, sizeof(GlobalConfig), CURR_CONFIG_FILE)) {
    sendPlainTextResponse(request, 500, "Unable to save config file");
    return;
  }
  
  memcpy(&currConf, &newConf, sizeof(GlobalConfig));
  
  if (newTimeZoneVal) setTimeZone();
  if (newRGBLedVal) setRGBLEDColor(currConf.rgbLedColor, currConf.rgbLedBri, currConf.rgbLedOn);
  if (newTubesVal) displayTimeOnTubes(currTime);
  
  sendJsonResponse(request, 200, "{\"success\":true}");
  
  if (cmdCode == COMMAND_CODE::WIFI_UPD) {
    MASTER_QUEUE_EVENT qItemNew = MASTER_QUEUE_EVENT::REBOOT;
    xQueueSendToBack(masterQueue, &qItemNew, NULL);
  }
  
  return;
}

void onRequestEmptyHandler(AsyncWebServerRequest *request) {}

void onUploadEmptyHandler(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {}

void setTimeZone() {
  int16_t offsetTZ = 0;
  if (currConf.timeZoneEnable) offsetTZ = currConf.timeZoneOffset;

  dstEnd.week   = currConf.dstEnd.week;
  dstEnd.dow    = currConf.dstEnd.weekDay;
  dstEnd.month  = currConf.dstEnd.month;
  dstEnd.hour   = currConf.dstEnd.h;
  dstEnd.offset = offsetTZ;
  
  dstStart.week   = currConf.dstStart.week;
  dstStart.dow    = currConf.dstStart.weekDay;
  dstStart.month  = currConf.dstStart.month;
  dstStart.hour   = currConf.dstStart.h;
  dstStart.offset = offsetTZ + currConf.dstOffset;
  
  if (currConf.dstEnable)
    timeZoneObj.setRules(dstStart, dstEnd);
  else
    timeZoneObj.setRules(dstEnd, dstEnd);
}

void writeDAC(uint8_t nDac, uint8_t channel, uint8_t value) {
  digitalWrite(PIN_DAC_CS[nDac], LOW);
  
  SPI.beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE3));

  SPI.transfer(DAC_CH_ADDR[channel]);
  SPI.transfer(value);
  
  digitalWrite(PIN_DAC_CS[nDac], HIGH);
  digitalWrite(PIN_DAC_CS[nDac], LOW);
  SPI.endTransaction();
}

void setTubeDigit(uint8_t tube, int8_t digit) {
  float intensity = (( (float) currConf.nixieBri[tube] )/100.0 )*numTubes[tube].scaleDAC100;
  if (intensity > 255) intensity = 255;
  if (digit == -1)     intensity = 0;
  
  if ( (numTubes[tube].digitOn >= 0) && (numTubes[tube].digitOn <= 9) )
    writeDAC(numTubes[tube].chMap[numTubes[tube].digitOn].nDAC, numTubes[tube].chMap[numTubes[tube].digitOn].channel, 0);
  
  if (digit >= 0)
    writeDAC(numTubes[tube].chMap[digit].nDAC, numTubes[tube].chMap[digit].channel, (uint8_t) intensity);
  
  numTubes[tube].digitOn = digit;
}

void setDots(bool dotState) {
  for (uint8_t dot=0; dot<=1; dot++) {
    float intensity = 0;
    
    if (dotState) {
      intensity = (( (float) currConf.dotLampBri )/100.0 )*dotLamps[dot].scaleDAC100;
      if (intensity > 255) intensity = 255;
    }
    
    writeDAC(dotLamps[dot].chMap.nDAC, dotLamps[dot].chMap.channel, (uint8_t) intensity);
    
    dotLamps[dot].dotOn = dotState;
  }
}

void displayTimeOnTubes(uint32_t nowUtcUnixTime) {
  if (numOnTubes) return;
  
  DateTime nowTimeLocal(timeZoneObj.toLocal(nowUtcUnixTime));
  
  uint8_t hours   = nowTimeLocal.hour();
  uint8_t minutes = nowTimeLocal.minute();
  
  if (hours > 12 && currConf.timeFormat12) hours -= 12;
  
  uint8_t h1, h2, m1, m2;
  
  h1 = (hours/10U) % 10;
  h2 = hours % 10;
  
  m1 = (minutes/10U) % 10;
  m2 = minutes % 10;
  
  //Serial.printf("\n\n ----- %u - %u : %u - %u -----", h1, h2, m1, m2);
  
  setTubeDigit(0, h1);
  setTubeDigit(1, h2);
  setTubeDigit(2, m1);
  setTubeDigit(3, m2);
  
  setDots(true);
}

void displayNumberOnTubes(uint16_t number) {
  numOnTubes = true;
  
  uint8_t h1, h2, m1, m2;
  
  h1 = (number/1000U) % 10;
  h2 = (number/100U) % 10;
  
  m1 = (number/10U) % 10;
  m2 = number % 10;
  
  if (h1 == 0) setTubeDigit(0, -1);
  else         setTubeDigit(0, h1);
  
  if (h2 == 0) setTubeDigit(1, -1);
  else         setTubeDigit(1, h2);

  if (m1 == 0) setTubeDigit(2, -1);
  else         setTubeDigit(2, m1);

  if (m2 == 0) setTubeDigit(3, -1);
  else         setTubeDigit(3, m2);
  
  setDots(false);
}

void wifiEventHandler(WiFiEvent_t event) {
  
  Serial.printf("\n[WiFi-event] event: %d - ", event);
  
  switch (event) {
    case ARDUINO_EVENT_WIFI_READY: 
      Serial.println("WiFi interface ready");
      break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
      Serial.println("Completed scan for access points");
      break;
    case ARDUINO_EVENT_WIFI_STA_START:
      Serial.println("WiFi client started");
      break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
      Serial.println("WiFi clients stopped");
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("Connected to access point");
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("Disconnected from WiFi access point");
      break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
      Serial.println("Authentication mode of access point has changed");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("Obtained IP address: ");
      Serial.println(WiFi.localIP());
      break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
      Serial.println("Lost IP address and IP address is reset to 0");
      break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
      Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
      break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
      Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
      break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
      Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
      break;
    case ARDUINO_EVENT_WPS_ER_PIN:
      Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
      break;
    case ARDUINO_EVENT_WIFI_AP_START:
      Serial.println("WiFi access point started");
      break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
      Serial.println("WiFi access point  stopped");
      break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      Serial.println("Client connected");
      break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      Serial.println("Client disconnected");
      break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      Serial.println("Assigned IP address to client");
      break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
      Serial.println("Received probe request");
      break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
      Serial.println("AP IPv6 is preferred");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
      Serial.println("STA IPv6 is preferred");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP6:
      Serial.println("Ethernet IPv6 is preferred");
      break;
    case ARDUINO_EVENT_ETH_START:
      Serial.println("Ethernet started");
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("Ethernet stopped");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("Ethernet connected");
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("Ethernet disconnected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.println("Obtained IP address");
      break;
    default:
      break;
  }
}

bool wifiConnect() {
  Serial.println("n\> Starting wifi");
  
  if (currConf.netAPMode) {
    Serial.println("AP mode");
    
    if (WiFi.softAP(DEFAULT_AP_SSID, DEFAULT_AP_KEY)) {
      Serial.print("AP started. IP: ");
      Serial.println(WiFi.softAPIP());
    }
    else {
      Serial.println("Failed to start AP");
      return false;
    }
  }
  else {
    Serial.println("Client mode");
    
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(currConf.netSSID, currConf.netKey);
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      Serial.print("Connected. IP: ");
      Serial.println(WiFi.localIP());
    }
    else {
      Serial.println("Failed");
      return false;
    }
  }
  
  return true;
}

void wifiDisconnect() {
  Serial.println("n\> Disabling wifi");
  
  if (currConf.netAPMode) {
    WiFi.enableAP(false);
    WiFi.softAPdisconnect(true);
  }
  else {
    WiFi.disconnect(true);
  }
}

bool syncNTP() {
  Serial.println("n\> NTP sync");
  
  uint32_t secSince1900 = sendNTPpacket();
  
  if (!secSince1900) {
    Serial.println("Bad response. Skipping sync");
    return false;
  }
  
  uint32_t utcTimeInt = secSince1900 - SEVENTY_YEARS;
  DateTime utcTime(utcTimeInt);
  DateTime rtcTime;
  
  if (xSemaphoreTake(rtcSemaphore, RTC_TAKE_DELAY)) {
    rtc.adjust(utcTime);
    rtcTime = rtc.now();
    xSemaphoreGive(rtcSemaphore);
  }
  else {
    Serial.println("RTC busy");
    return false;
  }
  
  currTime = utcTimeInt;
  lastNTPSync = utcTimeInt;
  saveBinaryData(&utcTimeInt, sizeof(uint32_t), LAST_NTP_SYNC_FILE);
  
  char dateTimeStringFormat[] = "DDD DD-MM-YYYY hh:mm:ss";
  char auxBuffer[25];
  
  Serial.printf("NTP raw (seconds from 1900): %d\n", secSince1900);
  
  strcpy(auxBuffer, dateTimeStringFormat);
  utcTime.toString(auxBuffer);
  Serial.printf("NTP formatted: %s\n", auxBuffer);
  
  strcpy(auxBuffer, dateTimeStringFormat);
  rtcTime.toString(auxBuffer);
  Serial.printf("RTC UTC: %s\n", auxBuffer);
  
  DateTime localTime(timeZoneObj.toLocal(rtcTime.unixtime()));
  strcpy(auxBuffer, dateTimeStringFormat);
  localTime.toString(auxBuffer);
  Serial.printf("RTC local: %s\n", auxBuffer);
  
  return true;
}

uint32_t sendNTPpacket() {
  IPAddress timeServerIP;
  WiFi.hostByName(currConf.ntpAddr, timeServerIP);
  
  WiFiUDP udp;
  udp.begin(NTP_PORT);
  
  uint8_t packetBuffer[NTP_PACKET_SIZE];
  
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  
  // Initialize values needed to form NTP request
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  
  udp.beginPacket(timeServerIP, NTP_PORT);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
  
  uint32_t sendTime = millis();
  int16_t cb;
  uint16_t i = 0;
  
  do {
    delay(NTP_CYCLES_DELAY);
    cb = udp.parsePacket();
    i++;
  } while (!cb && (i<NTP_N_CYCLES));
  
  if (cb != NTP_PACKET_SIZE) return 0;
  
  udp.read(packetBuffer, NTP_PACKET_SIZE);
  
  //the timestamp starts at byte 40 of the received packet and is four bytes
  return (packetBuffer[40] << 24) | (packetBuffer[41] << 16) | (packetBuffer[42] << 8) | packetBuffer[43];
}

void setRGBLEDColor(uint32_t hexColor, uint8_t brightness, bool ledOn) {
  if (!ledOn) {
    ledcWrite(PWM_CH_LED_R, 0);
    ledcWrite(PWM_CH_LED_G, 0);
    ledcWrite(PWM_CH_LED_B, 0);
    
    return;
  }
  
  uint8_t red = (hexColor & 0xFF0000) >> 16;
  uint8_t green = (hexColor & 0x00FF00) >> 8;
  uint8_t blue = (hexColor & 0x0000FF);
  
  ledcWrite(PWM_CH_LED_R, (uint8_t)(red*brightness/100));
  ledcWrite(PWM_CH_LED_G, (uint8_t)(green*brightness/100));
  ledcWrite(PWM_CH_LED_B, (uint8_t)(blue*brightness/100));
}

bool readBinaryData(void* destPnt, const size_t size, const char* filename) {
  Serial.print("Reading ");
  Serial.print(filename);
  Serial.print(" ...");
  
  if (!FFat.exists(filename)) {
    Serial.println("File not found");
    return false;
  }
    
  File binFile = FFat.open(filename, FILE_READ);
    
  if (!binFile) {
    Serial.println("Error opening file");
    return false;
  }
  
  if (binFile.size() != size) {
    Serial.println("Wrong file size");
    return false;
  }
  
  byte tmpBuffer[size];
  size_t bytesRd = binFile.read((uint8_t*) &tmpBuffer, size);
  binFile.close();
  
  if (bytesRd != size) {
    Serial.println("Error reading file");
    return false;
  }
  
  memcpy(destPnt, &tmpBuffer, size);
  
  Serial.println(STR_OK);
  return true;
}

bool saveBinaryData(const void* sourcePnt, const size_t size, const char* filename) {
  Serial.print("Saving ");
  Serial.print(filename);
  Serial.print(" ...");
  
  if (FFat.exists(filename) && !FFat.remove(filename)) {
    Serial.println("Error deleting old file");
    return false;
  }
    
  File binFile = FFat.open(filename, FILE_WRITE);
    
  if (!binFile) {
    Serial.println("Error opening file");
    return false;
  }
    
  size_t bytesWr = binFile.write((const uint8_t*) sourcePnt, size);
  binFile.close();
    
  if (bytesWr != size) {
    Serial.println("Error writing file");
    return false;
  }
  
  Serial.println(STR_OK);
  return true;
}

void loop() {
  MASTER_QUEUE_EVENT qItem;
  
  if( xQueueReceive( masterQueue, &qItem, ( TickType_t ) 0) == pdPASS ) {
    
    if (qItem == MASTER_QUEUE_EVENT::CONNECT_WIFI) {
      if (wifiConnect()) wifiTimeCounter = 0;
    }
    
    if (qItem == MASTER_QUEUE_EVENT::RTC_MINUTE_TOCK) {
      wifiTimeCounter++;
      if (wifiTimeCounter == WIFI_ON_TIME_MIN) wifiDisconnect();
      
      if (((currTime >= lastNTPSync + currConf.ntpSyncIntH*3600) || !lastNTPSyncSuccess) && !currConf.netAPMode) {
        MASTER_QUEUE_EVENT qItemNew = MASTER_QUEUE_EVENT::NTP_SYNC;
        xQueueSendToBack(masterQueue, &qItemNew, NULL);
      }
    }
    
    if (qItem == MASTER_QUEUE_EVENT::NTP_SYNC) {
      if (!currConf.netAPMode) {
        lastNTPSyncSuccess = false;
        if (WiFi.status() == WL_CONNECTED) {
          lastNTPSyncSuccess = syncNTP();
          if (lastNTPSyncSuccess) {
            SLAVE_QUEUE_EVENT qItem = SLAVE_QUEUE_EVENT::UPDATE_TIME_ON_TUBES;
            xQueueSendToFront(slaveQueue, &qItem, NULL);
          }
        }
        else {
          MASTER_QUEUE_EVENT qItemNew = MASTER_QUEUE_EVENT::CONNECT_WIFI;
          xQueueSendToFront(masterQueue, &qItemNew, NULL);
        }
      }
    }
    
    if (qItem == MASTER_QUEUE_EVENT::REBOOT) {
      static uint32_t startTime = millis();
      if (millis() > startTime + REBOOT_DELAY_MS)
        ESP.restart();
      else
        xQueueSendToFront(masterQueue, &qItem, NULL);
      
    }
  }
}



/* SLAVE CORE */
void IRAM_ATTR RTCAlarmIsr() {
  SLAVE_QUEUE_EVENT qItemSlave = SLAVE_QUEUE_EVENT::RTC_ALARM_FIRED;
  xQueueSendToFrontFromISR(slaveQueue, &qItemSlave, NULL);
}

void coreSlaveTask(void* parameter) {
  rtc.clearAlarm(1);
  attachInterrupt(PIN_RTC_SQW, RTCAlarmIsr, FALLING);
  
  currTime = rtc.now().unixtime();
  displayTimeOnTubes(currTime);
  //displayNumberOnTubes(21);
  
  while (true) {
    SLAVE_QUEUE_EVENT qItem;
    
    if( xQueueReceive( slaveQueue, &qItem, ( TickType_t ) 0) == pdPASS ) {
      
      if (qItem == SLAVE_QUEUE_EVENT::UPDATE_TIME_ON_TUBES || qItem == SLAVE_QUEUE_EVENT::RTC_ALARM_FIRED) {
        //Serial.println("Update time on tubes");
        
        if (xSemaphoreTake(rtcSemaphore, RTC_TAKE_DELAY)) {
          if (rtc.alarmFired(1)) rtc.clearAlarm(1);
          currTime = rtc.now().unixtime();
          xSemaphoreGive(rtcSemaphore);
          
          displayTimeOnTubes(currTime);
        }
        else {
          xQueueSendToFrontFromISR(slaveQueue, &qItem, NULL);
        }
      }
      
      
      if (qItem == SLAVE_QUEUE_EVENT::RTC_ALARM_FIRED) {
        MASTER_QUEUE_EVENT qItemMaster = MASTER_QUEUE_EVENT::RTC_MINUTE_TOCK;
        xQueueSendToBackFromISR(masterQueue, &qItemMaster, NULL);
      }
    }
    
    vTaskDelay(2);
  }
}