#include "StaticConfig.h"
#include "NixieTubesDigitMap.h"

// WiFi network handling
#include <WiFi.h>               // ESP32 core

// FFAT partition handling
#include <FFat.h>               // ESP32 core

// NVS partition handling
#include <Preferences.h>        // ESP32 core

// UTP needed for NTP request
#include <WiFiUdp.h>            // ESP32 core

// SPI needed for DAC communication
#include <SPI.h>                // ESP32 core

// HTTP client for OTA updates
#include <HTTPClient.h>         // ESP32 core

// RTC DS3231 and DateTime handling
#include <RTClib.h>             // https://github.com/adafruit/RTClib

// DST and timezone handling
#include <Timezone.h>           // https://github.com/JChristensen/Timezone
                                // require TimeLib https://github.com/PaulStoffregen/Time

// Web server
#include <ESPAsyncWebServer.h>  // https://github.com/me-no-dev/ESPAsyncWebServer
                                // require AsyncTCP https://github.com/me-no-dev/AsyncTCP

// JSON handling
#include <ArduinoJson.h>        // https://github.com/bblanchon/ArduinoJson

// OTA updates from web server
#include <EnhancedHTTPUpdate.h> // https://github.com/ciberbob1987/EnhancedHTTPUpdate

// Semantic version compare
#include <SemanticVersion.h>   // https://github.com/ciberbob1987/SemanticVersion

/*** MASTER CORE GLOBAL VARIABLES ***/
GlobalConfig  currConf;
QueueHandle_t masterQueue, slaveQueue;

RTC_DS3231        rtc;

SemaphoreHandle_t rtcSemaphore;
SemaphoreHandle_t nvsSemaphore;
SemaphoreHandle_t tubesSemaphore;

TimeChangeRule dstStart = {"DST", 0, 0, 0, 0, 0};
TimeChangeRule dstEnd = {"STD", 0, 0, 0, 0, 0};
Timezone       timeZoneObj(dstEnd);

TaskHandle_t   slaveCoreTaskHandler;
TaskHandle_t   cathodePoiTaskHandler;

AsyncWebServer webServer(80);

Preferences prefsNVS;

uint8_t  masterCore, slaveCore;

volatile bool     cathodePoiOngoing = false;
volatile uint32_t currTime = 0;
volatile uint32_t lastNTPSync = 0;

