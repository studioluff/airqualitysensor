//___________ SAVE DATA___________ 


void writeData(String data) {

  // Read date/time
  if (!rtc.getDateTime(&hour, &minute, &sec, &mday, &mon, &year, &wday)) {
    serialPrintln(F("Read date/time failed"));
    return;
  }
   serialPrintln("---- Beginning writing data ---");
  serialPrint("Saving values:");
  serialPrintln(data);

  String yearDir = "/data/" + String(year);
  String monthDir = yearDir + "/" + String(mon, DEC);
  String filename = monthDir + "/" + String(mday, DEC) + ".csv";

  if ((!SD.exists("/data") && !SD.mkdir("/data")) || (!SD.exists(yearDir) && !SD.mkdir(yearDir)) || (!SD.exists(monthDir) && !SD.mkdir(monthDir))) {
    serialPrintln(F("Failed to create directory"));
    return;
  }

  working_epoch = rtc.getEpoch();
  // Read Unix epoch UTC from RTC
  if (working_epoch <= EPOCH_PAST_THRESHOLD ){
    // Error: RTC read failed
    serialPrint(working_epoch);
    serialPrintln(" - UNABLE TO GET UPDATED EPOCH");
    return;
  }
  serialPrint("opening file:");
  serialPrintln(filename);
  File dataFile = SD.open(filename, FILE_APPEND);
  if (!dataFile) {
    serialPrintln(F("Failed to create file"));
    return;
  }
  dataFile.print(data + ",");  // Replace "data" with the actual value to write to file
  dataFile.print(String(ldr1Value) + ",");
  dataFile.print(String(ldr2Value) + ",");
  dataFile.println(String(working_epoch)+ ",");
  dataFile.close();

  serialPrintln(working_epoch);
  serialPrintln("---- end writing data ---");
}
