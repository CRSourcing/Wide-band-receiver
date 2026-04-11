
void tune() {  // selects tuning method depending on frequency and programs si5351

#ifdef TV_TUNER_PRESENT

  if (FREQ > HIGH_BAND_UPPER_LIMIT)  // upper limit
    FREQ = HIGH_BAND_UPPER_LIMIT;
#endif

#ifndef TV_TUNER_PRESENT
  if (FREQ > HIGHEST_ALLOWED_FREQUENCY)  // upper limit without tuner
    FREQ = HIGHEST_ALLOWED_FREQUENCY;
#endif


  // The below sequences programs the SI5351 directly if the RF is within DIRECTMODE range (tuner not in use)
  if (FREQ <= SHORTWAVE_MODE_UPPER_LIMIT && FREQ >= LOWEST_ALLOWED_FREQUENCY) {
    TVTunerActive = false;


    if (!singleConversionMode) {
      singleConversionMode = true;  // single conversion with LO above the IF, update needed to select BFO's
      loadSi4735parameters();
    }


    OLDPLLFREQ = -1;
    digitalWrite(RELAY_SWITCH_OUTPUT, HIGH);  // connect bandpass with AD831


    if (!lowSideInjection)
      LO_RX = abs((SI4735TUNED_FREQ * 1000) + FREQ);  // normal mode with LO above RF
    else
      LO_RX = abs((SI4735TUNED_FREQ * 1000) - FREQ);  // debugging mode with LO below RF


#ifdef SATELLITE_TUNER_PRESENT
#define SHORTWAVE_MODE_UPPER_LIMIT 150000000            // needss to cover 144HMz, 144 * 3 = 432MHz IF from satellite tuner
    LO_RX = abs((SI4735TUNED_FREQ * 1000) + FREQ / 3);  // 3rd harmonic of CLK2

#endif



    if (passbandShift)
      LO_RX -= passbandShift;  // if passband was shifted

    if (displayFineTuneOffset == false) {  // when using fine tune as "clarifier"
      LO_RX += fineTuneOffset;
    }



    si5351.set_freq(LO_RX * 100ULL, SI5351_CLK2);
    return;
  }

#ifdef TV_TUNER_PRESENT
  // If the freq is higher, we need to build the I2C data packet and program the TV tuner
  if (FREQ > SHORTWAVE_MODE_UPPER_LIMIT) {
    TVTunerActive = true;

    if (singleConversionMode) {
      singleConversionMode = false;  // double conversion, 2 * sideband inversion  cancel themselves out
      loadSi4735parameters();
    }

    programTVTuner();
  }

#endif
}

//##########################################################################################################################//


void setFreq() {
  // Sets the SI4732 to required frequency if WBFM or no mixer. Otherwise calls tune() to set the SI5351 and TVtuner.


  if (noMixer) {  // debug - tune the SI4732 directly

    if (SI4735WBFMTune) {
      TVTunerActive = false;
      tuneWBFMSI4735();
      return;
    }

    si4735.setFrequency(FREQ_TO_KHZ);
    return;
  }

  if (SI4735WBFMTune) {
    tuneWBFMSI4735();  // tune SI4735  for WBFM

#ifdef TINYSA_PRESENT
    LO_RX = abs(((SI4735TUNED_FREQ * 1000) - FREQ));
    si5351.set_freq(LO_RX * 100ULL, SI5351_CLK2);  // tune the synthesizer so that the tinySA shows WBFM spectrum. Use low injection;
    return;
#endif
  }

  else
    tune();  // select tuning method and tune in
  return;
}


//##########################################################################################################################//

