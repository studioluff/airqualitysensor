//___________ LIGHT OUTPUT___________
// Starting colors
int startingColors[23][3] = {
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 },
  { 255, 255, 150 }
};

// Ending colors
int endingColors[23][3] = {
  { 26, 229, 53 },
  { 26, 229, 53 },
  { 26, 225, 53 },
  { 26, 225, 53 },
  { 26, 225, 53 },
  { 26, 225, 53 },
  { 26, 225, 53 },
  { 103, 213, 39 },
  { 141, 205, 31 },
  { 180, 200, 25 },
  { 248, 112, 0 },
  { 248, 112, 0 },
  { 248, 112, 0 },
  { 248, 112, 0 },
  { 248, 112, 0 },
  { 243, 95, 0 },
  { 243, 50, 0 },
  { 243, 50, 0 },
  { 243, 20, 0 },
  { 243, 0, 0 },
  { 255, 10, 4 },
  { 255, 15, 10 },
  { 255, 22, 90 },

};


void initWS2812() {
  // Initialize LEDs
  LEDS.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
  LEDS.setBrightness(255);
  initLEDAnimation();
}

void initLEDAnimation() {
  //CHECK LDR - if very dark damp the brightness
  workingLdrRaw = readLDR();
  workingLdrMapped = map(readLDR(), LDR_UPPER_THRESHOLD, 0.0, 1.0, 0.1);
  workingLdrMapped = fConstrain(workingLdrMapped, 0.1, 1.0);
  workingBrightness = fMin(custom_brightness_factor, workingLdrMapped);

  LEDS.setBrightness(100 * workingBrightness);

  // Black to white transition
  unsigned long startTime = millis();
  while (millis() - startTime < 500) {
    float progress = (millis() - startTime) / 500.0;
    for (workingIndexLed = 0; workingIndexLed < NUM_LEDS; workingIndexLed++) {
      int r = 0 * (1 - progress) + 255 * progress;
      int g = 0 * (1 - progress) + 255 * progress;
      int b = 0 * (1 - progress) + 150 * progress;
      leds[indexLeds[workingIndexLed]] = CRGB(g, r, b);
    }
    FastLED.show();
  }

  // Color transition
  startTime = millis();
  while (millis() - startTime < 1000) {
    float progress = (millis() - startTime) / 1000.0;
    for (workingIndexLed = 0; workingIndexLed < NUM_LEDS; workingIndexLed++) {
      int r = 255 * (1 - progress) + endingColors[workingIndexLed][0] * progress;
      int g = 255 * (1 - progress) + endingColors[workingIndexLed][1] * progress;
      int b = 150 * (1 - progress) + endingColors[workingIndexLed][2] * progress;
      leds[indexLeds[workingIndexLed]] = CRGB(g, r, b);
    }
    FastLED.show();
  }
  delay(1500);

  // Fade out
  startTime = millis();
  while (millis() - startTime < 1000) {
    float progress = (millis() - startTime) / 1000.0;
    for (workingIndexLed = 0; workingIndexLed < NUM_LEDS; workingIndexLed++) {
      int r = endingColors[workingIndexLed][0] * (1 - progress);
      int g = endingColors[workingIndexLed][1] * (1 - progress);
      int b = endingColors[workingIndexLed][2] * (1 - progress);
      leds[indexLeds[workingIndexLed]] = CRGB(g, r, b);
    }
    FastLED.show();
  }
  LEDS.setBrightness(255);
}