char dataPartitionVersion[FFAT_IMAGE_VERSION_FILE_MAX_SIZE] = "";

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
  
  tubesSemaphore = xSemaphoreCreateMutex();
  
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
  
  
  /* ----------------------------------------------- */
  /* --- NVS storage and settings initialization --- */
  /* ----------------------------------------------- */
  Serial.println("\n> NVS initialization");
  prefsNVS.begin(NVS_NAMESPACE);
  nvsSemaphore = xSemaphoreCreateMutex();
  
  Serial.println("\n> Reading config files");
  if (!readBinaryFromNVS(&currConf, sizeof(GlobalConfig), NVS_KEY_CONFIG)) {
    Serial.println("Using default config");
    saveBinaryToNVS(&currConf, sizeof(GlobalConfig), NVS_KEY_CONFIG);
  }
  
  lastNTPSync = readIntFromNVS(NVS_KEY_LAST_NTP_SYNC);
  
  
  /* ----------------------------------- */
  /* --- FFAT storage initialization --- */
  /* ----------------------------------- */
  for(uint8_t i=0; i<=1; i++) {
    Serial.println("\n> Mounting FAT volume");  
    if (FFat.begin()) break;
    else {
      Serial.println("failed. Webserver not available. Formatting");
      FFat.format();
      // TODO: Print error code on tubes
    }
  }
  readFatImageV();
  
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
    String jsonStr = "{\"firmwareV\": \"";
    jsonStr += FIRMWARE_V;
    jsonStr += "\", \"dataV\": \"";
    jsonStr += dataPartitionVersion;
    jsonStr += "\"}";
    
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
    
    doc["cath_p_on"]   = currConf.catPoiOn;
    doc["cath_p_h"]    = currConf.catPoiAtH;
    doc["cath_p_min"]  = currConf.catPoiAtMin;
    doc["cath_p_dur"]  = currConf.catPoiDurMin;
    
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
    
    qItem = MASTER_QUEUE_EVENT::NTP_SYNC;
    xQueueSendToBack(masterQueue, &qItem, NULL);
    
    qItem = MASTER_QUEUE_EVENT::CHECK_FIRMWARE_UPD;
    xQueueSendToBack(masterQueue, &qItem, NULL);
    
    qItem = MASTER_QUEUE_EVENT::CHECK_DATA_PART_UPD;
    xQueueSendToBack(masterQueue, &qItem, NULL);
    
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
      newConf.catPoiOn     = doc["cath_p_on"];
      newConf.catPoiAtH    = doc["cath_p_h"];
      newConf.catPoiAtMin  = doc["cath_p_min"];
      newConf.catPoiDurMin = doc["cath_p_dur"];
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
        
        if (xSemaphoreTake(rtcSemaphore, SEM_TAKE_DELAY_TICKS)) {
          rtc.adjust(newTimeUTC);
          xSemaphoreGive(rtcSemaphore);
        }
        else {
          sendPlainTextResponse(request, 500, "RTC busy");
          return;
        }
        
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
      !saveBinaryToNVS(&newConf, sizeof(GlobalConfig), NVS_KEY_CONFIG)) {
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
  if (intensity > 255)        intensity = 255;
  else if (digit == -1)       intensity = 0;
  else if (cathodePoiOngoing) intensity = CATHODE_POI_INTENSITY;
  
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
  if (!xSemaphoreTake(tubesSemaphore, SEM_TAKE_DELAY_TICKS)) return;
  
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
  
  xSemaphoreGive(tubesSemaphore);
}