void displayLOFreq() {  // diplays the clock frequency in the lower left corner

  if (displayDebugInfo == false)
    return;

  if ((FREQ >> 10 == F_OLD) && (PLLFREQ == OLDPLLFREQ))  // roughly 1 KHz deviation
    return;

  else
    F_OLD = FREQ >> 10;

  tft.fillRect(0, 298, 134, 10, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 298);

  if (SI4735WBFMTune) {
    tft.printf("F: %ld", FREQ_TO_KHZ);
    return;
  }

  if (FREQ > SHORTWAVE_MODE_UPPER_LIMIT)
    tft.printf("LO:%ldK  TU:%ldM", LO_RX / 1000, PLLFREQ / 1000000);
  else
    tft.printf("LO:%ldK", LO_RX / 1000);
  tft.setTextSize(2);
  tft.setTextColor(textColor);
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
    disableFFT = true;                                         //temporarily disable mini window for speed

    if (lastPos)
      tft.fillCircle(lastPos, 47, 4, TFT_BLACK);  // override previous dot
    tft.drawFastHLine(lastPos - 5, 46, 15, TFT_GRID);

    if (!potResult)
      tft.fillCircle(potVal / 37 + 10, 47, 4, TFT_GREEN);  // green center dot
    else
      tft.fillCircle(potVal / 37 + 10, 47, 4, TFT_RED);  // create moving dot
    lastPos = potVal / 37 + 10;

    if (displayFineTuneOffset) {  // display fine tune frequency ofset
      FREQ -= oldPot;             // adjust FREQ directly
      FREQ += potResult;
    }



    else {                       // don't
      fineTuneOffset -= oldPot;  // adjust LO_RX instead, FREQ display will not change
      fineTuneOffset += potResult;
      tune();  // call sune so that LO_RX gets updated
    }

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
    Serial_printf("Last saved after %ldsec (FREQ=%ld)\n", millis() / 1000, FREQ);
    hasWritten = true;  // Block further writes until next change
  }
}
//##########################################################################################################################//



void addFrequencyAndModeToHistoryBuffer(uint32_t freq, char mode) {
  buffer[bufferIndex].frequency = freq;
  buffer[bufferIndex].mode = mode;
  bufferIndex = (bufferIndex + 1) % 8;  // Wrap around
  if (bufferIndex == 0) bufferFull = true;
}


//##########################################################################################################################//



void vfoSelector() {

  static bool init = false;
  static long vfo1Freq = 0;
  static long vfo2Freq = 0;
  static int vfo1ModType = -1;
  static int vfo2ModType = -1;

  if (vfo1Active) {  //VFO 2 was active, VFO1 will be used

    vfo2Freq = FREQ;  // save FREQ and modType of VFO2
    vfo2ModType = modType;
    FREQ = vfo1Freq;

    if (modType != vfo1ModType) {
      modType = vfo1ModType;
      loadSi4735parameters();
    }
  }

  else {  //VFO 1 was active, VFO2 gwill be used

    vfo1ModType = modType;

    if (!init) {  // first toggle, must initiate VFO2
      vfo2Freq = FREQ;
      vfo2ModType = modType;
      init = true;
    }

    vfo1Freq = FREQ;
    FREQ = vfo2Freq;

    if (modType != vfo2ModType) {
      modType = vfo2ModType;
      loadSi4735parameters();
    }
  }
}

//##########################################################################################################################//


void vfoMenu() {

  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);
  tft.fillRect(3, 121, 333, 114, TFT_BLACK);

  drawButton(8, 234, TILE_WIDTH, TILE_HEIGHT, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setTTFFont(Arial_14);
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(20, 254);
  etft.print(F("BACK"));

  etft.setCursor(102, 244);
  etft.print(F("Use"));
  etft.setCursor(102, 264);
  etft.print(F("VFO 1"));

  etft.setCursor(185, 244);
  etft.print(F("Use"));
  etft.setCursor(185, 264);
  etft.print(F("VFO 2"));

  // etft.setCursor(270, 244);
  // etft.print(F("Use");
  // etft.setCursor(270, 264);
  // etft.print(F("List");


  tRel();
  tPress();
  readVfoBtns();
}


//##########################################################################################################################//

void readVfoBtns() {


  int buttonID = getButtonID();
  if (!buttonID)
    return;                                   // outside of area
                                              // outside of keypad area
  redrawMainScreen = true;                    // save freq for returning from waterfall
  tft.fillRect(135, 295, 92, 25, TFT_BLACK);  // overwrite frozen spectrum window
  switch (buttonID) {
    case 41:
      return;
    case 42:
      vfo1Active = true;
      vfoSelector();
      tRel();
      return;
    case 43:
      vfo1Active = false;
      vfoSelector();
      tRel();
      return;
    case 44:
      return;
    default:
      tx = ty = pressed = 0;
      tRel();
      return;
  }
}
//##########################################################################################################################//
// Analog transceiver style dial and nixie tube frequency display
//##########################################################################################################################//