void updateLEDS() {
  timeline.update();

  if (currentAQIvalue > custom_threshold_indicator) {
    //show leds
    // showStaticLED(currentAQIvalue);
    if (didDismissAboveThreshold && millis() - didDismissAboveThresholdAnimationEvent > 3600000)  //60 minutes
    {
      didDismissAboveThreshold = false;
    }

    if (!isShowing && !didDismissAboveThreshold) {
      changeAnimState(ANIM_ABOVE_THRESHOLD);
    }


  } else if (currentAQIvalue < custom_threshold_indicator) {

    if (isShowing) {
      changeAnimState(ANIM_RAMP_DOWN_TO_NONE);
      isShowing = false;
    }
    didDismissAboveThreshold = false;
  }


  switch (animState) {
    // User press animation logic
    case ANIM_USER_PRESS:
      {
        float additionalMulti = 1.0;
        for (workingIndexLed = 0; workingIndexLed < NUM_LEDS; workingIndexLed++) {
          // Set the i'th led to red
          leds[indexLeds[workingIndexLed]] = CRGB(0, 0, 0);
        }

        for (workingIndexLed = 0; workingIndexLed < animLedIndexTarget - 1; workingIndexLed++) {

          if ((animLedIndexTarget - 1) - workingIndexLed < 1.0) {
            additionalMulti = (animLedIndexTarget - 1) - workingIndexLed;
          }
          leds[indexLeds[workingIndexLed]] = CRGB(endingColors[workingIndexLed][1] * animIntensityTarget * additionalMulti, endingColors[workingIndexLed][0] * additionalMulti * animIntensityTarget, endingColors[workingIndexLed][2] * additionalMulti * animIntensityTarget);
        }
        FastLED.show();
        break;
      }
      // Above threshold and ramp down animation logic
    case ANIM_ABOVE_THRESHOLD:
    case ANIM_RAMP_DOWN_TO_NONE:
      {
        for (workingIndexLed = 0; workingIndexLed < NUM_LEDS; workingIndexLed++) {
          leds[indexLeds[workingIndexLed]] = CRGB(0, 0, 0);
        }

        leds[indexLeds[(int)animLedIndexTarget] - 1] = CRGB(endingColors[(int)animLedIndexTarget][1] * animIntensityTarget, endingColors[(int)animLedIndexTarget][0] * animIntensityTarget, endingColors[(int)animLedIndexTarget][2] * animIntensityTarget);
        leds[indexLeds[(int)animLedIndexTarget] + 1] = CRGB(endingColors[(int)animLedIndexTarget][1] * animIntensityTarget, endingColors[(int)animLedIndexTarget][0] * animIntensityTarget, endingColors[(int)animLedIndexTarget][2] * animIntensityTarget);
        leds[indexLeds[(int)animLedIndexTarget]] = CRGB(endingColors[(int)animLedIndexTarget][1] * animIntensityTarget, endingColors[(int)animLedIndexTarget][0] * animIntensityTarget, endingColors[(int)animLedIndexTarget][2] * animIntensityTarget);
        FastLED.show();
        isShowing = true;
        break;
      }
      // No sensor animation logic
    case ANIM_NO_SENSOR:
      {
        for (workingIndexLed = 0; workingIndexLed < NUM_LEDS; workingIndexLed++) {
          // Set the i'th led to red
          leds[indexLeds[workingIndexLed]] = CRGB(0, 0, 0);
        }
        for (workingIndexLed = 0; workingIndexLed < NUM_LEDS_NO_SENSOR_ANIM; workingIndexLed++) {
          leds[indexLeds[errorNoSensorLeds[workingIndexLed]]] = CRGB(0, (int)255 * animIntensityTarget, 0);
        }
        FastLED.show();
        break;
      }
      // Not read yet animation logic
    case ANIM_NOT_READ_YET:
      {
        for (workingIndexLed = 0; workingIndexLed < NUM_LEDS; workingIndexLed++) {
          // Set the i'th led to red
          leds[indexLeds[workingIndexLed]] = CRGB(0, 0, 0);
        }

        leds[indexLeds[0]] = CRGB((int)120 * animIntensityTarget, (int)0 * animIntensityTarget, (int)255 * animIntensityTarget);

        FastLED.show();

        break;
      }
      // Diagnostics animation logic
    case ANIM_DIAGNOSTICS:
      for (workingIndexLed = 0; workingIndexLed < NUM_LEDS; workingIndexLed++) {
        // Set the i'th led to red
        leds[indexLeds[workingIndexLed]] = CRGB(0, 0, 0);
      }

      leds[indexLeds[NUM_LEDS - 1]] = (!wasPrevConnectedToInternet) ? CRGB((int)0 * animIntensityTarget, (int)120 * animIntensityTarget, (int)0 * animIntensityTarget) : CRGB((int)120 * animIntensityTarget, (int)0 * animIntensityTarget, (int)0 * animIntensityTarget);
      leds[indexLeds[int((NUM_LEDS - 1) * 0.75)]] = (!wasPrevConnectedToLocalNet) ? CRGB((int)0 * animIntensityTarget, (int)120 * animIntensityTarget, (int)0 * animIntensityTarget) : CRGB((int)120 * animIntensityTarget, (int)0 * animIntensityTarget, (int)0 * animIntensityTarget);
      leds[indexLeds[int((NUM_LEDS - 1) * 0.5)]] = (!isSDCardOk) ? CRGB((int)0 * animIntensityTarget, (int)120 * animIntensityTarget, (int)0 * animIntensityTarget) : CRGB((int)120 * animIntensityTarget, (int)0 * animIntensityTarget, (int)0 * animIntensityTarget);


      FastLED.show();
      break;
  }
}

