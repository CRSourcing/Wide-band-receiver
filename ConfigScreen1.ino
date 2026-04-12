void ConfigScreen1() {  //first configuration screen
  clearStatusBar();
  redrawMainScreen = true;
  drawCf1Btns();
  readCf1Btns();
}

//##########################################################################################################################//

void drawCf1Btns() {

  draw12Buttons(TFT_VIOLET, TFT_PURPLE);
  etft.setTTFFont(Arial_14);

  if (altStyle)
    etft.setTextColor(textColor);
  else
    etft.setTextColor(TFT_PINK);


  struct Button {
    const int x;
    const int y;
    const char *text;
  };

  const Button buttons[] PROGMEM = {
    { 20, 134, "Show" },
    { 20, 152, "Panor." },
    { 105, 132, "Retro" },
    { 105, 152, "Dial" },
    { 185, 132, "Show" },
    { 187, 151, "Meters " },
    { 270, 132, "Auto" },
    { 270, 151, "Sqlch" },
    { 25, 190, "Fine" },
    { 25, 210, "Tune" },
    { 105, 190, "Loop" },
    { 105, 210, "Bands" },
    { 190, 190, "Set" },
    { 190, 210, "AVC" },
    { 272, 190, "Btn" },
    { 272, 210, "Style" },
    { 270, 245, "WIFI" },
    { 268, 265, "Cred." },
    { 182, 245, "Master" },
    { 178, 268, "Volume" },
    { 100, 245, "Set" },
    { 100, 268, "SMUTE" }
  };


  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].text);
  }


  drawButton(8, 236, 75, 49, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(20, 245);
  etft.print(F("More"));
  etft.setCursor(20, 265);
  etft.print(F("Config"));
  tDoublePress();
}


//##########################################################################################################################//

void readCf1Btns() {
  if (!pressed) return;

  int buttonID = getButtonID();

  if (!buttonID)
    return;  // outside of key area
  tft.setTextColor(TFT_GREEN);
  switch (buttonID) {
    case 21:
      panoramaScreen();
      break;
    case 22:
      useNixieDial = !useNixieDial;
      tft.fillRect(3, 3, 335, 42, TFT_BLACK);                    // overwrite freq digits
      tft.fillRect(330, 4, 145, 22, TFT_BLACK);                  // overwrite microvolts
      tft.fillRoundRect(5, 3, 235, 42, 3, TFT_DIAL_BACKGROUND);  // overwrite space for dial or FREQ digits
      if (useNixieDial)
        tft.fillRect(335, 26, 140, 20, TFT_BLACK);  // overwrite step display
      else
        displaySTEP(true);  // restore
      preferences.putBool("dial", useNixieDial);
      FREQ_OLD -= 1;  // force freq display
      break;
    case 23:
      showMeters = !showMeters;
      if (showMeters) {
        tft.setCursor(10, 75);
        tft.print(F("Tap step display L/R"));
        tft.setCursor(10, 95);
        tft.print(F("to change frequency"));
        delay(1000);
      }
      preferences.putBool("sM", showMeters);
      preferences.putBool("fB", true);  // activate fastboot
      ESP.restart();
      break;

    case 24:
      SNRSquelch = !SNRSquelch;  //  SNRSquelch use SNR to trigger audio mute and the squelch pot will not be used. This only works in AM mode
      preferences.putBool("SNRSquelch", SNRSquelch);
      tft.setCursor(10, 75);
      tft.printf("Use SNR AND RSSI");
      tft.setCursor(10, 95);
      tft.printf("for Squelch: %s", SNRSquelch ? "Yes" : "No");
      delay(1000);
      break;

    case 31:
      displayFineTuneOffset = !displayFineTuneOffset;
      preferences.putBool("dft", displayFineTuneOffset);
      tft.setCursor(10, 90);
      tft.printf("Show fine tune offset: %s", displayFineTuneOffset ? "Yes" : "No");
      delay(1000);

      break;
    case 32:
      loopBands = !preferences.getBool("uBL", false);
      preferences.putBool("uBL", loopBands);
      tft.setCursor(10, 75);
      tft.print(loopBands ? "Band looping enabled!" : "Band looping disabled!");
      delay(1000);
      return;

    case 33:
      setAvcAmMaxGain();
      break;
    case 34:
      selectButtonStyle();
      break;
    case 41:
      clearStatusBar();
      ConfigScreen2();
      break;
    case 42:
      setSMute();
      break;
    case 43:
      setVol();
      break;
    case 44:
      saveWifiCredentials();
      break;
    default:
      resetMainScreen();
      return;
  }
  redrawMainScreen = true;
  tRel();
}

//##########################################################################################################################//

