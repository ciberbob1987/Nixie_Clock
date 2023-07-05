#ifndef _StaticConfig_h
#define _StaticConfig_h

const char     FIRMWARE_V[]         = "1.0";
const uint16_t REBOOT_DELAY_MS      = 2000; // delay ms before ESP restart

const uint8_t  CONF_STR_LEN         = 60;
const uint16_t UPD_JSON_MAX_SIZE    = 1024;

const char     DEFAULT_AP_SSID[]    = "NixieClock";
const char     DEFAULT_AP_KEY[]     = "";   // Length must be 0 or >8
const char     HIDDEN_WIFI_KEY[]    = "**********";

const char     NVS_NAMESPACE[]         = "nixieclock";
const char     NVS_KEY_CONFIG[]        = "conf.data";
const char     NVS_KEY_LAST_NTP_SYNC[] = "ntpsync.data";

const char     HOST_NAME[]          = "NixieClock";

const char     STR_OK[]             = "OK";

const uint32_t SEM_TAKE_DELAY_MS    = 30;

const uint8_t  WIFI_ON_TIME_MIN     = 10;

const uint8_t  NTP_PACKET_SIZE      = 48;
const uint8_t  NTP_PORT             = 123;
const uint16_t NTP_N_CYCLES         = 30;
const uint16_t NTP_CYCLES_DELAY     = 10;
const uint32_t SEVENTY_YEARS        = 2208988800UL;

const uint8_t  MASTER_QUEUE_SIZE    = 10;
const uint8_t  SLAVE_QUEUE_SIZE     = 10;

const uint8_t  PIN_BUFF_EN  = 32;
const uint8_t  PIN_RTC_SQW  = 35;
const uint8_t  PIN_TOUCH_1  = 33;
const uint8_t  PIN_TOUCH_2  = 27;
const uint8_t  PIN_TOUCH_3  = 13;

const uint8_t  PIN_DAC_CS[] = {26, 2, 4, 15};
const uint8_t  DAC_NUM      = 4;
const uint8_t  DAC_CHANNELS = 12;

const uint8_t  PIN_SPI_SCK  = 18;
const uint8_t  PIN_SPI_MISO = 34;
const uint8_t  PIN_SPI_MOSI = 23;
const uint8_t  PIN_SPI_SS   = 12;
const uint32_t SPI_FREQ     = 2000000;

const uint8_t  PIN_LED_R = 16;
const uint8_t  PIN_LED_G = 17;
const uint8_t  PIN_LED_B = 25;

const uint8_t  PWM_CH_LED_R = 0;
const uint8_t  PWM_CH_LED_G = 1;
const uint8_t  PWM_CH_LED_B = 2;
const uint16_t LED_PWM_FREQ = 1000;

const uint8_t DAC_CH_ADDR[] = {
  0b00001000,
  0b00000100,
  0b00001100,
  0b00000010,
  0b00001010,
  0b00000110,
  0b00001110,
  0b00000001,
  0b00001001,
  0b00000101,
  0b00001101,
  0b00000011
};

// main queue events
enum class MASTER_QUEUE_EVENT : uint8_t {
  CONNECT_WIFI,
  NTP_SYNC,
  RTC_MINUTE_TOCK,
  REBOOT
};

// slave queue events
enum class SLAVE_QUEUE_EVENT : uint8_t {
  UPDATE_TIME_ON_TUBES,
  RTC_ALARM_FIRED
};

enum class COMMAND_CODE : uint16_t {
  RGB_LEDS_CHANGE = 101, // real time
  TUBES_BRIG      = 201, // real time
  CATH_POI        = 202, // real time
  WIFI_UPD        = 301, // reboot required
  TIME_UPD        = 401, // real time
  DST_UPD         = 501, // real time
  NTP_SYNC        = 601, // real time
  TIME_SYNC       = 602  // real time
};

struct DSTInfo {
  uint8_t week;
  uint8_t weekDay;
  uint8_t month;
  uint8_t h;
};

struct GlobalConfig {
  char    netSSID[CONF_STR_LEN] = "WifiNetworkName";
  char    netKey[CONF_STR_LEN]  = "WifiNetworkPass";
  bool    netAPMode             = false;
  
  char    ntpAddr[CONF_STR_LEN] = "pool.ntp.org";
  bool    ntpSync     = true;
  uint8_t ntpSyncIntH = 1;
  
  bool    timeFormat12   = false;
  bool    timeZoneEnable = true;
  uint8_t timeZoneId     = 48;
  int16_t timeZoneOffset = 60;
  
  bool    dstEnable = true;
  uint8_t dstOffset = 60;
  DSTInfo dstStart  = {0, 1, 3, 2};
  DSTInfo dstEnd    = {0, 1, 10, 3};
  
  uint8_t nixieBri[4] = {70, 70, 70, 70};
  uint8_t dotLampBri  = 70;
  bool    catPoiOn    = true;
  
  bool     rgbLedOn    = true;
  uint8_t  rgbLedBri   = 50;
  uint32_t rgbLedColor = 0x5A32A8;
};


#endif