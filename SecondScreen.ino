
void SecScreen() {

  clearStatusBar();
  if (timeSet) {
    timeOld = 0;
    tft.fillRect(340, 6, 116, 16, TFT_BLACK);  // overwrite last time
  }
  redrawMainScreen = true;
  drawSecBtns();
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
    const char *label;
  };


  const Button buttons[] PROGMEM = {
    { 20, 134, "Station" },
    { 20, 152, "Scan" },
    { 100, 132, "Audio" },
    { 100, 152, "DSP" },
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



  while (tft.getTouchRawZ() < 300) {  // run a small loop without display updates for tuning without display noise

    if (modType != WBFM && (clw || cclw)) {
      if (clw) {
        FREQ += STEP;
        clw = false;
      } else if (cclw) {
        FREQ -= STEP;
        cclw = false;
      }
      FREQCheck();        //check whether within FREQ range
      displayFREQ(FREQ);  // display new FREQ
      setFreq();
    }
    delay(20);
  }

  if (pressSound)
    sineTone(440, 20);  // missing here

  tRel();
  tft.setTextSize(2);
  tft.fillRect(10, 70, 325, 40, TFT_BLACK);
  etft.setTTFFont(Arial_14);
  readSecBtns();
}
//##########################################################################################################################//

void readSecBtns() {

  tft.fillRect(10, 70, 325, 40, TFT_BLACK);

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
      rebuildMainScreen(false);
      return;
    case 22:
      dspAudio();
      tRel();
      rebuildMainScreen(false);
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
      showEiBiStations(1);  // 1 = regular mode
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
    tft.pushImage(225, 92, 102, 26, (uint16_t *)Oval102);


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
      tft.pushImage(8 + i * 83, 178, SPRITEBTN_WIDTH, SPRITEBTN_HEIGHT, (uint16_t *)buttonImages[i]);  // Use the corresponding button image
    else

      tft.pushImage(8 + (i - 4) * 83, 235, SPRITEBTN_WIDTH, SPRITEBTN_HEIGHT, (uint16_t *)buttonImages[i]);
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
void showEiBiStations(int mode) {

  int timeNowInt = 9999;  // HHMM, init out of range

  float targetFreq = (float)FREQ_TO_KHZ;
  if (targetFreq > 30000)
    return;

  if (timeSet) {
    getLocalTime(&timeinfo);
    timeNowInt = timeinfo.tm_hour * 100 + timeinfo.tm_min;  //  to HHMM
  }


  File file = LittleFS.open("/eibi.lst", FILE_READ);
  if (!file) {
    Serial_println("Failed to open file");
    return;
  }


  if (mode == 1) {  // regular mode
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(0, 0);
    tft.printf("Loading stations at %5.1f KHz...\n", targetFreq);
    tft.println();
  }

  int xPosEibi = 3;
  int yPosEibi = 160;
  bool fillEibi = false;
  bool found = false;
  bool duplicate = false;
  float freq = 0;
  String line;

  String prevF4 = "";  // store prev station name to check for duplicates


  // 2 stage search, first a binary search, then when close a forward search

  uint32_t low = 0;
  uint32_t high = file.size();

  while (high - low > 512) {  // stop binary when window is small, around 5 lines
    uint32_t mid = (low + high) / 2;
    file.seek(mid);

    // Align to start of next line
    file.readStringUntil('\n');

    // read a line
    String line = file.readStringUntil('\n');
    if (line.length() == 0) break;

    // Extract freq
    int sepPt = line.indexOf(';');
    if (sepPt == -1) continue;
    freq = line.substring(0, sepPt).toFloat();

    if (freq < targetFreq) {
      low = mid;
    } else {
      low = mid - 20000;  // correct overshot
      high = mid;
    }
  }

  file.seek(low);

  //Serial.printf("binsearch: %6.0f for FREQ:%ld\n", freq, FREQ / 1000);

  while (file.available()) {  //we're close, start 2nd stage, search, extract and print info

    if (!fillEibi && mode == 2) {
      tft.fillRect(3, 165, 334, 100, TFT_BLACK);  // make room for Eibi entries, y =160 - y =165 is reserved for cursor
      fillEibi = true;
    }

    line = file.readStringUntil('\n');
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
    freq = line.substring(0, sepIndex).toFloat();

    // Small frequency error allowed
    if (abs(freq - targetFreq) <= 0.5f) {
      found = true;

      // Extract first 5 fields,
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
        if (!duplicate && mode == 1) {  // flag the duplicate
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
      

      String days = fields[2];
      dashIdx = days.indexOf('-');

      // Extract start and end day
     String startDay = days.substring(0, dashIdx);
     String endDay = days.substring(dashIdx + 1);
     

  
     int startD = 0;

      if (startDay == "Mo")
        startD = 1;
      else if (startDay == "Tu")
        startD = 2;
      else if (startDay=="We")
        startD = 3;
      else if (startDay== "Th")
        startD = 4;
      else if (startDay == "Fr")
        startD = 5;
      else if (startDay =="Sa")
        startD = 6;
      else if (startDay== "Su")
        startD = 7;   

      int endD = 0;

         if (endDay == "Mo")
        endD = 1;
      else if (endDay == "Tu")
        endD = 2;
      else if (endDay == "We")
        endD = 3;
      else if (endDay == "Th")
        endD = 4;
      else if (endDay == "Fr")
        endD = 5;
      else if (endDay == "Sa")
        endD = 6;
      else if (endDay =="Su")
        endD= 7;

      
    int dayOfWeek = timeinfo.tm_wday;

       Serial.println(startDay + "  " + endDay);
 

    Serial.printf(" %s  startday:%d  endday:%d  today %d\n", fields[4].c_str(), startD, endD, dayOfWeek);


    if (startD < dayOfWeek  && endD < dayOfWeek && startD && endD) // 0  when fields are not populated
       continue; 
      
    if (startD > dayOfWeek  && endD > dayOfWeek)
       continue;   

    if (startD > dayOfWeek  && endD < dayOfWeek) // overrun
       continue; 

    if (endD > dayOfWeek  && startD < dayOfWeek) // overrun
       continue;     
      
    if(startD && !endD && startD != dayOfWeek ) // single day
       continue;   
     
      prevF4 = fields[4];

      if (mode == 1) { // full screen
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



      else if (mode == 2) {  // embedded in waterfall
        if (active) {
          Serial.printf("%s\n", fields[4].c_str());
          if (yPosEibi < 260) {  // room for max 4 entries
            tft.setCursor(xPosEibi + 10, yPosEibi + 10);
            tft.printf("%s", fields[4].c_str());
          }
          yPosEibi += 25;
        }
      }
    }  // endif freq matched

    if (freq > targetFreq + 0.5f) {  // above target frequency

      break;
    }
  }

  file.close();


  if (mode == 1) {
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
    sineTone(880, 100);
    sineTone(1000, 100);


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
}


//##########################################################################################################################//
// digital audio processor
//##########################################################################################################################//

bool invertComb = false;
bool usePeakF = false;  // noch mode
bool nbMode = false;    // noiseblanker 1 or 2
bool nrMode = false;    // false = freq domain, true = time domain noise reductor
void drawSwitch(tftSwitch sw);
void drawSlider(tftSlider s);
bool updateSlider(tftSlider &s, uint16_t tx, uint16_t ty);
bool updateSwitch(tftSwitch &sw, uint16_t tx, uint16_t ty);
// prototypes needed

//##########################################################################################################################//
void drawSwitch(tftSwitch sw) {
  const int W = 50;
  const int H = 20;
  uint16_t bg = sw.state ? TFT_BLUE : TFT_DARKDARKGREY;


  tft.fillRoundRect(sw.x, sw.y, W, H, H / 2, bg);

  tft.drawRoundRect(sw.x, sw.y, W, H, H / 2, TFT_GREEN);

  // knob pos
  int knobX = sw.state ? (sw.x + W - H / 2) : (sw.x + H / 2);

  tft.fillCircle(knobX, sw.y + H / 2, 8, TFT_WHITE);
}
//##########################################################################################################################//
bool updateSwitch(tftSwitch &sw,
                  uint16_t tx,
                  uint16_t ty) {
  const int W = 50;
  const int H = 20;

  // touch inside?
  if (tx < sw.x || tx > (sw.x + W))
    return sw.state;

  if (ty < sw.y || ty > (sw.y + H))
    return sw.state;

  // left/right = false/true
  if (tx < (sw.x + W / 2))
    sw.state = false;
  else
    sw.state = true;

  drawSwitch(sw);

  return sw.state;
}

//##########################################################################################################################//

void drawSlider(tftSlider s) {

  tft.drawRoundRect(s.x - 12, s.y, s.w + 24, s.h, 4, TFT_GREEN);  // frame

  int fillW = ((s.w - 4) * s.value) / 100;

  // active
  tft.fillRect(
    s.x - 8,
    s.y + 1,
    fillW + 8,
    s.h - 2,
    TFT_BLUE);

  // inactive
  tft.fillRect(
    s.x + 2 + fillW,
    s.y + 1,
    s.w - fillW + 6,
    s.h - 2,
    TFT_DARKDARKGREY);

  int knobX = s.x + (s.value * s.w) / 100;
  int knobColor = TFT_GREEN;

  if (!s.value)
    knobColor = TFT_GREY;


  tft.fillCircle(
    knobX,
    s.y + s.h / 2,
    s.h / 2 - 3,
    knobColor);
}

//##########################################################################################################################//

bool updateSlider(tftSlider &s, uint16_t tx, uint16_t ty) {
  if (tx < s.x || tx > (s.x + s.w))
    return false;

  if (ty < s.y || ty > (s.y + s.h))
    return false;

  s.value = ((tx - s.x) * 100) / s.w;

  if (s.value < 2) s.value = 0;
  if (s.value > 100) s.value = 100;

  drawSlider(s);

  return true;
}

//##########################################################################################################################//


void drawSliders() {
  tft.fillRect(3, 50, 335, 240, TFT_BLACK);
  drawSlider(pulseSlider);
  drawSlider(notchSlider);
  drawSlider(combSlider);
  drawSlider(muteSlider);
  drawSlider(nrSlider);
  drawSwitch(combSwitch);
  drawSwitch(npSwitch);
  drawSwitch(nbSwitch);
  drawSwitch(nrSwitch);
  tft.setCursor(10, 50);
  tft.print("Pulse Blanker:");
  tft.setCursor(10, 100);
  tft.print("Notch:");
  tft.setCursor(10, 150);
  tft.print("Comb:");
  tft.setCursor(10, 200);
  tft.print("Softmute:");
  tft.setCursor(10, 250);
  tft.print("Noise reduction:");
  drawButton(350, 205, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
  tft.setCursor(365, 225);
  tft.print("Back");
}

//##########################################################################################################################//


void dspAudio() {

  si4735.setHardwareAudioMute(true);
  tft.setTextColor(TFT_YELLOW);
  drawSliders();
  audioBufSize = 512;
  printSliderValues();
  bool leave = false;

  while (!leave) {  // break if back button touched

    //start filling buffer
    bufferFilled = false;

    if (!nrMode)  // FFT mode
      fftNR();    // basic noise reductor in freq domain, RvReal and RvImag are filled, perform FFT and IFFT. Causes clicks

    for (int i = 0; i < audioBufSize; i++) {

      int16_t s = (int16_t)frame[i] - 128;  // move to signed


      if (pulseSlider.value) {
        if (!nbMode)
          s = noiseBlanker(s);
        else
          s = noiseBlanker2(s);
      }

      if (notchSlider.value) {
        if (!usePeakF)
          s = notchF(s);
        else
          s = peakF(s);
      }


      if (combSlider.value)
        s = combFilter(s, 20 * combSlider.value);

      if (muteSlider.value)
        s = softMute(s, muteSlider.value / 3);


      if ((nrSlider.value) && nrMode)  // time domain mode
        s = noiseReduction(s, nrSlider.value);

      if ((nrSlider.value) && !nrMode) {  // frequency domain mode
        RvReal[i] = (float)s;
        RvImag[i] = 0;
      }

      else
        playBuffer[i] = (uint8_t)s + 128;  // back to unsigned
    }



    while (!bufferFilled) {
      if (tft.getTouchRawZ() > 300) {          // we have a touch
        tft.getTouch(&tx, &ty);                // get_Touch() would glitch here
        if (tx > 358 && ty > 205 && ty < 245)  // Back touched
          leave = true;
        processSliderTouch(tx, ty);
        tx = 0;
        ty = 0;
      }


      if (clw || cclw) {  // allow tuning
        if (clw) {
          FREQ += STEP;
          clw = false;
        } else if (cclw) {
          FREQ -= STEP;
          cclw = false;
        }
        displayFREQ(FREQ);
        setFreq();
      }
    }

    // --- Start playback via ISR ---
    playIndex = 0;
    bufferPlaying = true;
  }

  si4735.setHardwareAudioMute(false);
  bufferPlaying = false;
  audioBufSize = 4096;  // reset to default
  tft.setTextColor(textColor);
}



//##########################################################################################################################//

void processSliderTouch(uint16_t tx, uint16_t ty) {


  if (tx >= pulseSlider.x - 5 && tx <= pulseSlider.x + pulseSlider.w + 5 && ty >= pulseSlider.y - 10 && ty <= pulseSlider.y + pulseSlider.h) {
    updateSlider(pulseSlider, tx, ty);

    tft.fillRect(185, 50, 60, 16, TFT_BLACK);

    tft.setCursor(185, 50);
    tft.printf("%d", pulseSlider.value);

    return;
  }



  else if (tx > 350 && ty >= pulseSlider.y && ty <= pulseSlider.y + pulseSlider.h)

    nbMode = updateSwitch(nbSwitch, tx, ty);  //noise blanker 1 or 2


  else if (tx >= notchSlider.x - 5 && tx <= notchSlider.x + notchSlider.w + 5 && ty >= notchSlider.y - 10 && ty <= notchSlider.y + notchSlider.h) {
    updateSlider(notchSlider, tx, ty);

    tft.fillRect(85, 100, 90, 16, TFT_BLACK);

    tft.setCursor(85, 100);
    tft.printf("%d Hz", notchSlider.value * 20);

    return;
  }


  else if (tx > 350 && ty >= notchSlider.y && ty <= notchSlider.y + notchSlider.h) {

    usePeakF = updateSwitch(npSwitch, tx, ty);
    tft.fillRect(10, 100, 70, 16, TFT_BLACK);
    tft.setCursor(10, 100);
    if (usePeakF)
      tft.print("Peak:");
    else
      tft.print("Notch:");
    return;
  }



  else if ((tx >= combSlider.x - 5 && tx <= combSlider.x + combSlider.w + 5 && ty >= combSlider.y - 10 && ty <= combSlider.y + combSlider.h)
           || (tx > 350 && ty >= combSlider.y && ty <= combSlider.y + combSlider.h)) {



    if (tx > 350 && ty >= combSlider.y && ty <= combSlider.y + combSlider.h) {

      invertComb = updateSwitch(combSwitch, tx, ty);

      tft.fillRect(10, 150, 325, 16, TFT_BLACK);
      tft.setCursor(10, 150);
      if (invertComb)
        tft.print("Comb inverse:");
      else
        tft.print("Comb normal:");
      return;
    }

    updateSlider(combSlider, tx, ty);

    tft.fillRect(180, 150, 90, 16, TFT_BLACK);
    tft.setCursor(180, 150);
    tft.printf("%d Hz", combSlider.value * 20);

    return;
  }


  else if (tx >= muteSlider.x - 5 && tx <= muteSlider.x + muteSlider.w + 5 && ty >= muteSlider.y - 10 && ty <= muteSlider.y + muteSlider.h) {
    updateSlider(muteSlider, tx, ty);

    tft.fillRect(120, 200, 60, 16, TFT_BLACK);

    tft.setCursor(120, 200);
    tft.printf("%d", muteSlider.value);

    return;
  }

  else if (tx > 350 && ty >= nrSlider.y && ty <= nrSlider.y + nrSlider.h)

    nrMode = updateSwitch(nrSwitch, tx, ty);  //noise reduction 1 or 2

  else if (tx >= nrSlider.x - 5 && tx <= nrSlider.x + nrSlider.w + 5 && ty >= nrSlider.y - 10 && ty <= nrSlider.y + nrSlider.h) {
    updateSlider(nrSlider, tx, ty);

    tft.fillRect(220, 250, 90, 16, TFT_BLACK);

    tft.setCursor(220, 250);

    tft.printf("%d", nrSlider.value);
    return;
  }
}


//##########################################################################################################################//

void printSliderValues() {

  tft.setCursor(345, 65);
  tft.setTextColor(TFT_YELLOW);

  tft.fillRect(185, 50, 60, 16, TFT_BLACK);
  tft.setCursor(185, 50);
  tft.printf("%d", pulseSlider.value);


  tft.fillRect(85, 100, 90, 16, TFT_BLACK);
  tft.setCursor(85, 100);
  tft.printf("%d Hz", notchSlider.value * 20);

  tft.fillRect(10, 100, 70, 16, TFT_BLACK);
  tft.setCursor(10, 100);
  if (usePeakF)
    tft.print("Peak:");
  else
    tft.print("Notch:");


  tft.fillRect(10, 150, 325, 16, TFT_BLACK);
  tft.setCursor(10, 150);
  if (invertComb)
    tft.print("Comb inverse:");
  else
    tft.print("Comb normal:");


  tft.fillRect(180, 150, 90, 16, TFT_BLACK);
  tft.setCursor(180, 150);
  tft.printf("%d Hz", combSlider.value * 20);

  tft.fillRect(120, 200, 60, 16, TFT_BLACK);

  tft.setCursor(120, 200);
  tft.printf("%d", muteSlider.value);

  tft.fillRect(220, 250, 90, 16, TFT_BLACK);

  tft.setCursor(220, 250);

  tft.printf("%d", nrSlider.value);
}




//##########################################################################################################################//

int16_t noiseBlanker(int16_t s)  // adaptive noise blanker with audio buffer to repair samples
{
  const int BUF = 32;
  static int16_t buf[BUF];
  static uint8_t wr = 0;
  const int delay = 12;


  delayMicroseconds(20);
  buf[wr] = s;

  // delayed output pointer
  int rd = wr - delay;

  if (rd < 0)
    rd += BUF;

  // shift prev. samples

  int p1 = wr - 1;
  if (p1 < 0) p1 += BUF;

  int p2 = wr - 2;
  if (p2 < 0) p2 += BUF;

  int16_t last = buf[p1];



  // calculate trigger
  static float avgDelta = 10;

  int16_t delta = s - last;

  int16_t absDelta = abs(delta);

  // track normal signal changes
  avgDelta += 0.01f * ((float)absDelta - avgDelta);

  int16_t trigger = avgDelta + (101 - pulseSlider.value / 3);

  trigger -= 75;  // shift  active area

  // pulse detected
  if (absDelta > trigger) {

    //calculate width

    int pulseLen = 1;

    while (pulseLen < 12) {

      int idx = (wr + pulseLen) % BUF;

      int idxPrev = (idx - 1 + BUF) % BUF;

      int16_t d = abs(buf[idx] - buf[idxPrev]);

      // still impulsive?
      if (d > (trigger >> 1)) pulseLen++;

      else
        break;
    }


    // start and stop of noise
    int startIdx = p1;

    int endIdx = (wr + pulseLen) % BUF;

    int16_t y1 = buf[startIdx];

    int16_t y2 = buf[endIdx];

    //interpolate
    for (int i = 0; i < pulseLen; i++) {

      int idx = (wr + i) % BUF;

      float t = (float)(i + 1) / (pulseLen + 1);

      buf[idx] = y1 + (y2 - y1) * t;
    }
  }

  //output

  int16_t out = 1.5f * buf[rd];

  // increase bufptr
  wr++;

  if (wr >= BUF)
    wr = 0;

  return out;
}

//##########################################################################################################################//

int16_t noiseBlanker2(int16_t s)  // adaptive noise blanker with audio buffer to repair samples
{
  const int BUF = 32;
  static int16_t buf[BUF];
  static uint8_t wr = 0;
  const int delay = 12;

  delayMicroseconds(20);
  buf[wr] = s;

  int rd = wr - delay;
  if (rd < 0) rd += BUF;

  int p1 = wr - 1;
  if (p1 < 0) p1 += BUF;

  int16_t last = buf[p1];

  // --- adaptive trigger calculation ---
  static float avgDelta = 10;
  int16_t delta = s - last;
  int16_t absDelta = abs(delta);
  avgDelta += 0.01f * ((float)absDelta - avgDelta);

  int16_t trigger = avgDelta + (101 - pulseSlider.value / 3);
  trigger -= 75;

  // --- noise blanker ---
  if (absDelta > trigger) {
    int pulseLen = 1;
    while (pulseLen < 12) {
      int idx = (wr + pulseLen) % BUF;
      int idxPrev = (idx - 1 + BUF) % BUF;
      int16_t d = abs(buf[idx] - buf[idxPrev]);
      if (d > (trigger >> 1)) pulseLen++;
      else break;
    }

    int startIdx = p1;
    int endIdx = (wr + pulseLen) % BUF;
    int16_t y1 = buf[startIdx];
    int16_t y2 = buf[endIdx];

    for (int i = 0; i < pulseLen; i++) {
      int idx = (wr + i) % BUF;
      float t = (float)(i + 1) / (pulseLen + 1);
      buf[idx] = y1 + (y2 - y1) * t;
    }
  }


  static float gain = 256;
  static uint64_t quietStart = 0;

  int16_t out = buf[rd];
  int16_t level = abs(out);

  if (level >= pulseSlider.value / 3) {
    quietStart = 0;
    if (gain < 256) gain += 2;  // slow recovery
  } else {
    if (quietStart == 0) quietStart = audioTicks;
    if ((audioTicks - quietStart) > 2000) {  // ~0.2s quiet
      gain -= (gain / 1000.0f);              // fade out
    }
  }




  out = (out * (uint16_t)gain) >> 8;

  wr++;
  if (wr >= BUF) wr = 0;

  return out;
}

//##########################################################################################################################//
int16_t notchF(int16_t x) {
  // history
  static float x1 = 0, x2 = 0;
  static float y1 = 0, y2 = 0;

  const float r = 0.94;  // determines how narrow the filter is

  const float w0 = 2.0f * PI * (float)notchSlider.value * 20.0f / 8000.0f;

  const float c = cosf(w0);

  float y =
    x
    - 2.0f * c * x1
    + x2
    + 2.0f * r * c * y1
    - r * r * y2;

  x2 = x1;
  x1 = x;

  y2 = y1;
  y1 = y;

  // clamp
  if (y > 32767) y = 32767;
  if (y < -32768) y = -32768;

  return (int16_t)y;
}


//##########################################################################################################################//


int16_t peakF(int16_t x) {
  // history
  static float y1 = 0, y2 = 0;


  float freq = (float)notchSlider.value * 20.0f;

  if (freq < 40.0f)
    freq = 40.0f;

  // bandw
  const float r = 0.90f;  // wider than the notch

  // boost
  const float gain = 2.0f;

  float w0 =
    2.0f * PI * freq / 8000.0f;

  float c = cosf(w0);

  // narrow bandpass
  float bp =
    (1.0f - r) * x
    + 2.0f * r * c * y1
    - r * r * y2;

  // update
  y2 = y1;
  y1 = bp;

  // add gain
  float y = gain * bp;

  // clamp
  if (y > 32767) y = 32767;
  if (y < -32768) y = -32768;

  return (int16_t)y;
}
//##########################################################################################################################//

int16_t combFilter(int16_t x, int fundamentalHz) {
  const int fs = 8000;  // sample rate
  int32_t y;
  // calculate period
  int N = fs / fundamentalHz;
  if (N < 2) N = 2;      // need min. 2 samples
  if (N > 256) N = 256;  //clamp to buffer size for very low fundamentals

  static int16_t delayBuffer[256];
  static int idx = 0;
  static int currentN = 0;

  // if fundamental changed, reset index
  if (N != currentN) {
    idx = 0;
    currentN = N;
  }

  // wrapper
  if (idx >= currentN) idx = 0;

  // delayed sample
  int16_t delayed = delayBuffer[idx];

  if (!invertComb)
    y = (int32_t)x - (int32_t)delayed;

  else
    y = (int32_t)x + (int32_t)delayed;


  // store sample
  delayBuffer[idx] = x;
  idx++;

  // clamp
  if (y > 32767) y = 32767;
  if (y < -32768) y = -32768;

  return (int16_t)y;
}

//##########################################################################################################################//


int16_t softMute(int16_t s, uint8_t triggerLevel) {
  static int32_t gain = 256;

  static uint64_t quietStart = 0;

  int16_t level = abs(s);


  if (level >= triggerLevel) {

    quietStart = 0;

    // recover slowly sounds better than aprupt
    if (gain < 256)
      gain++;

  } else {

    if (quietStart == 0)
      quietStart = audioTicks;

    // low amplitude >0.5s
    if ((audioTicks - quietStart) > 4000) {

      // decrease
      if (gain > 0)
        gain--;
    }
  }
  s = (s * gain) >> 8;

  return s;
}


//##########################################################################################################################//


void fftNR() {  // basic FFT noise reductor. causes clicks.

  if (nrSlider.value) {
    FFT.compute(RvReal, RvImag, audioBufSize, FFT_FORWARD);
    float gain = (float)1 / nrSlider.value;
    float gainAdjust = 1.0f + (float)nrSlider.value / 100;  // add  Gain for high slider values

    for (int i = 1; i < audioBufSize / 2; i++) {

      float mag = fabsf(RvReal[i]) + fabsf(RvImag[i]);  // fast magnitude approximation

      if (!nrMode) {
        // noise threshold
        float threshold = (nrSlider.value) * 4;

        if (mag < threshold) {

          RvReal[i] = RvReal[i + 1] * gain;
          RvImag[i] = RvImag[i + 1] * gain;

          //mirror bin
          int j = audioBufSize - i;

          RvReal[j] = RvReal[j + 1] * gain;
          RvImag[j] = RvImag[j + 1] * gain;
        }

        else {

          RvReal[i] *= (gainAdjust + (float)i / 255);
          RvImag[i] *= (gainAdjust + (float)i / 255);

          //mirror bin
          int j = audioBufSize - i;

          RvReal[j] *= (gainAdjust + (float)i / 255);
          RvImag[j] *= (gainAdjust + (float)i / 255);
        }
      }
    }
    FFT.compute(RvReal, RvImag, audioBufSize, FFT_REVERSE);

    for (int i = 0; i < audioBufSize; i++)

      playBuffer[i] = (uint8_t)((RvReal[i]) + 128);
  }
}

//##########################################################################################################################//


int16_t noiseReduction(int16_t s, uint8_t alphaPercent) {  // time domain noise reductor


  alphaPercent = 100 - alphaPercent;

  float alpha = (float)alphaPercent / 100.0f;

  static float yPrev = 0.0f;  // previous output

  // weighted average
  float y = alpha * (float)s + (1.0f - alpha) * yPrev;

  yPrev = y;

  return 1.5f * (int16_t)y;
}