void calibSI5351() {  // calibrates the SI5351.

  encLockedtoSynth = false;
  clearStatusBar();
  while (digitalRead(ENCODER_BUTTON) == HIGH) {
    clearNotification();

    if (clw)
      SI5351calib += 500;
    if (cclw)
      SI5351calib -= 500;
    clw = false;
    cclw = false;

    si5351.set_correction(SI5351calib, SI5351_PLL_INPUT_XO);
    calculateAndDisplaySignalStrength();
    tft.fillRect(5, 100, 333, 16, TFT_GREY);
    tft.setCursor(5, 100);
    tft.printf("SI5351 calibration.:%ld ", SI5351calib);
    delay(50);
  }
  preferences.putLong("calib", SI5351calib);
  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;
  encLockedtoSynth = true;
}

//##########################################################################################################################//

void setVol() {  //sets SI4732 master volume

  vol = preferences.getChar("Vol", 50);
  encLockedtoSynth = false;
  clearStatusBar();
  displayText(10, 75, 0, 0, "Use encoder to set volume");
  delay(1000);
  clearNotification();
  while (digitalRead(ENCODER_BUTTON) == HIGH) {
    if (vol > 63)
      vol = 0;
    if (vol < 1)
      vol = 1;
    clearNotification();
    tft.setCursor(10, 75);
    tft.printf(" Master Volume: %d", vol);
    delay(50);
    if (clw) {
      vol++;
      si4735.setVolume(vol);
    }

    if (cclw) {
      vol--;
      si4735.setVolume(vol);
    }

    clw = false;
    cclw = false;
  }
  preferences.putChar("Vol", vol);
  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;
  encLockedtoSynth = true;
}
//##########################################################################################################################//

void setIF() {  // adjusts 21.4 MHz IF up and down so that it matches center frequency of crystal filter.

  encLockedtoSynth = false;
  audioMuted = false;

  displayText(10, 75, 0, 0, "Use encoder to set IF.");
  delay(1000);
  clearNotification();
  tft.setCursor(10, 75);
  tft.printf("IF %d KHz", si4735.getCurrentFrequency());

  while (digitalRead(ENCODER_BUTTON) == HIGH) {

    delay(50);

    if (clw) {
      si4735.setFrequencyUp();
    }
    if (cclw) {
      si4735.setFrequencyDown();
    }

    if (clw + cclw) {
      clearNotification();
      tft.setCursor(5, 75);
      SI4735TUNED_FREQ = si4735.getCurrentFrequency();
      tft.printf("IF: %d KHz", SI4735TUNED_FREQ);
      clw = false;
      cclw = false;
    }
  }

  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;
  preferences.putLong("IF", SI4735TUNED_FREQ);
  encLockedtoSynth = true;
}



//##########################################################################################################################//

void setAvcAmMaxGain() {


  encLockedtoSynth = false;
  displayText(10, 75, 0, 0, "Use encoder to change AVC");

  uint8_t AvcAmMaxGain = preferences.getChar("AVC", 0);
  si4735.setAvcAmMaxGain(AvcAmMaxGain);

  while (digitalRead(ENCODER_BUTTON) == HIGH) {

    delay(20);
    if (clw)
      AvcAmMaxGain += 10;
    if (cclw)
      AvcAmMaxGain -= 10;

    if (AvcAmMaxGain > 127)
      AvcAmMaxGain = 0;

    si4735.setAvcAmMaxGain(AvcAmMaxGain);

    if (clw != cclw) {
      printAvcAmMaxGain(AvcAmMaxGain);
    }
    clw = false;
    cclw = false;
  }

  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;

  preferences.putChar("AVC", AvcAmMaxGain);

  encLockedtoSynth = true;
}

//##########################################################################################################################//

void printAvcAmMaxGain(uint8_t AvcAmMaxGain) {

  tft.setTextColor(TFT_GREEN);

  if (altStyle)
    tft.fillRoundRect(225, 95, 100, 22, 10, TFT_BLUE);

  else

    tft.pushImage(225, 92, 102, 26, (uint16_t *)Oval102);

  tft.setCursor(235, 98);
  tft.printf("AVC:%d", AvcAmMaxGain);
  tft.setTextColor(textColor);
}

//##########################################################################################################################//


void setSMute() {

  static uint8_t AMSoftMuteSnrThreshold = 0;

  encLockedtoSynth = false;
  clearStatusBar();
  displayText(10, 75, 0, 0, "Set AM SoftMuteSnrThreshold");

  AMSoftMuteSnrThreshold = preferences.getChar("SMute", 0);
  si4735.setAMSoftMuteSnrThreshold(AMSoftMuteSnrThreshold);

  while (digitalRead(ENCODER_BUTTON) == HIGH) {

    delay(50);
    if (clw)
      AMSoftMuteSnrThreshold += 4;
    if (cclw)
      AMSoftMuteSnrThreshold -= 4;

    if (AMSoftMuteSnrThreshold > 63)
      AMSoftMuteSnrThreshold = 0;

    si4735.setAMSoftMuteSnrThreshold(AMSoftMuteSnrThreshold);

    if (clw != cclw) {
      printSmute(AMSoftMuteSnrThreshold);
    }
    clw = false;
    cclw = false;
  }

  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;
  preferences.putChar("SMute", AMSoftMuteSnrThreshold);
  encLockedtoSynth = true;
  clearStatusBar();
}

