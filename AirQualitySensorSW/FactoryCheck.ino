//___________ FACTORY CHECK___________
#include <Preferences.h>

// Define the namespace and key for the factory state
#define FACTORY_STATE_NAMESPACE "factory"
#define FACTORY_STATE_KEY "state"

// Global variable to store the factory state
bool isFactoryState = true;

Preferences preferences;

void initFactoryCheck() {
  // Initialize the Preferences library
  preferences.begin(FACTORY_STATE_NAMESPACE, false);
  // Check the factory state from Preferences
  isFactoryState = preferences.getBool(FACTORY_STATE_KEY, true);
  if (!isFactoryState) {
    return;
  }

  //CHECK ALL LEDS
  LEDS.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
  LEDS.setBrightness(50);

  //TURN ALL THE LIGHTS RED
  for (workingIndexLed = 0; workingIndexLed < NUM_LEDS; workingIndexLed++) {
    leds[workingIndexLed] = CRGB(0, 255, 0);
    FastLED.show();
    delay(40);
  }
  //TURN ALL THE LIGHTS GREEN
  for (workingIndexLed = 0; workingIndexLed < NUM_LEDS; workingIndexLed++) {
    leds[workingIndexLed] = CRGB(255, 0, 0);
    FastLED.show();
    delay(40);
  }
  //TURN ALL THE LIGHTS BLUE
  // First slide the led in one direction
  for (workingIndexLed = 0; workingIndexLed < NUM_LEDS; workingIndexLed++) {
    leds[workingIndexLed] = CRGB(0, 0, 255);
    FastLED.show();
    delay(40);
  }
  //TURN ALL THE LIGHTS OFF
  // First slide the led in one direction
  for (workingIndexLed = 0; workingIndexLed < NUM_LEDS; workingIndexLed++) {
    leds[workingIndexLed] = CRGB(0, 0, 0);
    FastLED.show();
    delay(40);
  }

  //START SERIAL
  enableSerial();
  Serial.println("****** STARTING FACTORY CHECK ******");

  //CHECK I2C devices
  leds[0] = CRGB(0, 255, 0);
  FastLED.show();

  debug_nDevices = 0;
  Wire.begin(DS1307_I2C_SCL, DS1307_I2C_SDA);
  while (debug_nDevices == 0) {
    for (debug_address = 1; debug_address < 127; debug_address++) {
      Wire.beginTransmission(debug_address);
      debug_error = Wire.endTransmission();
      if (debug_error == 0) {
        serialPrint("I2C device found at debug_address 0x");
        if (debug_address < 16) {
          serialPrint("0");
        }
        serialPrintln(debug_address, HEX);
        debug_nDevices++;

      } else if (debug_error == 4) {

        serialPrint("Unknow error at address 0x");
        if (debug_address < 16) {
          serialPrint("0");
        }
        serialPrintln(debug_address, HEX);
      }
    }
    if (debug_nDevices == 0) {
      serialPrintln("No I2C devices found\n");
      delay(1000);
    } else {
      serialPrintln("done\n");
    }
  }
  leds[0] = CRGB(255, 255, 255);
  FastLED.show();
  //CHECK DS1307
  leds[1] = CRGB(0, 255, 0);
  FastLED.show();

  bool rtcNotFount = true;
  while (rtcNotFount) {
    Wire.begin(DS1307_I2C_SCL, DS1307_I2C_SDA);
    Wire.setClock(100000);

    serialPrintln("starting rtc");
    // Initialize RTC
    if (!rtc.begin()) {

      serialPrintln(F("RTC not found"));
      delay(1000);
    } else {
      rtcNotFount = false;
    }
  }
  // Set square wave out pin
  rtc.setSquareWave(SquareWaveDisable);
  rtc.clockEnable(true);

  while (!rtc.isRunning()) {
    // Error: RTC oscillator stopped. Date/time cannot be trusted.
    // Set new date/time before reading date/time.
    // Enable oscillator
    rtc.clockEnable(true);
    serialPrintln("oscillator not working");
    delay(500);
  }

  if (!rtc.setDateTime(23, 34, 56, 13, 05, 2024, 0)) {
    Serial.println(F("Set date/time failed"));
  }

  working_epoch = getEpochNow();
  while (working_epoch < EPOCH_PAST_THRESHOLD) {
    serialPrint("epoch found:");
    serialPrintln(working_epoch);
    serialPrintln("Failed to read time");

    delay(1000);
  }
  getDateNow();

  leds[1] = CRGB(255, 255, 255);
  FastLED.show();

  //CHECK SD CARD
  leds[2] = CRGB(0, 255, 0);
  FastLED.show();
  spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  bool sdCardNotWorking = true;
  while (sdCardNotWorking) {
    if (!SD.begin(SD_CS, spi, 80000000)) {  // if errors, first try to reduce clock speed!!!!!
      serialPrintln("Card Mount Failed");
      delay(1000);
    } else {
      sdCardNotWorking = false;
    }
  }
  leds[2] = CRGB(255, 255, 255);
  FastLED.show();

  //CHECK LDR

  leds[3] = CRGB(0, 255, 0);
  FastLED.show();
  delay(1500);
  LEDS.setBrightness(5);

  pinMode(LDR1, INPUT);
  pinMode(LDR2, INPUT);

  int ldr1Value = analogRead(LDR1);
  int ldr2Value = analogRead(LDR2);

  FastLED.show();
  int prevValue = ldr1Value;
  int sumTmp = 0;
  bool aboveThreshold = false;
  Serial.print("ldr1Value:");
  while ( !( (abs(sumTmp)>10) && aboveThreshold) ) {
    ldr1Value = analogRead(LDR1);
    if(ldr1Value > 1023)
    {
      aboveThreshold = true;
    }
    
    sumTmp += ldr1Value - prevValue;
    prevValue = ldr1Value; 
    Serial.print(" ");
    Serial.print(ldr1Value);
    Serial.print(" ");
    
    delay(25);
  }
  Serial.println(" ");
  Serial.println("LDR1 passed");
  prevValue = ldr2Value;
  sumTmp = 0;
  aboveThreshold = false;
   Serial.println(" ");
  Serial.println("ldr2Value:");

  while ( !(abs(sumTmp)>10 && aboveThreshold) ) {
    ldr2Value = analogRead(LDR2);
    if(ldr2Value > 1023)
    {
      aboveThreshold = true;
    }
    sumTmp += ldr2Value - prevValue;
    prevValue = ldr2Value; 
    Serial.print(" ");
    Serial.print(ldr2Value);
    
    Serial.print(" ");

    delay(25);
  }
    Serial.println(" ");
    Serial.println("LDRs ok");

  leds[0] = CRGB(255, 255, 255);
  leds[1] = CRGB(255, 255, 255);
  leds[2] = CRGB(255, 255, 255);
  leds[3] = CRGB(255, 255, 255);
  FastLED.show();
  LEDS.setBrightness(50);
  //CHECK PMS7003
  leds[4] = CRGB(0, 255, 0);
  FastLED.show();

  pinMode(PMS7003_ENABLE, OUTPUT);
  digitalWrite(PMS7003_ENABLE, HIGH);
  pinMode(PMS7003_RST, OUTPUT);
  digitalWrite(PMS7003_RST, HIGH);
  SerialPMS.begin(PMS::BAUD_RATE, SERIAL_8N1, PMS7003_TX, PMS7003_RX);

  pms.wakeUp();

  bool pmsNotWorking = true;

  while (pmsNotWorking) {
    pms.requestRead();
    delay(10000);
    PMS::DATA data;
    if (pms.readUntil(data)) {
      pmsValues[0] = data.PM_AE_UG_1_0;
      serialPrint("PM 1.0 (ug/m3): ");
      serialPrintln((pmsValues[1]));

      pmsValues[1] = data.PM_AE_UG_2_5;
      serialPrint("PM 2.5 (ug/m3): ");
      serialPrintln((pmsValues[1]));

      pmsValues[2] = data.PM_AE_UG_10_0;
      serialPrint("PM 10.0 (ug/m3): ");
      serialPrintln((pmsValues[2]));
      pmsNotWorking = false;
    } else {

      serialPrintln("No data.");
      pmsValues[0] = -1;
      pmsValues[1] = -1;
      pmsValues[2] = -1;
    }
  }
  leds[4] = CRGB(255, 255, 255);
  FastLED.show();

  //CHECK TOUCH
  leds[5] = CRGB(0, 255, 0);
  FastLED.show();
  workingTouchValue = touchRead(TOUCHPIN);
  while (workingTouchValue < touchThreshold) {
    delay(25);
    workingTouchValue = touchRead(TOUCHPIN);
  }
  leds[5] = CRGB(255, 255, 255);
  FastLED.show();
  //SAVE PREFERENCES
  preferences.putBool(FACTORY_STATE_KEY, false);
  disableSerial();
  ESP.restart();
}
