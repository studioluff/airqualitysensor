/*

  ___  _        _____             _ _ _           _____                           
 / _ \(_)      |  _  |           | (_) |         /  ___|                          
/ /_\ \_ _ __  | | | |_   _  __ _| |_| |_ _   _  \ `--.  ___ _ __  ___  ___  _ __ 
|  _  | | '__| | | | | | | |/ _` | | | __| | | |  `--. \/ _ \ '_ \/ __|/ _ \| '__|
| | | | | |    \ \/' / |_| | (_| | | | |_| |_| | /\__/ /  __/ | | \__ \ (_) | |   
\_| |_/_|_|     \_/\_\\__,_|\__,_|_|_|\__|\__, | \____/ \___|_| |_|___/\___/|_|   
                                           __/ |                                  
                                          |___/                                   
 _                 _             _ _         _     _   _____________              
| |               | |           | (_)       | |   | | | |  ___|  ___|             
| |__  _   _   ___| |_ _   _  __| |_  ___   | |   | | | | |_  | |_                
| '_ \| | | | / __| __| | | |/ _` | |/ _ \  | |   | | | |  _| |  _|               
| |_) | |_| | \__ \ |_| |_| | (_| | | (_) | | |___| |_| | |   | |                 
|_.__/ \__, | |___/\__|\__,_|\__,_|_|\___/  \_____/\___/\_|   \_|                 
        __/ |                                                                     
       |___/   

Copyright (C) 2024 Pierluigi Dalla Rosa - studio LUFF

Version 1.0.1

“Commons Clause” License Condition v1.0

The Software is provided to you by the Licensor under the License, as defined below, subject to the following condition.
Without limiting other conditions in the License, the grant of rights under the License will not include, and the License does not grant to you, the right to Sell the Software.
For purposes of the foregoing, “Sell” means practicing any or all of the rights granted to you under the License to provide to third parties, for a fee or other consideration (including without limitation fees for hosting or consulting/ support services related to the Software), a product or service whose value derives, entirely or substantially, from the functionality of the Software. Any license notice or attribution required by the License must also include this Commons Clause License Condition notice.
Software: [Air Quality Sensor by studio LUFF]
License: [GNU Affero General Public License]
Licensor: [studio LUFF]



Article about the reliability of PMS7003: 
https://www.nature.com/articles/s41598-019-43716-3/
*/

/*
Libraries to install:
PMS Library@1.1.0
FastLED@3.6.0
ESPAsyncWebSrv@1.2.7
ArduinoJson@6.21.4
Tween.h@0.4.2 by hideakitai
NTPClient@3.2.1
*/


/** ---------------------------------- PMS7003 ---------------------------------- **/
#include <PMS.h>

#define PMS7003_RX 17
#define PMS7003_TX 18
#define PMS7003_ENABLE 9
#define PMS7003_RST 8

HardwareSerial SerialPMS(1);
PMS pms(SerialPMS);
// PMS::DATA data;
int currentAQIvalue = -1;
enum PMSHwState {
  PMS_NOT_KNOWN,
  PMS_OK,
  PMS_ERROR
};

PMSHwState pms_hardware_state = PMS_NOT_KNOWN;

/** ---------------------------------- LOGIC AND TIMING - PMS ---------------------------------- **/
time_t lastTimeSensedTimeStampt = 0;
enum PMSLogicState { SLEEP,
                     WAIT_TO_WAKEUP,
                     READING };
unsigned long startTime = 0;
unsigned long elapsedTime = 0;
PMSLogicState pms_logic_state = SLEEP;
//1.0 , 2.5, 10.0
int pmsValues[] = { 0, 0, 0 };
bool didReadForFirstTime = false;


/** ----------------- LEDS ----------------- **/
#include <FastLED.h>

#define NUM_LEDS 23
#define DATA_PIN 48

CRGB leds[NUM_LEDS];

//THESE ARE THE VALUES OF THE SINGLE LEDS
int scaleValues[] = { 2, 4, 6, 8, 10, 15, 20, 25, 30, 35, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 160, 180, 200 };


//THIS IS THE INDEX OF THE LEDS
int indexLeds[] = { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 22, 21, 20, 19, 18, 17, 16 };

bool isShowing = false;


/** ---------------------------------- SD CARD ---------------------------------- **/
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define SD_SCK 36
#define SD_MISO 37
#define SD_MOSI 35
#define SD_CS 38

#define VSPI FSPI

SPIClass spi = SPIClass(VSPI);

bool isSDCardOk = false;
unsigned long lastTimeCheckedForSdCard = 0;
unsigned long timeoutCheckForSdCard = 100;

/** ---------------------------------- SETTINGS ---------------------------------- **/
File wifiCredentialsFile;
File configFile;

#define CONFIG_FILE_NAME "/config/config.txt"
bool SERIAL_ENABLED = 1;

/** ---------------------------------- SAVE DATA ---------------------------------- **/
File dataFile;  // The current data file


/** ---------------------------------- DS1307 Real Time Clock ---------------------------------- **/
#include <Wire.h>
#include <ErriezDS1307.h>
ErriezDS1307 rtc;

#define DS1307_I2C_SDA 5
#define DS1307_I2C_SCL 4
//USE TO CHECK I2C
byte debug_error, debug_address;
int debug_nDevices;

