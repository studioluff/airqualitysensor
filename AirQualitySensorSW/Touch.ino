//___________ TOUCH___________ 
// Add these variables at the beginning of your sketch or in the appropriate scope
const unsigned long LONG_PRESS_DURATION = 5000; // 1 second for long press
unsigned long touchStartTime = 0;
bool longPressDetected = false;

void updateTouch() {
    // Use unsigned long for proper overflow arithmetic
    unsigned long currentTime = millis();
    
    // Safe time difference calculation
    if ((currentTime - touchSensorLastUpdate) >= 10) {
        workingTouchValue = touchRead(TOUCHPIN);
        
        if (workingTouchValue > touchThreshold && prevTouched == false) {
            prevTouched = true;
            touchStartTime = currentTime;  // Store current time
            longPressDetected = false;
            serialPrintln("touch start!");
           
        }
        else if (workingTouchValue > touchThreshold && prevTouched == true) {
           
            // Safe long press detection
            unsigned long pressDuration = currentTime - touchStartTime;
            if (!longPressDetected && pressDuration >= LONG_PRESS_DURATION) {
                longPressDetected = true;
                handleLongPress();
            }
        }
        else if (workingTouchValue < touchThreshold && prevTouched == true) {
            serialPrintln("touch end!");
            prevTouched = false;
            if (longPressDetected) {
                handleLongPressRelease();
            } else {
                handleShortPressRelease();
            }
        }
        
        touchSensorLastUpdate = currentTime;
    }
}

void handleLongPress() {
  playDiagnosticsAnimation();
}

void handleShortPressRelease() {

      
      if(pms_hardware_state == PMS_OK)
      {
       
        playUserPressAnimation();
      }
      else if(pms_hardware_state == PMS_ERROR)
      {
       
        playNoSensorAnimation();
      }
      else
      {
        //(pms_hardware_state == PMS_NOT_KNOWN)
      
        playNotReadYetAnimation();
      }
}

void handleLongPressRelease() {


}
