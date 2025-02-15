//___________ SERIAL___________
bool doesPrintSerialMessages = true;  // Flag to control serial printing

// Helper function to enable serial printing
void enableSerial() {
  doesPrintSerialMessages = true;
  Serial.begin(115200);
  //Serial.setTxTimeoutMs(0); // TODO: Fix-up

  Serial.print("__________________________\n");
  Serial.print("|                        |\n");
  Serial.print("|   Air Quality Sensor   | \n");
  Serial.print("|   by studio LUFF       | \n");
  Serial.print("|   version 1.0.1        | \n");
  Serial.print("|________________________| \n");
}

// Helper function to disable serial printing
void disableSerial() {
  doesPrintSerialMessages = false;
  Serial.end();
}

// Wrapper functions for Serial.print and Serial.println
template<typename T>
void serialPrint(T value) {
  if (doesPrintSerialMessages) {
    Serial.print(value);
  }
}

template<typename T, typename... Args>
void serialPrint(T value, Args... args) {
  if (doesPrintSerialMessages) {
    Serial.print(value);
    serialPrint(args...);
  }
}

template<typename T>
void serialPrintln(T value) {
  if (doesPrintSerialMessages) {
    Serial.println(value);
  }
}

template<typename T, typename... Args>
void serialPrintln(T value, Args... args) {
  if (doesPrintSerialMessages) {
    Serial.print(value);
    serialPrintln(args...);
  }
}
// Wrapper function for Serial.printf
void serialPrintf(const char* format, ...) {
  if (doesPrintSerialMessages) {
    va_list args;
    va_start(args, format);
    Serial.printf(format, args);
    va_end(args);
  }
}