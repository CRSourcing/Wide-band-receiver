void setLO() {
  // Sets the oscillator(s) to required frequency. Redirects to selectTuningMethod() when using VHF/UHF.


  if (noMixer) {  // debug - tune the SI4732 directly

    if (SI4735WBFMTune) {
      tuneWBFMSI4735();
      return;
    }
    si4735.setFrequency(FREQ / 1000);
    return;
  }


  if (SI4735WBFMTune) {
    tuneWBFMSI4735();  // tune SI4735  for WBFM
#ifdef TINYSA_PRESENT
    LO_RX = abs(((SI4735TUNED_FREQ * 1000) - FREQ));
    si5351.set_freq(LO_RX * 100ULL, SI5351_CLK2);  // tune the synthesizer so that the tinySA shows WBFM spectrum. Use low injectionurn;
    return;
#endif
  } else

    selectTuningMethod();
  return;
}


//##########################################################################################################################//

void clockDisplay() {  // diplays the clock frequency in the lower left corner

  static long OLD_LO_RX = -1;

  if (displayDebugInfo == false)
    return;


  if (OLD_LO_RX != LO_RX) {  // overwrite only when changed. This avoids flicker
    tft.fillRect(0, 296, 134, 12, TFT_BLACK);
    OLD_LO_RX = LO_RX;
  }

  etft.setTextColor(TFT_GREEN);
  etft.setCursor(0, 296);
  etft.setTTFFont(Arial_9);

  if (SI4735WBFMTune) {
    OLD_LO_RX = -1;
    etft.printf("F: %ld", FREQ / 1000);
    return;
  }

  if (FREQ > SHORTWAVE_MODE_UPPER_LIMIT)
    etft.printf("LO:%ldK  TU:%ldM", LO_RX / 1000, PLLFREQ / 1000000);
  else
    etft.printf("LO:%ldK", LO_RX / 1000);
  etft.setTextColor(textColor);
}

//##########################################################################################################################//


void fineTune() {  // reads fine tune potentiometer and adjusts FREQ

  static uint32_t lastResult = 0;
  static long oldPot = 0;
  uint32_t os = 0;
  const float div = 10.92;
  long potResult = 0;
  static uint16_t lastPos = 0;

  if (modType == WBFM)
    return;

  for (int l = 0; l < 32; l++)
    os += analogRead(FINETUNE_PIN);
  potVal = os / div;  // oversample and  set range from 0 to 12000 (= +-1 crystal filter bandwith)

  if (lastResult == potVal)
    return;
  else
    lastResult = potVal;

  // Normalize the ADC value to a range of -1 to 1
  double normalized_value = ((double)potVal / 12000) * 2 - 1;

  // quadratic transformation to stretch values around mid
  double transformed_value = normalized_value * normalized_value * (normalized_value < 0 ? -1 : 1);

  // Normalize back to 0 to 1 range
  transformed_value = (transformed_value + 1) / 2;

  // Scale the transformed value back to the ADC range
  potVal = (uint16_t)(transformed_value * 12000);
  potResult = (potVal - 6000) / 30 * 10;  // fine tune +- 2KHz in 10 Hz Steps

  if (potResult <= oldPot - 10 || potResult >= oldPot + 10) {  // pot has changed
    disableFFT = true;                                         //temporarily disable spectrum analyzer to continuously read values

    if (lastPos)
      tft.fillCircle(lastPos, 47, 4, TFT_BLACK);  // override previous dot
    tft.drawFastHLine(lastPos - 5, 46, 15, TFT_GRID);

    if (!potResult)
      tft.fillCircle(potVal / 37 + 10, 47, 4, TFT_GREEN);  // green center dot
    else
      tft.fillCircle(potVal / 37 + 10, 47, 4, TFT_RED);  // create moving dot
    lastPos = potVal / 37 + 10;

    FREQ -= oldPot;
    FREQ += potResult;
    oldPot = potResult;
  
  
  }
}

//##########################################################################################################################//
void saveCurrentSettings() {
  static uint32_t lastChangeTime = millis();
  static uint32_t FROLD = 0;
  static bool hasWritten = false;
  const uint32_t writeDelay = 60000;  // 60s cycle

  // Reset timer and flags if FREQ changes more than 1 step
  if (labs(FREQ - FROLD) > STEP) {
    lastChangeTime = millis();
    hasWritten = false;
    FROLD = FREQ;
  }

  // Write ONCE after 1 minute of inactivity
  if (!hasWritten && (millis() - lastChangeTime >= writeDelay)) {
    preferences.putLong("lastFreq", FREQ);
    preferences.putInt("lastBw", bandWidth);
    preferences.putChar("lastMod", modType);
    addFrequencyAndModeToHistoryBuffer(FREQ, modType);
    Serial_printf("Last saved after %ldsec (FREQ=%ld)\n", millis()/1000, FREQ);
    hasWritten = true;  // Block further writes until next change
  }
}
//##########################################################################################################################//



void addFrequencyAndModeToHistoryBuffer(uint32_t freq, char mode) {
  buffer[bufferIndex].frequency = freq;
  buffer[bufferIndex].mode = mode;
  bufferIndex = (bufferIndex + 1) % 10;  // Wrap around
  if (bufferIndex == 0) bufferFull = true;
}


//##########################################################################################################################//