//TIME Variables
uint8_t hour;
uint8_t minute;
uint8_t sec;
uint8_t mday;
uint8_t mon;
uint16_t year;
uint8_t wday;
time_t working_epoch;
const byte daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
const byte daysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

bool ds1307_started = false;
unsigned long lastTimeCheckedDs1307ToStart = 0;

/** ---------------------------------- TOUCH BUTTON ---------------------------------- **/
#define TOUCHPIN 14
int touchThreshold = 50000;
int workingTouchValue = 0;

bool prevTouched = false;
unsigned long touchSensorLastUpdate = 0;


/** ---------------------------------- LDR ---------------------------------- **/
#define LDR_UPPER_THRESHOLD 30.0
int ldr1Value = 0;
int ldr2Value = 0;
float workingBrightness = 0.0f;
int workingLdrRaw;
float workingLdrMapped;


#define LDR1 6
#define LDR2 7

/** ---------------------------------- WIFI ---------------------------------- **/
#include <WiFi.h>
#include <ESPping.h>

bool wasPrevConnectedToLocalNet = false;
bool wasPrevConnectedToInternet = false;
unsigned long lastTimeCheckedForWifiConnection = 0;
unsigned long lastTimeCheckedForInternetConnection = 0;

String ssid = "";
String password = "";
String localname = "";
String hostname = "Air Quality Sensor";
int reconnecting_wifi_interval = 5000;


/** ---------------------------------- WEBSERVER + WEBSOCKET  ---------------------------------- **/
#include <AsyncEventSource.h>
#include <AsyncJson.h>
#include <AsyncWebSocket.h>
#include <AsyncWebSynchronization.h>
#include <ESPAsyncWebSrv.h>
#include <SPIFFSEditor.h>
#include <StringArray.h>
#include <WebAuthentication.h>
#include <WebHandlerImpl.h>
#include <WebResponseImpl.h>
#include <ESPmDNS.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");


/** ---------------------------------- SEND DATA TO CLIENT ---------------------------------- **/
unsigned long lastTimeSentToClient = 0;
int indexSendHistoricalDataToClient = -1;
#define NUM_OF_DAYS_HISTORICAL 13
bool sendHistoricalToClients = false;
int historicalDayToSend = 0;
int historicalMonthToSend = 0;
int historicalYearToSend = 0;

String path = "";
File workingFile;
#define MAX_LINE_LENGTH 128

/** ---------------------------------- CUSTOMIZABLE VARIABLES ---------------------------------- **/
//custom variables can be read from the sd card in the config.js file
unsigned long custom_sleep_time = 360000;  //360000;  //in ms, is the time the device sleeps before waking up, should be 5minutes by default 300000
float custom_brightness_factor = 1.0;      //100 = 100%, 50 = 50% etc..
bool custom_autobrightness = 1;            //1 means ignore LDR and 0 means use LDR to set the brightness
bool custom_does_use_NTP_client = 1;       //true by default --> use a NTPClient to set the date, otherwise if false read date from SD Card
int custom_time_on = 3000;                 //default 3000ms
int custom_threshold_indicator = 50;       //default 50

/** ---------------------------------- NTP SERVER ---------------------------------- **/
//NTP Server is going to be used to set automatically the date
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>
WiFiUDP ntpUDP;
const time_t EPOCH_PAST_THRESHOLD = 1715640636;
// Define NTP server and time zone
const int ntpUpdatePeriodInMilliseconds = 43200000;  //every 12 hours
const char* ntpServerName = "pool.ntp.org";
const long UTC_OFFSET_IN_SECONDS = 0;
NTPClient timeClient(ntpUDP, ntpServerName, UTC_OFFSET_IN_SECONDS, ntpUpdatePeriodInMilliseconds);
unsigned long lastTimeCheckedTime = 0;


/** ---------------------------------- ANIMATION ---------------------------------- **/
#include <Tween.h>
Tween::Timeline timeline;
float animIntensityTarget = 0.f;
float animLedIndexTarget = 0.f;
int workingAnimValue = 0;
int workingIndexLed = 0;
enum AnimState {
  ANIM_NONE,
  ANIM_ABOVE_THRESHOLD,
  ANIM_USER_PRESS,
  ANIM_RAMP_DOWN_TO_USER,
  ANIM_RAMP_DOWN_TO_NONE,
  ANIM_NOT_READ_YET,
  ANIM_NO_SENSOR,
  ANIM_DIAGNOSTICS
};


AnimState animState = ANIM_NONE;

unsigned long didDismissAboveThresholdAnimationEvent = 0;
bool didDismissAboveThreshold = false;

//ANIMATION ERROR LEDS
#define NUM_LEDS_NO_SENSOR_ANIM 8
int errorNoSensorLeds[] = { 2, 3, 4, 5, 20, 19, 18, 17, 16 };

/** ---------------------------------- STATUS WIFI/SDCARD ---------------------------------- **/
#define CHECK_PIN 0
bool isCheckPinOn = false;
unsigned long lastTimeReadCheckPin = 0;

void setup() {
  initFactoryCheck();

  initSerial();

  initSDCard();

  initLDR();

  initWS2812();

  initPMS7003();

  initDS1307();

  initWiFi();

  loadSettings();
}

void loop() {

  updatePMSState();

  updateTouch();

  updateWifi();

  updateServer();

  updateLEDS();

  updateDS1307();

  updateNTPClient();


}