void mapColor(int value, int &rVal, int &gVal, int &bVal) {
  // Calculate the RGB values based on the input value
  if (value >= 0 && value <= 30) {
    gVal = map(value, 0, 30, 0, 255);
    rVal = 255;
    bVal = 0;
  } else if (value > 30 && value <= 100) {
    gVal = map(value, 30, 100, 128, 255);
    rVal = 255;
    bVal = map(value, 30, 100, 0, 255);
  } else if (value > 100 && value <= 200) {
    gVal = 255;
    rVal = map(value, 100, 200, 128, 0);
    bVal = map(value, 100, 200, 50, 255);
  } else if (value > 200 && value <= 500) {
    gVal = 255;
    rVal = 0;
    bVal = map(value, 200, 500, 255, 0);
  }
}

void playUserPressAnimation() {
  changeAnimState(ANIM_USER_PRESS);
}

void playNoSensorAnimation() {
  changeAnimState(ANIM_NO_SENSOR);
}
void playNotReadYetAnimation() {
  changeAnimState(ANIM_NOT_READ_YET);
}
void playDiagnosticsAnimation() {
  changeAnimState(ANIM_DIAGNOSTICS);
}
void playRampDownToNone() {
  changeAnimState(ANIM_RAMP_DOWN_TO_NONE);
}

