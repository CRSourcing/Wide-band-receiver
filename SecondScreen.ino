
void SecScreen() {

  clearStatusBar();
  if (timeSet) {
    timeOld = 0;
    tft.fillRect(340, 6, 116, 16, TFT_BLACK);  // overwrite last time
  }
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
    const int x;
    const int y;
    const char* label;
  };


  const Button buttons[] PROGMEM = {
    { 20, 134, "Station" },
    { 20, 152, "Scan" },
    { 100, 132, "" },
    { 100, 152, "" },
    { 185, 132, "Web" },
    { 185, 152, "Tools" },
    { 270, 132, "Pass" },
    { 270, 155, "band" },
    { 18, 190, "Assign" },
    { 18, 210, "VFO" },
    { 100, 190, "Search" },
    { 100, 210, "EiBi" },
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



  if (modType != AM && modType != NBFM) {  // AGC only working in AM
    etft.setCursor(273, 200);
    etft.setTextColor(TFT_DARKDARKGREY);
    etft.print(F("Attn."));
  }

  etft.setTextColor(TFT_SKYBLUE);
  etft.setCursor(180, 255),
    etft.print(F("Storage"));
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(20, 254);
  etft.print(F("Config"));
  etft.setTextColor(TFT_YELLOW);

#ifdef SHOW_DEBUG_UTILITIES

  etft.setCursor(100, 255);
  etft.print(F("Debug"));
  etft.setTextColor(textColor);
#endif

  tDoublePress();
}
//##########################################################################################################################//