//##########################################################################################################################//

void printSmute(uint8_t AMSoftMuteSnrThreshold) {

  tft.setTextColor(TFT_GREEN);

  if (altStyle)
    tft.fillRoundRect(225, 95, 100, 22, 10, TFT_BLUE);

  else
    tft.pushImage(225, 92, 102, 26, (uint16_t *)Oval102);

  tft.setCursor(232, 98);
  tft.printf("SMut:%d ", AMSoftMuteSnrThreshold / 4);
  tft.setTextColor(textColor);
}


//##########################################################################################################################//


void panoramaScreen() {

  showPanorama = !showPanorama;
  preferences.putBool("showPan", showPanorama);
  if (showPanorama)
    showAudioWaterfall = false;  // panoramascan has priority

  tft.setCursor(10, 65);
  tft.print(F("Shows waterfall when"));
  tft.setCursor(10, 85);
  tft.print(F("squelch is closed (AM/FM)."));
  tft.setCursor(10, 110);
  tft.printf("Panorama: %s\n", showPanorama ? "ON" : "OFF");
  delay(2000);
}


//##########################################################################################################################//



void shiftPassBand() {

  encLockedtoSynth = false;
  audioMuted = false;
  tft.fillRect(5, 63, 333, 229, TFT_BLACK);
  tft.setCursor(10, 65);
  tft.print(F("Shifts passband through"));
  tft.setCursor(10, 90);
  tft.print(F("crystal filter window."));

  tft.setCursor(10, 150);
  tft.printf("Passband shift: %ld KHz", passbandShift / 1000);

  int16_t offset = 0;

  if (modType == USB && singleConversionMode)
    offset = preferences.getInt("B1", 0);
  if (modType == USB && !singleConversionMode)
    offset = preferences.getInt("B2", 0);

  if (modType == LSB && singleConversionMode)
    offset = preferences.getInt("B3", 0);
  if (modType == LSB && !singleConversionMode)
    offset = preferences.getInt("B4", 0);



  if (modType == SYNC)
    offset = preferences.getInt("SYNCBfoOffset", 0);
  if (modType == CW)
    offset = preferences.getInt("CWBfoOffset", 0);



  while (digitalRead(ENCODER_BUTTON) == HIGH) {

    delay(50);

    if (clw) {

      SI4735TUNED_FREQ++;
      si4735.setFrequencyUp();
      passbandShift += 1000;

      tune();
    }
    if (cclw) {
      SI4735TUNED_FREQ--;
      si4735.setFrequencyDown();
      passbandShift -= 1000;

      tune();
    }

    if (clw + cclw) {

      si4735.setSSBBfo(offset + passbandShift);


      tft.fillRect(10, 150, 325, 15, TFT_BLACK);
      tft.setCursor(10, 150);
      tft.printf("Passband shift: %ld KHz", passbandShift / 1000);
      clw = false;
      cclw = false;
    }
  }

  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;
  encLockedtoSynth = true;
}

//##########################################################################################################################//
void saveWifiCredentials() {

  String ssid, password;

  tft.fillScreen(TFT_BLACK);

  displayText(0, 0, 0, 0, "Enter network name:");

  ssid = readKeyboard();

  if (ssid.length() == 0) {
    rebuildMainScreen(false);
    return;
  }


  tft.fillRect(0, 0, 480, 50, TFT_BLACK);

  displayText(0, 0, 0, 0, "Enter password:");
  password = readKeyboard();

  if (password.length() == 0) {
    rebuildMainScreen(false);
    return;
  }


  tft.fillRect(0, 0, 480, 50, TFT_BLACK);
  tft.setCursor(0, 0);
  tft.printf("SSID: %s\nPassword: %s", ssid.c_str(), password.c_str());

  displayText(0, 50, 0, 0, "Tap Enter to save, move encoder to skip.");
  tRel();

  pressed = false;

  while (true) {
    pressed = get_Touch();

    if (pressed && (ty >= 270 && ty <= 304) && (tx >= 340 && tx <= 440)) {
      preferences.putString("ssid", ssid);
      preferences.putString("password", password);
      preferences.putBool("fB", true);
      ESP.restart();
    }

    else if (clw + cclw) {
      clw = false;
      cclw = false;
      rebuildMainScreen(false);
      return;
    }
  }
}