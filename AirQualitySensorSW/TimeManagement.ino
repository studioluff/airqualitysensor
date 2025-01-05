/*___________ TIME MANAGEMENt___________ 
 * 1. communication with RTC (DS1307)
 * 2. Read time (when starting up)
 * 3. Set time (if SD Card has new time, read new time and set it on DS1307)
 * 4. Get timestamps when saving on SD CARD
 * 
 */
/* -------------------------------------- DS1307 -----------------------------------------------*/
void initDS1307() {
  // Initialize I2C
  Wire.begin(DS1307_I2C_SCL, DS1307_I2C_SDA);
  Wire.setClock(100000);

  serialPrintln("starting rtc");
  // Initialize RTC
  if (!rtc.begin()) {

    serialPrintln(F("RTC not found"));
    checkI2CDevices();

    return;
  }

  // Set square wave out pin
  rtc.setSquareWave(SquareWaveDisable);
  if (!rtc.setDateTime(23, 34, 56, 13, 05, 2024, 0)) {
    Serial.println(F("Set date/time failed"));
  }

  working_epoch = getEpochNow();
  if (working_epoch < EPOCH_PAST_THRESHOLD) {
    serialPrintln("Failed to read time");
  }
  getDateNow();

  if (!rtc.isRunning()) {
    // Enable oscillator
    rtc.clockEnable(true);
  }

  ds1307_started = true;
}

void updateDS1307() {
  if (!ds1307_started) {
    if (millis() - lastTimeCheckedDs1307ToStart > 1000) {
      initDS1307();
      lastTimeCheckedDs1307ToStart = millis();
    }
  }
}

time_t getEpochNow() {
  return rtc.getEpoch();
}
String getDateNow() {

  // Read date/time
  if (!rtc.getDateTime(&hour, &minute, &sec, &mday, &mon, &year, &wday)) {
    serialPrintln(F("Read date/time failed"));
    return "";
  }
  String timestamp = String(year) + "," + String(mon, DEC) + "," + String(mday, DEC) + "," + String(hour, DEC) + "," + String(minute, DEC) + "," + String(sec, DEC);

  return timestamp;
}

bool setTime(time_t newTime) {
  if (!rtc.setEpoch(newTime)) {
    // Error: Set epoch failed
    serialPrintln("ERROR: Failed to set epoch on RTC");
    return false;
  }
  serialPrintln("RTC set correctly. (time_t) ");
  serialPrintln(getDateNow());
  return true;
}
void setTime(unsigned long timestamp) {
  // Convert timestamp to time_t
  time_t t = timestamp;
  serialPrintln((t));
  // Set the RTC using time_t
  struct timeval tv = { t, 0 };
  settimeofday(&tv, NULL);

  // Check if the RTC was set correctly
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    serialPrintln("Error setting RTC.");
  } else {
    serialPrintln("RTC set correctly. (unsigned long)");
    serialPrintln(getDateNow());
  }
}
boolean isLeapYear(int year) {
  if (year % 4 != 0) {
    return false;
  } else if (year % 100 != 0) {
    return true;
  } else if (year % 400 != 0) {
    return false;
  } else {
    return true;
  }
}
long stringToTimeT(String timestampStr) {
  // Remove the leading "s" or "ms" from the POSIX/Unix timestamp string if present
  if (timestampStr.length() > 1 && (timestampStr[0] == 's' || timestampStr[0] == 'm')) {
    timestampStr.remove(0, 1);
  }
  // Convert the POSIX/Unix timestamp string to a long integer representing the number of milliseconds since the Arduino boot
  long timestamp = timestampStr.toInt();

  // Convert the number of milliseconds to a time_t value
  time_t result = (time_t)timestamp / 1000;

  return result;
}
/* -------------------------------------- NTP CLIENT -----------------------------------------------*/
void initNTPClient() {
  if ((WiFi.status() == WL_CONNECTED)) {
    if (custom_does_use_NTP_client) {
      timeClient.begin();
      timeClient.forceUpdate();
      if (!timeClient.isTimeSet()) {
        return;
      }
      working_epoch = timeClient.getEpochTime();
      serialPrint("Current time from NTP Server");
      serialPrintln(working_epoch);
      if (working_epoch <= EPOCH_PAST_THRESHOLD) {
        serialPrintln("Error getting current time from NTP Server");
        return;
      }
      if (!rtc.setEpoch(working_epoch)) {
        serialPrintln(F("Set time failed from NTP"));
        return;
      }
      serialPrintln("Successfully got epoch");
    }
  }
}


void updateNTPClient() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  if (!wasPrevConnectedToInternet) {
    return;
  }

  if (custom_does_use_NTP_client) {

    //do this check every half a day (12 hours)
    if (millis() - lastTimeCheckedTime > ntpUpdatePeriodInMilliseconds) {  //43200000) {
      serialPrintln("-- udpateding timeClient from updateNTPCLIENT -- ");
      bool result = timeClient.forceUpdate();
      lastTimeCheckedTime = millis();
      if (result) {
        serialPrintln(" received EPOCH INSIDE updateNPTClient");
        //SET RTC Clock
        if (!rtc.setEpoch(timeClient.getEpochTime())) {
          serialPrintln(F("Set time failed from NTP"));
          return;
        }
        serialPrintln("successfully got epoch");
      }
    }
  }
}