void changeAnimState(AnimState newState) {
  if (newState == animState) {
    return;
  }

  switch (newState) {
    case ANIM_NONE:
      // Handle no animation
      isShowing = false;

      break;

    case ANIM_ABOVE_THRESHOLD:
      // Handle animation when the device is above a certain threshold
      timeline.clear();

      timeline.mode(Tween::Mode::REPEAT_SQ);

      workingBrightness = getCalculatedBrightness();

      timeline.add(animIntensityTarget)
        .init(0.1 * workingBrightness)
        .then(workingBrightness, 1000)
        .hold(50)
        .then(0, 500)
        .hold(1500);

      updateLightIndicatorHeight();
      isShowing = true;
      timeline.start();
      break;

    case ANIM_USER_PRESS:
      {
        //
        if (animState == ANIM_ABOVE_THRESHOLD) {
          didDismissAboveThresholdAnimationEvent = millis();
          didDismissAboveThreshold = true;
        }

        // Handle animation upon user interaction (press)
        timeline.clear();

        timeline.mode(Tween::Mode::ONCE);

        workingBrightness = getCalculatedBrightness();

        timeline.add(animIntensityTarget, true)
          .init(0)
          .then(workingBrightness, 500)
          .hold(custom_time_on + 1000)
          .then(0, 500, []() {
            changeAnimState(ANIM_NONE);
          });
        //find volume
        animLedIndexTarget = 0;
        workingAnimValue = 0;
        workingIndexLed = 0;
        while (workingAnimValue < currentAQIvalue) {
          workingAnimValue = scaleValues[workingIndexLed];
          workingIndexLed++;
        }
        workingIndexLed = fMax(2, workingIndexLed);
        timeline.add(animLedIndexTarget, true)
          .init(0)
          .then(workingIndexLed, 1500)
          .hold(custom_time_on)
          .then(0, 500, []() {
            changeAnimState(ANIM_NONE);
          });
        timeline.start();
        break;
      }

    case ANIM_RAMP_DOWN_TO_USER:
      // Handle animation ramping down to the user's state
      // This could include fading out notifications or updating visual cues to match user input
      timeline.clear();
      timeline.mode(Tween::Mode::ONCE);
      timeline.add(animIntensityTarget, true)
        .init(animIntensityTarget)
        .then(0, 200, []() {
          changeAnimState(ANIM_USER_PRESS);
        });
      timeline.start();
      break;

    case ANIM_RAMP_DOWN_TO_NONE:
      // Handle animation ramping down to no animation (default)
      timeline.clear();
      timeline.mode(Tween::Mode::ONCE);
      timeline.add(animIntensityTarget, true)
        .init(animIntensityTarget)
        .then(0, 500, []() {
          changeAnimState(ANIM_NONE);
        });
      timeline.start();
      break;

    case ANIM_NOT_READ_YET:
      // Handle animation for content that has not been read yet
      // This could include displaying a progress bar or adding visual cues to indicate newness

      workingBrightness = getCalculatedBrightness();

      timeline.clear();
      timeline.mode(Tween::Mode::ONCE);
      timeline.add(animIntensityTarget, true)
        .init(0)
        .then<Ease::SineInOut>(workingBrightness, 220)
        .then<Ease::SineInOut>(0.0, 120)
        .then<Ease::SineInOut>(workingBrightness, 220)
        .then<Ease::SineInOut>(0.0, 120)
        .then<Ease::SineInOut>(workingBrightness, 220)
        .then<Ease::SineInOut>(0.0, 120)
        .then<Ease::SineInOut>(workingBrightness, 220)
        .then<Ease::SineInOut>(0.0, 120, []() {
          changeAnimState(ANIM_NONE);
        });
      timeline.start();
      break;

    case ANIM_NO_SENSOR:
      // Handle animation when there is no sensor data available
      // For instance, provide feedback to the user that no sensor input was detected

      workingBrightness = getCalculatedBrightness();

      timeline.clear();
      timeline.mode(Tween::Mode::ONCE);
      timeline.add(animIntensityTarget, true)
        .init(0)
        .then<Ease::ExpoInOut>(workingBrightness, 2000)
        .hold(1500)
        .then<Ease::ExpoInOut>(0.0, 1000, []() {
          changeAnimState(ANIM_NONE);
        });
      timeline.start();

      break;
    case ANIM_DIAGNOSTICS:
      timeline.clear();
      timeline.mode(Tween::Mode::ONCE);

      workingBrightness = getCalculatedBrightness();

      timeline.add(animIntensityTarget, true)
        .init(0)
        .then<Ease::ExpoInOut>(workingBrightness, 2000)
        .hold(1500)
        .then<Ease::ExpoInOut>(0.0, 1000, []() {
          changeAnimState(ANIM_NONE);
        });
      timeline.start();
      break;
  }
  animState = newState;
}

void updateLightIndicatorHeight() {
  //find volume
  animLedIndexTarget = 0;
  workingAnimValue = 0;
  workingIndexLed = 0;
  while (workingAnimValue < currentAQIvalue) {
    workingAnimValue = scaleValues[workingIndexLed];
    workingIndexLed++;
  }
  animLedIndexTarget = fMax(2, workingIndexLed - 2);
  animLedIndexTarget = min((int)animLedIndexTarget, NUM_LEDS - 2);
}

float getCalculatedBrightness() {
  //CHECK LDR - if very dark damp the brightness
  workingLdrRaw = readLDR();
  workingLdrMapped = map(readLDR(), LDR_UPPER_THRESHOLD, 0.0, 1.0, 0.1);
  workingLdrMapped = fConstrain(workingLdrMapped, 0.1, 1.0);
  workingBrightness = fMin(custom_brightness_factor, workingLdrMapped);
  return workingBrightness;
}