#define DIAL_RADIUS 150
#define DIAL_CENTER_X 118
#define DIAL_CENTER_Y 224  // center below "horizon" so only upper arc is visible

void drawDial(uint32_t freq)  // draws an analog transceiver style dial
{
  displayFreqNixie(FREQ, 240, 6);  // displays the frequency with nixie tubes

  const int RADIUS = DIAL_RADIUS * 1.5;
  const int ARC_START = 60;  // degrees
  const int ARC_END = 118;
  const int ARC_SPAN = ARC_END - ARC_START;  // must be 58 to fit

  const int numTicks = 40;
  const float stepAngle = (float)ARC_SPAN / (numTicks - 1);
  float freqRatio;  // ticks per KHz
  // static arrays to erase old ticks ---
  static bool firstDraw = true;
  static float prevAngle[numTicks];
  static float prevLogicalValue[numTicks] = { 0 };

  freqRatio = (freq / STEP);  // calculate ratio


  float phaseFrac = fmodf(freqRatio, 1.0f);
  // calculate values
  float currLogicalValue[numTicks];
  bool currIsTenth[numTicks];
  bool currIsFifth[numTicks];

  for (int i = 0; i < numTicks; i++) {
    currLogicalValue[i] = freqRatio + i;

    int intPart = (int)floorf(currLogicalValue[i]);
    currIsTenth[i] = (intPart % 10 == 0);
    currIsFifth[i] = (intPart % 5 == 0) && !currIsTenth[i];
  }

  // --- erase prev ticks ---
  if (!firstDraw) {
    for (int i = 0; i < numTicks; i++) {
      float angle = prevAngle[i] * DEG_TO_RAD;

      int inner = RADIUS - 5;
      int outer = RADIUS - 8;

      int prevIntPart = (int)floorf(prevLogicalValue[i]);
      bool wasTenth = (prevIntPart % 10 == 0);
      bool wasFifth = (prevIntPart % 5 == 0) && !wasTenth;

      if (wasTenth) {
        outer = RADIUS - 15;
      } else if (wasFifth) {
        outer = RADIUS - 12;
      }

      int x1 = DIAL_CENTER_X + cos(angle) * inner;
      int y1 = DIAL_CENTER_Y - sin(angle) * inner;
      int x2 = DIAL_CENTER_X + cos(angle) * outer;
      int y2 = DIAL_CENTER_Y - sin(angle) * outer;

      if (wasTenth) {
        tft.drawLine(x1, y1, x2, y2, TFT_DIAL_BACKGROUND);
        tft.drawLine(x1 + 1, y1, x2 + 1, y2, TFT_DIAL_BACKGROUND);
        tft.drawLine(x1 - 1, y1, x2 - 1, y2, TFT_DIAL_BACKGROUND);
        tft.drawLine(x1, y1 + 1, x2, y2 + 1, TFT_DIAL_BACKGROUND);
        tft.drawLine(x1, y1 - 1, x2, y2 - 1, TFT_DIAL_BACKGROUND);

      } else if (wasFifth) {
        tft.drawLine(x1, y1, x2, y2, TFT_DIAL_BACKGROUND);
        tft.drawLine(x1 + 1, y1, x2 + 1, y2, TFT_DIAL_BACKGROUND);
        tft.drawLine(x1 - 1, y1, x2 - 1, y2, TFT_DIAL_BACKGROUND);
        tft.drawLine(x1, y1 + 1, x2, y2 + 1, TFT_DIAL_BACKGROUND);
      }
    }
  }



  tft.drawFastVLine(DIAL_CENTER_X, 18, 10, TFT_RED);  // center marker
  tft.setTextColor(TFT_DEEPORANGE);
  tft.setCursor(DIAL_CENTER_X - 40, 25);  // + ad -for fine tune
  tft.print(F("-"));
  tft.setCursor(DIAL_CENTER_X + 40, 25);
  tft.print(F("+"));




  // Calculate and save current angles and values
  for (int i = 0; i < numTicks; i++) {
    float angle = ARC_START + i * stepAngle + phaseFrac * stepAngle;
    prevAngle[i] = angle;
    prevLogicalValue[i] = currLogicalValue[i];
  }

  // --- draw new ticks ---
  for (int i = 0; i < numTicks; i++) {
    float angle = prevAngle[i] * DEG_TO_RAD;

    int inner = RADIUS - 5;
    int outer = RADIUS - 8;

    if (currIsTenth[i]) {
      outer = RADIUS - 15;
    } else if (currIsFifth[i]) {
      outer = RADIUS - 12;
    }

    int x1 = DIAL_CENTER_X + cos(angle) * inner;
    int y1 = DIAL_CENTER_Y - sin(angle) * inner;
    int x2 = DIAL_CENTER_X + cos(angle) * outer;
    int y2 = DIAL_CENTER_Y - sin(angle) * outer;

    if (i > 1) {
      if (currIsTenth[i]) {
        tft.drawLine(x1, y1, x2, y2, TFT_DIAL_TICKS);
        tft.drawLine(x1 + 1, y1, x2 + 1, y2, TFT_DIAL_TICKS);
        tft.drawLine(x1 - 1, y1, x2 - 1, y2, TFT_DIAL_TICKS);
        tft.drawLine(x1, y1 + 1, x2, y2 + 1, TFT_DIAL_TICKS);
        tft.drawLine(x1, y1 - 1, x2, y2 - 1, TFT_DIAL_TICKS);
      } else if (currIsFifth[i]) {
        tft.drawLine(x1, y1, x2, y2, TFT_DIAL_TICKS);
        tft.drawLine(x1 + 1, y1, x2 + 1, y2, TFT_DIAL_TICKS);
        tft.drawLine(x1 - 1, y1, x2 - 1, y2, TFT_DIAL_TICKS);
        tft.drawLine(x1, y1 + 1, x2, y2 + 1, TFT_DIAL_TICKS);
      } else {
        tft.fillCircle(x1, y1, 1, TFT_DIAL_TICKS);
      }
    }
  }

  firstDraw = false;
}
//##########################################################################################################################//