void displayNumberOnTubes(uint16_t number) {
  // Semaphore not taken here because is always taken by calling function
  int8_t h1, h2, m1, m2;

  h1 = (number/1000U) % 10;
  h2 = (number/100U) % 10;
  
  m1 = (number/10U) % 10;
  m2 = number % 10;
  
  if (number < 10) {
    h1 = -1;
    h2 = -1;
    m1 = -1;
  }
  else if (number <100) {
    h1 = -1;
    h2 = -1;
  }
  else if (number < 1000) {
    h1 = -1;
  }
  
  setTubeDigit(0, h1);
  setTubeDigit(1, h2);
  setTubeDigit(2, m1);
  setTubeDigit(3, m2);
  
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
  Serial.println("\n> Starting wifi");
  
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

bool checkOTAUpdate(OTA_UPDATE otaType) {
  String vJsonKey, urlJsonKey;
  
  SemanticVersion currV;
  SemanticVersion lastV;
  
  if (otaType == OTA_UPDATE::FIRMWARE) {
    vJsonKey = "firmware_v";
    urlJsonKey = "firmware_url";
    currV.parseV(FIRMWARE_V);
    Serial.println("\n> Checking firmware updates");
  }
  else {
    vJsonKey = "fat_image_v";
    urlJsonKey = "fat_image_url";
    currV.parseV(dataPartitionVersion);
    Serial.println("\n> Checking ffat image updates");
  }
  
  Serial.printf("Current version %s\n", currV.getCString());
  
  WiFiClientSecure client;
  
  client.setInsecure();
  
  HTTPClient https;
  https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  
  if (!https.begin(client, OTA_FIRMWARE_V_JSON_URL)) {
    Serial.println("Unable to connect");
    return false;
  }
  
  int httpCode = https.GET();
  if (httpCode < 0) {
    Serial.printf("GET failed: %s\n", https.errorToString(httpCode).c_str());
    return false;
  }
  
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("Failed: %u\n", httpCode);
    return false;
  }
  
  String responseString = https.getString();
  https.end();
  
  DynamicJsonDocument jDoc(512);
  DeserializationError desErr = deserializeJson(jDoc, responseString);
  
  if (desErr != DeserializationError::Ok) {
    Serial.printf("JSON DeserializationError: %s\n", desErr.c_str());
    return false;
  }
  
  lastV.parseV(jDoc[vJsonKey].as<const char*>());
  Serial.printf("Last available version %s\n", lastV.getCString());
  
  if (lastV > currV) {
    // semaphore take
    if (!xSemaphoreTake(rtcSemaphore, SEM_TAKE_DELAY_TICKS))
      return false;
    
    if (!xSemaphoreTake(nvsSemaphore, SEM_TAKE_DELAY_TICKS)) {
      xSemaphoreGive(rtcSemaphore);
      return false;
    }
    
    if (!xSemaphoreTake(tubesSemaphore, SEM_TAKE_DELAY_TICKS)) {
      xSemaphoreGive(rtcSemaphore);
      xSemaphoreGive(nvsSemaphore);
      return false;
    }
    
    httpUpdate.onStart(OTAStarted);
    httpUpdate.onEnd(OTAFinished);
    httpUpdate.onProgress(OTAOnProgress);
    httpUpdate.onError(OTAError);
    
    httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    httpUpdate.rebootOnUpdate(false);
    
    t_httpUpdate_return ret;
    
    if (otaType == OTA_UPDATE::FIRMWARE)
      ret = httpUpdate.update(client, jDoc[urlJsonKey]);
    else
      ret = httpUpdate.updateSpiffs(client, jDoc[urlJsonKey]);
    
    if (ret == HTTP_UPDATE_OK) {
      Serial.println("OTA OK. Restarting...");
      MASTER_QUEUE_EVENT qItemNew = MASTER_QUEUE_EVENT::REBOOT;
      xQueueSendToBack(masterQueue, &qItemNew, NULL);
      return true;
    }
    
    xSemaphoreGive(rtcSemaphore);
    xSemaphoreGive(nvsSemaphore);
    xSemaphoreGive(tubesSemaphore);
    
    if (ret == HTTP_UPDATE_FAILED) {
      Serial.printf("OTA Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      return false;
    }
    
    if (ret == HTTP_UPDATE_NO_UPDATES) {
      Serial.println("OTA no updates");
      return true;
    }
  }
  
  Serial.println("Nothing to update");
  return true;
}

void OTAStarted() {
  Serial.println("OTA started");
}

void OTAFinished() {
  Serial.println("OTA finished");
}

void OTAOnProgress(int cur, int total) {
  float percFloat = ((float)cur/(float)total)*100;
  uint8_t percInt = (uint8_t) percFloat;
  
  displayNumberOnTubes(percInt);
  Serial.printf("OTA update progress %d (%d of %d bytes)\n", percInt, cur, total);
}

void OTAError(int err) {
  Serial.printf("OTA fatal error (code %d)\n", err);
}

bool syncNTP() {
  Serial.println("\n> NTP sync");
  
  uint32_t secSince1900 = sendNTPpacket();
  
  if (!secSince1900) {
    Serial.println("Bad response. Skipping sync");
    return false;
  }
  
  uint32_t utcTimeInt = secSince1900 - SEVENTY_YEARS;
  DateTime utcTime(utcTimeInt);
  DateTime rtcTime;
  
  if (xSemaphoreTake(rtcSemaphore, SEM_TAKE_DELAY_TICKS)) {
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
  saveIntToNVS(lastNTPSync, NVS_KEY_LAST_NTP_SYNC);
  
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

bool readBinaryFromNVS(void* destPnt, const size_t size, const char* key) {
  Serial.printf("Reading %s...", key);
  
  byte tmpBuffer[size];
  
  size_t bytesRd = 0;
  
  if (xSemaphoreTake(nvsSemaphore, SEM_TAKE_DELAY_TICKS)) {
    bytesRd = prefsNVS.getBytes(key, tmpBuffer, size);
    xSemaphoreGive(nvsSemaphore);
  }
  else {
    Serial.println("NVS busy");
    return false;
  }
  
  if (bytesRd != size) {
    Serial.println("Wrong size");
    return false;
  }
  
  memcpy(destPnt, &tmpBuffer, size);
  
  Serial.println(STR_OK);
  return true;
}

bool saveBinaryToNVS(const void* sourcePnt, const size_t size, const char* key) {
  Serial.printf("Saving %s...", key);
  
  size_t bytesWr = 0;

  if (xSemaphoreTake(nvsSemaphore, SEM_TAKE_DELAY_TICKS)) {
    bytesWr = prefsNVS.putBytes(key, (const void*) sourcePnt, size);
    xSemaphoreGive(nvsSemaphore);
  }
  else {
    Serial.println("NVS busy");
    return false;
  }
  
  if (bytesWr != size) {
    Serial.println("Error writing file");
    return false;
  }
  
  Serial.println(STR_OK);
  return true;
}

uint32_t readIntFromNVS(const char* key) {
  Serial.printf("Reading %s...", key);
  
  uint32_t destInt = 0;
  
  if (xSemaphoreTake(nvsSemaphore, SEM_TAKE_DELAY_TICKS)) {
    destInt = prefsNVS.getUInt(key, 0);
    xSemaphoreGive(nvsSemaphore);
  }
  else {
    Serial.println("NVS busy");
    return false;
  }
  
  Serial.println(STR_OK);
  return destInt;
}

bool saveIntToNVS(const uint32_t sourceInt, const char* key) {
  Serial.printf("Saving %s...", key);
  
  size_t bytesWr = 0;
  
  if (xSemaphoreTake(nvsSemaphore, SEM_TAKE_DELAY_TICKS)) {
    bytesWr = prefsNVS.putUInt(key, sourceInt);
    xSemaphoreGive(nvsSemaphore);
  }
  else {
    Serial.println("NVS busy");
    return false;
  }
  
  if (bytesWr != sizeof(uint32_t)) {
    Serial.println("Error writing file");
    return false;
  }
  
  Serial.println(STR_OK);
  return true;
}

bool readFatImageV() {
  Serial.printf("Reading FAT image version from %s ... ", FFAT_IMAGE_VERSION_FILE);
  
  if (!FFat.exists(FFAT_IMAGE_VERSION_FILE)) {
    Serial.println("File not found");
    return false;
  }
  
  File vFile = FFat.open(FFAT_IMAGE_VERSION_FILE, FILE_READ);
  
  if (!vFile) {
    Serial.println("Error opening file");
    return false;
  }
  
  uint8_t vFileSize = vFile.size();
  if (vFileSize > FFAT_IMAGE_VERSION_FILE_MAX_SIZE) {
    Serial.println("File too large");
    return false;
  }
  
  byte tmpBuffer[vFileSize];
  size_t bytesRd = vFile.read((uint8_t*) &tmpBuffer, vFileSize);
  vFile.close();
  
  if (bytesRd != vFileSize) {
    Serial.println("Error reading file");
    return false;
  }
  
  memcpy(dataPartitionVersion, &tmpBuffer, vFileSize);
  
  Serial.println(dataPartitionVersion);
  return true;
}

void loop() {
  MASTER_QUEUE_EVENT qItem;
  
  if( xQueueReceive( masterQueue, &qItem, ( TickType_t ) 0) == pdPASS ) {
    
    static uint8_t wifiTimeCounter = 0;
    static bool    lastNTPSyncSuccess = true;
    
    static bool    lastFirmOTASuccess = true;
    static bool    lastDataPartOTASuccess = true;
    
    static uint8_t firmOTARetry = 0;
    static uint8_t dataPartOTARetry = 0;
    
    if (qItem == MASTER_QUEUE_EVENT::CONNECT_WIFI) {
      if (wifiConnect()) wifiTimeCounter = 0;
    }
    
    if (qItem == MASTER_QUEUE_EVENT::RTC_MINUTE_TOCK) {
      DateTime nowTimeLocal(timeZoneObj.toLocal(currTime));
      
      static uint8_t cathodePoiMinCounter = 0;
      
      if (!cathodePoiOngoing) {
        if (currConf.catPoiOn && nowTimeLocal.hour() == currConf.catPoiAtH && nowTimeLocal.minute() == currConf.catPoiAtMin) {
          // Start cathode poisoning prevention cycle
          cathodePoiOngoing = true;
          cathodePoiMinCounter = 0;
          
          xTaskCreatePinnedToCore(
          cathodePoisoningCycleTask, // Function to implement the task
          "cathodePoiT", // Name of the task
          1000,          // Stack size in words
          NULL,          // Task input parameter
          1,             // Priority of the task
          &cathodePoiTaskHandler, // Task handle
          slaveCore);    // Core
          
          Serial.println("\n> Strarting cathode poisoning routine");
        }
      }
      else {
        cathodePoiMinCounter++;
        if ((cathodePoiMinCounter >= currConf.catPoiDurMin) || !currConf.catPoiOn) {
          cathodePoiOngoing = false;
          Serial.println("\n> Stopping cathode poisoning routine");
        }
      }
      
      wifiTimeCounter++;
      if (wifiTimeCounter == WIFI_ON_TIME_MIN) wifiDisconnect();
      
      // If last NTP sync failed, retry evey minute
      if (((currTime >= lastNTPSync + currConf.ntpSyncIntH*3600) || !lastNTPSyncSuccess) && !currConf.netAPMode) {
        MASTER_QUEUE_EVENT qItemNew = MASTER_QUEUE_EVENT::NTP_SYNC;
        xQueueSendToBack(masterQueue, &qItemNew, NULL);
      }
      
      if (nowTimeLocal.hour() == OTA_CHECK_TIME_H && nowTimeLocal.minute() == OTA_CHECK_TIME_MIN && !currConf.netAPMode) {
        firmOTARetry = 0;
        dataPartOTARetry = 0;
        
        MASTER_QUEUE_EVENT qItemNew;
        
        qItemNew = MASTER_QUEUE_EVENT::CHECK_FIRMWARE_UPD;
        xQueueSendToBack(masterQueue, &qItemNew, NULL);
        
        qItemNew = MASTER_QUEUE_EVENT::CHECK_DATA_PART_UPD;
        xQueueSendToBack(masterQueue, &qItemNew, NULL);
      }
      
      // If last firmware update failed, retry every minute but max 5 times
      if (!lastFirmOTASuccess && firmOTARetry < 5) {
        MASTER_QUEUE_EVENT qItemNew = MASTER_QUEUE_EVENT::CHECK_FIRMWARE_UPD;
        xQueueSendToBack(masterQueue, &qItemNew, NULL);
        firmOTARetry++;
      }
      
      // If last data partition update failed, retry every minute but max 5 times
      if (!lastDataPartOTASuccess && dataPartOTARetry < 5) {
        MASTER_QUEUE_EVENT qItemNew = MASTER_QUEUE_EVENT::CHECK_DATA_PART_UPD;
        xQueueSendToBack(masterQueue, &qItemNew, NULL);
      }
    }
    
    if (qItem == MASTER_QUEUE_EVENT::CHECK_FIRMWARE_UPD && !currConf.netAPMode) {
      lastFirmOTASuccess = false;
      if (WiFi.status() == WL_CONNECTED) {
        lastFirmOTASuccess = checkOTAUpdate(OTA_UPDATE::FIRMWARE);
      }
      else {
        MASTER_QUEUE_EVENT qItemNew = MASTER_QUEUE_EVENT::CONNECT_WIFI;
        xQueueSendToFront(masterQueue, &qItemNew, NULL);
      }
    }
    
    if (qItem == MASTER_QUEUE_EVENT::CHECK_DATA_PART_UPD && !currConf.netAPMode) {
      lastDataPartOTASuccess = false;
      if (WiFi.status() == WL_CONNECTED) {
        lastDataPartOTASuccess = checkOTAUpdate(OTA_UPDATE::DATA_PARTITION);
      }
      else {
        MASTER_QUEUE_EVENT qItemNew = MASTER_QUEUE_EVENT::CONNECT_WIFI;
        xQueueSendToFront(masterQueue, &qItemNew, NULL);
      }
    }
    
    
    if (qItem == MASTER_QUEUE_EVENT::NTP_SYNC && !currConf.netAPMode && currConf.ntpSync) {
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
    
    if (qItem == MASTER_QUEUE_EVENT::REBOOT) {
      static uint32_t startTime = millis();
      if (millis() > startTime + REBOOT_DELAY_MS)
        ESP.restart();
      else
        xQueueSendToFront(masterQueue, &qItem, NULL);
      
    }
  }
}


/********************************************
*-*-*--*-*-*-*-*-*-*-*-*-*-*-*--*-*-*-*-*-*-*
* ---------- SLAVE CORE FUNCTIONS --------- *
*-*-*--*-*-*-*-*-*-*-*-*-*-*-*--*-*-*-*-*-*-*
********************************************/


void IRAM_ATTR RTCAlarmIsr() {
  SLAVE_QUEUE_EVENT qItemSlave = SLAVE_QUEUE_EVENT::RTC_ALARM_FIRED;
  xQueueSendToFrontFromISR(slaveQueue, &qItemSlave, NULL);
}

void coreSlaveTask(void* parameter) {
  rtc.clearAlarm(1);
  attachInterrupt(PIN_RTC_SQW, RTCAlarmIsr, FALLING);
  
  currTime = rtc.now().unixtime();
  displayTimeOnTubes(currTime);
  
  while (true) {
    SLAVE_QUEUE_EVENT qItem;
    
    if( xQueueReceive( slaveQueue, &qItem, ( TickType_t ) 0) == pdPASS ) {
      
      if (qItem == SLAVE_QUEUE_EVENT::UPDATE_TIME_ON_TUBES || qItem == SLAVE_QUEUE_EVENT::RTC_ALARM_FIRED) {
        Serial.println("\n> Updating time on tubes");
        
        if (xSemaphoreTake(rtcSemaphore, SEM_TAKE_DELAY_TICKS)) {
          if (rtc.alarmFired(1)) rtc.clearAlarm(1);
          currTime = rtc.now().unixtime();
          xSemaphoreGive(rtcSemaphore);
          
          // tubes semaphore taken inside the function
          displayTimeOnTubes(currTime);
        }
        else {
          static uint8_t tubeUpdateRetry = 0;
          
          if (tubeUpdateRetry < 5) {
            xQueueSendToFront(slaveQueue, &qItem, NULL);
            tubeUpdateRetry++;
          }
          else tubeUpdateRetry = 0;
        }
      }
      
      
      if (qItem == SLAVE_QUEUE_EVENT::RTC_ALARM_FIRED) {
        MASTER_QUEUE_EVENT qItemMaster = MASTER_QUEUE_EVENT::RTC_MINUTE_TOCK;
        xQueueSendToBack(masterQueue, &qItemMaster, NULL);
      }
    }
    
    vTaskDelay(2);
  }
}

void cathodePoisoningCycleTask(void* parameter) {
  while (!xSemaphoreTake(tubesSemaphore, SEM_TAKE_DELAY_TICKS)) vTaskDelay(2);
  
  setDots(false);
  uint8_t digit = 9;
  
  const TickType_t xPeriod = pdMS_TO_TICKS(CATHODE_POI_DIGIT_ON_TIME_MS);
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (cathodePoiOngoing) {
    setTubeDigit(0, digit);
    setTubeDigit(1, digit);
    setTubeDigit(2, digit);
    setTubeDigit(3, digit);
    
    if (digit > 0) digit--;
    else digit = 9;
    
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
  
  xSemaphoreGive(tubesSemaphore);
  // Required to end the task
  vTaskDelete(NULL);
}