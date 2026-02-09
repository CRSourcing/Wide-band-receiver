
void SecScreen() {

  clearStatusBar();
  redrawMainScreen = true;
  drawSecBtns();
  readSecBtns();
}

//##########################################################################################################################//

void drawSecBtns() {


  if (!altStyle)  // restore background
    tft.fillRect(2, 61, 337, 228, TFT_BLACK);

  else
    drawButton(2, 60, 337, 229, TFT_NAVY, TFT_DARKGREY);  // plain buttons background
  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);


  struct Button {
    int x;
    int y;
    const char* label;
  };


  Button buttons[] = {
    { 20, 134, "Station" },
    { 20, 152, "Scan" },
    { 100, 132, "Touch" },
    { 100, 152, "Tune" },
    { 185, 132, "Web" },
    { 185, 152, "Tools" },
    { 270, 132, "Draw" },
    { 270, 155, "3D" },
    { 18, 190, "Assign" },
    { 18, 210, "VFO" },
    { 100, 190, "" },
    { 100, 210, "" },
    { 190, 200, "BFO" },
    { 182, 208, " " },
    { 273, 200, "Attn." },
    { 265, 245, "Audio" },
    { 270, 265, "WF" },
  };

  etft.setTTFFont(Arial_14);
  etft.setTextColor(textColor);

  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].label);
  }



 if (modType != AM && modType != NBFM){  // AGC only working in AM
    etft.setCursor(273, 200);
    etft.setTextColor(TFT_DARKDARKGREY);
    etft.print("Attn.");
  }

  etft.setTextColor(TFT_SKYBLUE);
  etft.setCursor(180, 255),
    etft.print("Storage");
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(20, 254);
  etft.print("Config");
  etft.setTextColor(TFT_YELLOW);

#ifdef SHOW_DEBUG_UTILITIES

  etft.setCursor(100, 255);
  etft.print("Debug");
  etft.setTextColor(textColor);
#endif

  tDoublePress();
}
//##########################################################################################################################//

void readSecBtns() {

  if (!pressed) return;

  int buttonID = getButtonID();
  

  if (row < 2 || row > 4 || column > 4 || tx > 345){
    tRel();
    tx = ty = pressed = 0;
    return;  // outside of area
  } 

  tft.setTextColor(textColor);
  switch (buttonID) {


    case 21:
      slowScan = true;
      SlowStationScan();  
      tRel();
      tRel();
      tx = ty = pressed = 0;
      rebuildMainScreen(0);
      return;
    case 22:
      drawTrapezoid = false;
      touchTune = !touchTune;
      tRel();
      break;
    case 23:
      drawIBtns();
      readIBtns();
      tRel();
      break;
    case 24:
      drawTrapezoid = true;
      touchTune = !touchTune;
      tRel();
      break;
    case 31:
      vfoMenu();
      break;
    case 32:
      break;
    case 33:
      setBFO();
      tRel();
      break;
    case 34:
      setAGCMode(0);
      tRel();
      break;
    case 41:
      ConfigScreen1();
      break;
    case 42:
#ifdef SHOW_DEBUG_UTILITIES
      DebugScreen();
#endif
      break;
    case 43:
      SDCard();
      break;
    case 44:
      audioFreqAnalyzer();  // located in FFT.ino
      tRel();
      break;
    default:
      tx = ty = pressed = 0;
      return;
  }

  tRel();
}


//##########################################################################################################################//

void setAGCMode(int mode) {
  // AGCDIS: AGC enabled (0) or disabled (1)
  // AGCIDX: AGC Index (0 = max gain, 1-36 = intermediate, >36 = min gain)
  // mode 0 = button control, mode -1 or 1 = icon control

  if (modType != AM && modType != NBFM) {
    return;
  }

  if (mode) {
    // Handle icon control for AGC adjustment
    while (pressed) {
      pressed = tft.getTouch(&tx, &ty);
      if (mode == 1) {
        AGCIDX++;
      } else if (mode == -1 && AGCIDX) {
        AGCIDX--;
      }

      if (AGCIDX > 36) {
        AGCIDX = 36;
      }

      AGCDIS = (AGCIDX != 0);  // Disable AGC if AGCIDX is 0
      writeAGCMode();
      printAGC();
      delay(50);
    }
    return;
  }

  encLockedtoSynth = false;

  clearNotification();
  tft.setCursor(5, 75);
  tft.print("Use encoder to change Attn.");

  while (digitalRead(ENCODER_BUTTON) == HIGH) {
    delay(50);

    if (clw) {
      AGCIDX++;
    } else if (cclw) {
      AGCIDX--;
    }

    if (AGCIDX > 36) {
      AGCIDX = 0;
    }

    AGCDIS = (AGCIDX != 0);  // Disable AGC if AGCIDX is 0, enable otherwise
    writeAGCMode();

    if (clw != cclw) {
      printAGC();
    }

    clw = false;
    cclw = false;
  }

  // Prevent jumping into setStep function
  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;

  encLockedtoSynth = true;
  clearNotification();
  tx = ty = pressed = 0;
}


