//___________ SD CARD MANAGER___________ 
/* 1. mangage communication with SD CARD
   2. initialize values at startup (time related and password+SSID for WIFI CONNECTION)
   3. write on SD Card
*/

void initSDCard() {

  spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  ////CRITICAL
  if (!SD.begin(SD_CS, spi, 1000000)) {  // if errors, first try to reduce clock speed!!!!!
    serialPrintln("Card Mount Failed");
    return;
  }

  serialPrintln("SD ok");
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    serialPrintln("No SD card attached");
    return;
  }

  serialPrint("SD Card Type: ");
  if (cardType == CARD_MMC) {
    serialPrintln("MMC");
  } else if (cardType == CARD_SD) {
    serialPrintln("SDSC");
  } else if (cardType == CARD_SDHC) {
    serialPrintln("SDHC");
  } else {
    serialPrintln("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  serialPrintf("SD Card Size: %lluMB\n", cardSize);

  serialPrintf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  
  isSDCardOk = true;
}


/** OPEN WIFI CREDENTIALS -- 
 - reading the file time_config -- this file contains the correct time -- it is set just once and then the file is wiped out
 **/
void openWifiCredentials() {

  //Opening the SD card is a blocking operation, so it's better to do that not often and not when any animation is running
   if(!isSDCardOk)
  {
    initSDCard();
    if(!isSDCardOk)
    {
      serialPrintln("failed to mount SD CARD");
      return;
    }
  }
 
  // Open wifi_credentials.txt and read ssid and pass if it exists
  wifiCredentialsFile = SD.open("/config/wifi_credentials.txt", FILE_READ);
  if (wifiCredentialsFile) {

    while (wifiCredentialsFile.available()) {
      String line = wifiCredentialsFile.readStringUntil('\n');
      line.trim();  // remove trailing \r if file was saved with CRLF newlines (Windows)
     
      
      if (line.startsWith("ssid:")) {
        
        
        ssid = line.substring(5);
      
      } else if (line.startsWith("pass:")) {
        
        
        password = line.substring(line.indexOf("pass:") + 5);
       
      } else if (line.startsWith("localname:")) {
        localname = line.substring(line.indexOf("localname:") + 10);
      } else if (line.startsWith("timestamp:")) {
        
        String timeStampString = line.substring(line.indexOf("timestamp:") + 10);

        if (timeStampString.length() == 13) {
          intmax_t valueInt = stringToUInt64(timeStampString);
          working_epoch = (time_t)(valueInt / 1000);
        } else if (timeStampString.length() == 10) {
          intmax_t valueInt = stringToUInt64(timeStampString);
          working_epoch = (time_t)valueInt;
        }

       

        if (working_epoch == (time_t)-1) {
          // Handle the error case where the conversion failed
          serialPrintln("Error: Invalid POSIX/Unix timestamp format.");
        } else {
          serialPrintln("Setting EPOCH from SD CARD");
          setTime(working_epoch);
        }
      }
    }
    wifiCredentialsFile.close();
    wifiCredentialsFile = SD.open("/config/wifi_credentials.txt", FILE_WRITE);
  
    wifiCredentialsFile.print("ssid:");
    wifiCredentialsFile.println(ssid);
    wifiCredentialsFile.print("pass:");
    wifiCredentialsFile.println(password);
    wifiCredentialsFile.printf("localname:");
    wifiCredentialsFile.println(localname);
    wifiCredentialsFile.close();

    serialPrint("\nThe board will be avialable on local network at this address: \n  http://");
    serialPrint(localname);
    serialPrintln(".local\n");
  }
}

/** OPEN GENERAL CONFIG FILE
 - reading the file config.txt -- this file contains the custom settings -- 
 **/
void saveSettings() {
  serialPrintln("Saving Settings");
  configFile = SD.open(CONFIG_FILE_NAME, FILE_WRITE);
  if (!configFile) {
    serialPrintln("Failed to open the configuration file for writing.");
    return;
  }
  serialPrintln("opened SD CARD");
  char line[MAX_LINE_LENGTH];

  // Write the parameters to the file
  snprintf(line, MAX_LINE_LENGTH, "sleep_time=%lu\n", custom_sleep_time);
  configFile.print(line);

  snprintf(line, MAX_LINE_LENGTH, "brighness_factor=%.1f\n", custom_brightness_factor);
  configFile.print(line);


  snprintf(line, MAX_LINE_LENGTH, "time_on=%lu\n", custom_time_on);
  configFile.print(line);

  snprintf(line, MAX_LINE_LENGTH, "threshold_indicator=%d\n", custom_threshold_indicator);
  configFile.print(line);

  configFile.close();
  serialPrintln("Parameters saved to the configuration file.");
}

bool loadSettings() {
  configFile = SD.open(CONFIG_FILE_NAME);
  if (!configFile) {
    serialPrintln("Failed to open the configuration file for reading.");
    return false;
  }

  char line[MAX_LINE_LENGTH];

  while (configFile.available()) {
    configFile.readBytesUntil('\n', line, MAX_LINE_LENGTH);
    line[MAX_LINE_LENGTH - 1] = '\0';  // Ensure the line is null-terminated

    char* separatorIndex = strchr(line, '=');
    if (separatorIndex) {
      *separatorIndex = '\0';  // Separate the key and value
      separatorIndex++;

      if (strcmp(line, "sleep_time") == 0) {
        custom_sleep_time = strtoul(separatorIndex, nullptr, 10);
      } else if (strcmp(line, "brighness_factor") == 0) {
        custom_brightness_factor = strtof(separatorIndex, nullptr);
        custom_brightness_factor = max(0.1f, custom_brightness_factor);
      } else if (strcmp(line, "autobrightness") == 0) {
        custom_autobrightness = atoi(separatorIndex);
      } else if (strcmp(line, "time_on") == 0) {
        custom_time_on = strtoul(separatorIndex, nullptr, 10);
      } else if (strcmp(line, "threshold_indicator") == 0) {
        custom_threshold_indicator = atoi(separatorIndex);
      }
    }
  }

  configFile.close();
  return true;
}

// Retrieve a parameter value from the configuration file
int getParameter(String key) {
  configFile = SD.open(CONFIG_FILE_NAME, FILE_READ);
  if (!configFile) {
    serialPrintln("Failed to open the configuration file for reading.");
    return -1;
  }

  String line;
  while (configFile.available()) {
    line = configFile.readStringUntil('\n');
    int separatorIndex = line.indexOf('=');
    if (separatorIndex != -1 && line.substring(0, separatorIndex) == key) {
      configFile.close();
      return line.substring(separatorIndex + 1).toInt();
    }
  }

  configFile.close();
  return -1;  // Return -1 if the key is not found
}
