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
    int x;
    int y;
    const char *text;
  };

  Button buttons[] = {
    { 20, 134, "Show" },
    { 20, 152, "Panor." },
    { 105, 132, "7Seg" },
    { 105, 152, "Font" },
    { 185, 132, "Show" },
    { 187, 151, "Meters " },
    { 270, 132, "Auto" },
    { 270, 151, "Sqlch" },
    { 16, 190, "Set IF" },
    { 16, 210, "SI4732" },
    { 105, 190, "Loop" },
    { 105, 210, "Bands" },
    { 190, 190, "Set" },
    { 190, 210, "AVC" },
    { 272, 190, "Have" },
    { 272, 210, "Fun" },
    { 267, 245, "Calib" },
    { 267, 268, "5351" },
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
  etft.print("More");
  etft.setCursor(20, 265);
  etft.print("Config");
  tDoublePress();
}


//##########################################################################################################################//

void readCf1Btns() {
  if (!pressed) return;

  int buttonID = getButtonID();

  if (row > 4 || column > 4)
    return;  // outside of key area
  tft.setTextColor(TFT_GREEN);
  switch (buttonID) {
    case 21:
      showPanorama = !showPanorama;
      preferences.putBool("showPan", showPanorama);
      if (showPanorama)
        showAudioWaterfall = false;  // panoramascan has priority

      tft.setCursor(10, 65);
      tft.print("Shows waterfall when");
      tft.setCursor(10, 85);
      tft.print("squelch is closed (AM/FM).");
      tft.setCursor(10, 110);
      tft.printf("Panorama: %s\n", showPanorama ? "ON" : "OFF");
      delay(2000);


      break;
    case 22:
      tft.fillRect(5, 5, 325, 38, TFT_BLACK);
      sevenSeg = !sevenSeg;
      displayFREQ(FREQ);
      preferences.putBool("sevenSeg", sevenSeg);
      break;

    case 23:
      showMeters = !showMeters;
      if (showMeters) {
        tft.setCursor(10, 75);
        tft.print("Tap frequency display L/R");
        tft.setCursor(10, 95);
        tft.print("to change frequency");
        delay(1000);
      }
      preferences.putBool("sM", showMeters);
      preferences.putBool("fB", true);
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
      setIF();
      break;
    case 32:
      loopBands = preferences.getBool("uBL", 0);  // activate or deactivate band limits
      if (loopBands) {
        loopBands = false;
        preferences.putBool("uBL", loopBands);
        tft.setCursor(10, 75);
        tft.print("Band looping disabled!");
        delay(1000);
        return;
      }
      if (!loopBands) {
        loopBands = true;
        preferences.putBool("uBL", loopBands);
        tft.setCursor(10, 75);
        tft.print("Band looping enabled!");
        delay(1000);
        return;
      }
      break;
    case 33:
      setAvcAmMaxGain();
      break;
   case 34:
      funEnabled =!funEnabled;
      tft.setCursor(10, 75);
      if(funEnabled)
        tft.print("Have fun!");
      else
       tft.print("Fun disabled");
      preferences.putBool("fun", funEnabled);
      delay(500);
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
      calibSI5351();
      break;
    default:
      redrawMainScreen = true;
      tx = ty = pressed = 0;
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
    clw = 0;
    cclw = 0;


    si5351.set_correction(SI5351calib, SI5351_PLL_INPUT_XO);
    calculateAndDisplaySignalStrength();
    tft.fillRect(5, 100, 333, 16, TFT_GREY);
    tft.setCursor(5, 100);
    tft.printf("SI5351 calibration.:%ld ", SI5351calib);
    delay(50);
  }
  preferences.putLong("calib", SI5351calib);
  encLockedtoSynth = true;
}

//##########################################################################################################################//

void setVol() {  //sets SI4732 volume. For FFT to work the volume should be around 50

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
  while (digitalRead(ENCODER_BUTTON) == LOW)  // wait so that it does not jump into the setStep function
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
      FREQ++;
      si4735.setFrequencyUp();
    }
    if (cclw) {
      FREQ--;
      si4735.setFrequencyDown();
    }

    clearNotification();
    tft.setCursor(5, 75);
    SI4735TUNED_FREQ = si4735.getCurrentFrequency();
    tft.printf("IF: %d KHz", SI4735TUNED_FREQ);
    clw = false;
    cclw = false;
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

  while (digitalRead(ENCODER_BUTTON) == LOW)  // wait so that it does not jump into the setStep function
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