//##########################################################################################################################//

void writeAGCMode() {

  if (modType == AM || modType == NBFM)
    si4735.setAutomaticGainControl(AGCDIS, AGCIDX);
  if (modType == LSB || modType == USB || modType == SYNC || modType == CW) {
    si4735.setSsbAgcOverrite(1, AGCIDX);  // disable AGC to eliminate SSB humming noise
    printAGC();
  }
}
//##########################################################################################################################//


void printAGC() {

  tft.setTextColor(TFT_GREEN);
  if (altStyle)
    tft.fillRoundRect(225, 95, 100, 22, 10, TFT_BLUE);

  else
    tft.pushImage(225, 92, 102, 26, (uint16_t*)Oval102);
    

  tft.setCursor(235, 98);
  if (modType == AM || modType ==NBFM)
    tft.printf(AGCDIS == false ? "AGC:ON" : "Attn:%d ", AGCIDX);
  else
    tft.print("AGC:OFF");
  tft.setTextColor(textColor);
}
//##########################################################################################################################//

void setBFO() {  // uses seperate BFO offsets for USB/LSB Hi and Low injection. A total of 6 BFO offsets is needed. 

  int offset = 0, oldOffset = 0;

  encLockedtoSynth = false;

  tft.setCursor(10, 75);
  tft.print("Use encoder to change BFO.");
  tft.setCursor(10, 93);
  tft.print("Press encoder to save.");
  delay(1000);


  // using 4 BFO's for SSB:


  if (modType == USB && LOAboveRF)
    offset = preferences.getInt("B1", 0);
  if (modType == USB && !LOAboveRF)
    offset = preferences.getInt("B2", 0);

  if (modType == LSB && LOAboveRF)
    offset = preferences.getInt("B3", 0);
  if (modType == LSB && !LOAboveRF)
    offset = preferences.getInt("B4", 0);



  if (modType == SYNC)
    offset = preferences.getInt("SYNCBfoOffset", 0);
  if (modType == CW)
    offset = preferences.getInt("CWBfoOffset", 0);

  while (digitalRead(ENCODER_BUTTON) == HIGH) {

    if (oldOffset != offset) {
      tft.fillRect(10, 75, 320, 17, TFT_BLACK);
      tft.setCursor(10, 75);
      tft.printf("BFO Offset: %d", offset);



      clearStatusBar();
      tft.setCursor(5, 300);
      tft.printf("B1:%ld B2:%ld B3:%ld B4:%ld", preferences.getInt("B1", 0), preferences.getInt("B2", 0), preferences.getInt("B3", 0), preferences.getInt("B4", 0));
      oldOffset = offset;
    }

    delay(50);
    if (clw)
      offset += 25;
    if (cclw)
      offset -= 25;

    si4735.setSSBBfo(offset);

    clw = false;
    cclw = false;
  }

  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;


  if (modType == USB && LOAboveRF)
    offset = preferences.putInt("B1", offset);
  if (modType == USB && !LOAboveRF)
    offset = preferences.putInt("B2", offset);

  if (modType == LSB && LOAboveRF)
    offset = preferences.putInt("B3", offset);
  if (modType == LSB && !LOAboveRF)
    offset = preferences.putInt("B4", offset);




  if (modType == SYNC)
    preferences.putInt("SYNCBfoOffset", offset);

  if (modType == CW)
    preferences.putInt("CWBfoOffset", offset);

  encLockedtoSynth = true;
  clearStatusBar();
}

//##########################################################################################################################//


void selectButtonStyle() {  // selects btw. different sprites for the buttons
 
  tRel();
  tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  for (int i = 0; i < 8; i++) {


    if (i < 4)
      tft.pushImage(8 + i * 83, 178, SPRITEBTN_WIDTH, SPRITEBTN_HEIGHT, (uint16_t*)buttonImages[i]);  // Use the corresponding button image 
    else

    tft.pushImage(8 + (i - 4) * 83, 235, SPRITEBTN_WIDTH, SPRITEBTN_HEIGHT, (uint16_t*)buttonImages[i]);
  }
  tPress();
  column = 1 + (tx / HorSpacing);  // get row and column
  row = 1 + ((ty - 20) / vTouchSpacing);
  if (row > 4 || column > 4)
    return;
  buttonSelected = (column - 1) + 4 * (row - 3);
  
  Serial.printf("buttonSelected %d, row %d column %d\n", buttonSelected, row, column);
  preferences.putInt("sprite", buttonSelected);  // write selection to EEPROM
}

//##########################################################################################################################//

void clearStatusBar() {

  tft.fillRect(0, 294, 479, 26, TFT_BLACK);
}

//##########################################################################################################################//
void clearNotification() {

  tft.fillRect(5, 75, 330, 16, TFT_BLACK);
}