void readSecBtns() {

  if (!pressed) return;

  int buttonID = getButtonID();
  if (!buttonID) {
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
      tRel();
      break;
    case 23:
      drawIBtns();
      readIBtns();
      tRel();
      break;
    case 24:
      shiftPassBand();
      tRel();
      break;
    case 31:
      vfoMenu();
      break;
    case 32:
      showEiBiStations(FREQ);
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
  tft.print(F("Use encoder to change Attn."));

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
  if (modType == AM || modType == NBFM)
    tft.printf(AGCDIS == false ? "AGC:ON" : "Attn:%d ", AGCIDX);
  else
    tft.print(F("AGC:OFF"));
  tft.setTextColor(textColor);
}
//##########################################################################################################################//

void setBFO() {  // uses seperate BFO offsets for USB/LSB Hi and Low injection. A total of 6 BFO offsets is needed.

  int offset = 0, oldOffset = 0;

  encLockedtoSynth = false;

  tft.setCursor(10, 75);
  tft.print(F("Use encoder to change BFO."));
  tft.setCursor(10, 93);
  tft.print(F("Press encoder to save."));
  delay(1000);


  // using 4 BFO's for SSB:


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


  if (modType == USB && singleConversionMode)
    offset = preferences.putInt("B1", offset);
  if (modType == USB && !singleConversionMode)
    offset = preferences.putInt("B2", offset);

  if (modType == LSB && singleConversionMode)
    offset = preferences.putInt("B3", offset);
  if (modType == LSB && !singleConversionMode)
    offset = preferences.putInt("B4", offset);




  if (modType == SYNC)
    preferences.putInt("SYNCBfoOffset", offset);

  if (modType == CW)
    preferences.putInt("CWBfoOffset", offset);

  encLockedtoSynth = true;
  clearStatusBar();
}

//##########################################################################################################################//


void selectButtonStyle() {  // selects btw. different bitmaps for the buttons

  tRel();
  tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  for (int i = 0; i < 8; i++) {


    if (i < 4)
      tft.pushImage(8 + i * 83, 178, SPRITEBTN_WIDTH, SPRITEBTN_HEIGHT, (uint16_t*)buttonImages[i]);  // Use the corresponding button image
    else

      tft.pushImage(8 + (i - 4) * 83, 235, SPRITEBTN_WIDTH, SPRITEBTN_HEIGHT, (uint16_t*)buttonImages[i]);
  }
  tPress();

  getButtonID();
  if (row > 4 || column > 4)
    return;
  buttonSelected = (column - 1) + 4 * (row - 3);
  preferences.putInt("sprite", buttonSelected);  // write selection to flash
}

//##########################################################################################################################//

void clearStatusBar() {

  tft.fillRect(0, 294, 479, 26, TFT_BLACK);
}

//##########################################################################################################################//
void clearNotification() {

  tft.fillRect(5, 75, 330, 16, TFT_BLACK);
}


//##########################################################################################################################//
// EiBi list viewer
void showEiBiStations(uint32_t FREQ) {

  int timeNowInt = 9999;  // HHMM, init out of range

  float targetFreq = (float)FREQ_TO_KHZ;
  if (targetFreq > 30000)
    return;

  if (timeSet) {
    getLocalTime(&timeinfo);
    timeNowInt = timeinfo.tm_hour * 100 + timeinfo.tm_min;  // struct to HHMM
  }


  File file = LittleFS.open("/sked-b25.lst", FILE_READ);
  if (!file) {
    Serial_println("Failed to open file");
    return;
  }


  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(0, 0);
  tft.printf("Listing stations at %5.1f KHz...\n", targetFreq);
  tft.println();



  bool found = false;
  bool duplicate = false;

  String prevF4 = "";  // store prev station name to check for duplicates

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    // Each row must have 10 semicolons!
    int semicolons = 0;
    for (int i = 0; i < line.length(); i++) {
      if (line[i] == ';') semicolons++;
    }
    if (semicolons != 10) continue;

    // Extract freq
    int sepIndex = line.indexOf(';');
    if (sepIndex == -1) continue;
    float freq = line.substring(0, sepIndex).toFloat();

    // Small frequency error allowed
    if (abs(freq - targetFreq) < 0.5) {
      found = true;

      // Extract first 5 fields
      String fields[5];
      int start = 0;
      for (int f = 0; f < 5; f++) {
        int idx = line.indexOf(';', start);
        if (idx == -1) {
          fields[f] = line.substring(start);
          break;
        } else {
          fields[f] = line.substring(start, idx);
          start = idx + 1;
        }
      }


      //example,  fields[1] = "1600-1700"
      String range = fields[1];

      // find seperator dash
      int dashIdx = range.indexOf('-');

      // Extract bot substrings
      String startStr = range.substring(0, dashIdx);
      String endStr = range.substring(dashIdx + 1);


      int startT = startStr.toInt();  // 1600
      int endT = endStr.toInt();      //  1700
      bool active = false;




      if ((startT <= endT && timeNowInt >= startT && timeNowInt <= endT) || ((startT > endT && (timeNowInt >= startT || timeNowInt <= endT)) && timeNowInt != 9999))  // handle midnight crossing
        active = true;                                                                                                                                                // we have a match

      // Compare station name with prev station name
      if (fields[4] == prevF4 && !active) {
        if (!duplicate) {  // flag the duplicate
          tft.setTextColor(TFT_DARKGREY);
          int gy = tft.getCursorY();
          gy -= 32;  // 2 lines up to overwrite the 1st found time
          tft.fillRect(0, gy, 110, 16, TFT_BLACK);
          tft.setCursor(0, gy);
          tft.print("Multiple");  // multiple times found
          tft.println();
          tft.println();
        }

        duplicate = true;
        continue;  // skip new line to avoid cluttering
      }

      else
        duplicate = false;

      prevF4 = fields[4];


      // Print
      uint16_t cls[5] = { TFT_DARKGREY, TFT_ORANGE, TFT_CYAN, TFT_WHITE, TFT_WHITE };
      const uint8_t colWidths[4] = { 10, 6, 4, 19 };

      if (active) {
        cls[0] = TFT_GREEN;
        cls[3] = TFT_GREEN;
      }

      for (int f = 1; f < 5; f++) {
        tft.setTextColor(cls[f - 1]);
        String field = fields[f];

        if (field.length() > colWidths[f - 1]) {
          field = field.substring(0, colWidths[f - 1]);  // cut length
        }
        tft.printf("%-*s", colWidths[f - 1], field.c_str());
      }
      tft.println();
      tft.println();
    }
  }


  file.close();

  tft.fillRect(0, 0, 480, 16, TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_DARKGREY);
  tft.print(F("UTC - UTC "));
  tft.setTextColor(TFT_ORANGE);
  tft.print(F("Days "));
  tft.setTextColor(TFT_CYAN);
  tft.print(F("Cntry "));
  tft.setTextColor(TFT_WHITE);
  tft.print(F("Station name "));


  if (!found) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_RED);
    tft.println("No stations found.\n");
    delay(1000);
    rebuildMainScreen(false);
    return;
  }

  while (true) {

    uint16_t z = tft.getTouchRawZ();

    if ((z > 300) || clw || cclw || digitalRead(ENCODER_BUTTON) == LOW) {  // touch, encoder moved or pressed
      rebuildMainScreen(false);
      return;
    }
  }
}
