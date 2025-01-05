//___________ PMS7003___________ 
void initPMS7003() {
  pinMode(PMS7003_ENABLE, OUTPUT);
  digitalWrite(PMS7003_ENABLE, HIGH);
  pinMode(PMS7003_RST, OUTPUT);
  digitalWrite(PMS7003_RST, HIGH);
  SerialPMS.begin(PMS::BAUD_RATE, SERIAL_8N1, PMS7003_TX, PMS7003_RX);
  // Switch to passive mode.
  pms.passiveMode();
  changeStatePMSReading(WAIT_TO_WAKEUP);
}


void disableSensor() {
  pinMode(PMS7003_ENABLE, OUTPUT);
  digitalWrite(PMS7003_ENABLE, LOW);
}
void enableSensor() {
  pinMode(PMS7003_ENABLE, OUTPUT);
  digitalWrite(PMS7003_ENABLE, HIGH);
}

void updatePMSState() {
  switch (pms_logic_state) {

    // If in SLEEP state, sleep for a defined time
    case SLEEP:
      elapsedTime = millis() - startTime;      // Calculate the elapsed time
      if (elapsedTime >= custom_sleep_time) {  // If 30 seconds have passed
        changeStatePMSReading(WAIT_TO_WAKEUP);
      }
      break;

    // If in WAIT_TO_WAKEUP state, wait for 30 seconds before transitioning to READING
    case WAIT_TO_WAKEUP:
      elapsedTime = millis() - startTime;  // Calculate the elapsed time
      if (elapsedTime >= 30000) {          // If 30 seconds have passed
        changeStatePMSReading(READING);
      }
      break;

    // If in READING state, execute some code and transition back to SLEEP
    case READING:
      PMS::DATA data;

     
      if (pms.readUntil(data)) {

        pmsValues[0] = data.PM_AE_UG_1_0;
        serialPrint("PM 1.0 (ug/m3): ");
        serialPrintln((pmsValues[0]));

        pmsValues[1] = data.PM_AE_UG_2_5;
        serialPrint("PM 2.5 (ug/m3): ");
        serialPrintln((pmsValues[1]));

        pmsValues[2] = data.PM_AE_UG_10_0;
        serialPrint("PM 10.0 (ug/m3): ");
        serialPrintln((pmsValues[2]));
        writeData(String(pmsValues[0]) + "," + String(pmsValues[1]) + "," + String(pmsValues[2]));

        currentAQIvalue = AQIVal(data.PM_AE_UG_2_5);

        didReadForFirstTime = true;
        lastTimeSensedTimeStampt = getEpochNow();
        sendDataToClient();

        updateLightIndicatorHeight();

        pms_hardware_state = PMS_OK;
        SerialPMS.flush();
  

      } else {
        pms_hardware_state = PMS_ERROR;
        serialPrintln("No data.");
        pmsValues[0] = -1;
        pmsValues[1] = -1;
        pmsValues[2] = -1;
      }
      lastTimeSensedTimeStampt = getEpochNow();

      changeStatePMSReading(SLEEP);

      break;
  }
}
void changeStatePMSReading(PMSLogicState newState) {

  if (newState != pms_logic_state) {
    switch (newState) {
      case SLEEP:
        
        pms.sleep();
        startTime = millis();
        disableSensor();
        break;
      case WAIT_TO_WAKEUP:
        enableSensor();
        pms.wakeUp();
        
        startTime = millis();
        break;
      case READING:
        
        pms.requestRead();
        startTime = millis();

        break;
    }
    
    pms_logic_state = newState;
  }
}
