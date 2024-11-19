//___________ OTHER___________ 
void initSerial() {
  if (doesPrintSerialMessages) {
    enableSerial();
  }
}

void initLDR() {
  pinMode(LDR1, INPUT);
  pinMode(LDR2, INPUT);
}

int readLDR() {

  ldr1Value = analogRead(LDR1);
  ldr2Value = analogRead(LDR2);

  return abs(ldr1Value + ldr2Value);
}
void checkI2CDevices() {

  serialPrintln("Scanning for I2C devices (DS1307)...");
  debug_nDevices = 0;
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
  } else {
    serialPrintln("done\n");
  }
}
uint64_t stringToUInt64(const String& str) {
  uint64_t result = 0;
  for (uint8_t i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (isDigit(c)) {
      result = result * 10 + (c - '0');
    } else {
      return 0;  // Invalid character, return 0
    }
  }
  return result;
}