void dialFineTune(void) {  //

  if (pressed) {

    if (tx > DIAL_CENTER_X)
      FREQ += STEP / 100;

    else if (tx < DIAL_CENTER_X)
      FREQ -= STEP / 100;
    pressed = false;
    tx = 0;
    ty = 0;
  }
}

//##########################################################################################################################//


void displayFreqNixie(uint32_t frequency, uint16_t x, uint16_t y) {
  // Array of pointers to nixie bitmaps
  const unsigned short* nixieDigits[] = { nx0, nx1, nx2, nx3, nx4, nx5, nx6, nx7, nx8, nx9 };

  uint16_t digitSpacing = 2;  // Space btw digits
  uint16_t currentX = x;
  uint8_t digits = 0;
  static uint8_t oldDigits = 0xFF;


  tft.setSwapBytes(false);  // in case was set true

  char freqStr[12];

  snprintf(freqStr, sizeof(freqStr), "%lu", frequency);

  // Display characters
  for (digits = 0; freqStr[digits] != '\0'; digits++) {
    char c = freqStr[digits];

    if (c >= '0' && c <= '9') {
      // Draw nixie
      int digit = c - '0';
      tft.pushImage(currentX, y, nxfontWidth, nxfontHeight, nixieDigits[digit]);
      currentX += nxfontWidth + digitSpacing;
    }
  }

  uint32_t decPt = frequency / 1000000l;
  uint16_t decPtX = x + 80;  // calculate decimal point. position for >= 100MHz

  if (decPt >= 10 && decPt < 100) decPtX -= nxfontWidth + digitSpacing;  // 10-100MHz
  else if (decPt < 10) decPtX -= 2 * (nxfontWidth + digitSpacing);       // 1-10MHz
  else if (!decPt) decPtX -= 3 * (nxfontWidth + digitSpacing);           // < 1Mhz



  if (decPt) {  // draw decimal point
    tft.fillCircle(decPtX, y + nxfontHeight - 7, 2, TFT_DEEPORANGE);
    tft.fillCircle(decPtX + 3 * (nxfontWidth + digitSpacing), y + nxfontHeight - 7, 2, TFT_DEEPORANGE);
  } else
    tft.fillCircle(decPtX + 2 * (nxfontWidth + digitSpacing), y + nxfontHeight - 7, 2, TFT_DEEPORANGE);


  if (digits < oldDigits && oldDigits != 0xFF) {  // overwrite space to the right

    tft.fillRect(currentX, y, 476 - currentX, nxfontHeight, TFT_BLACK);
  }

  oldDigits = digits;
}
