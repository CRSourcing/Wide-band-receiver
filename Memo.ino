


//Memory functions for 8 memo pages with 12 memory slots each
// Use encoder to select a slot to listen without leaving memo. Press or touch to set and leave.
//Press SCAN once, then press the memory slots that should be scanned.
// Press SCAN again and scanning begins.

//##########################################################################################################################//

int currentPage = 0;
const int totalPages = 8;
long SAVE_FREQ = FREQ;
//##########################################################################################################################/

void printMemoName(int index) {

  displayDebugInfo = false;  // make room for memo name
  tft.setTextColor(TFT_GREEN);
  tft.fillRect(2, 294, 477, 25, TFT_BLACK);
  displayText(2, 300, 252, 19, memoList[index].memoName);
  tft.setTextColor(textColor);
}

//##########################################################################################################################//


void showMemo(bool isRead, bool usePageZero) {  // displays memo buttons

  long FR;
  int index = 0;
  row = 0;
  column = 0;

  tft.fillRect(0, 295, 479, 25, TFT_BLACK);  // clear status line
  if (timeSet) {
    timeOld = 0;
    tft.fillRect(340, 6, 116, 16, TFT_BLACK);  // overwrite last time
  }

  showFreqHistory();

  if (isRead)
    modType = 1;  // default to AM at startup

  loadSi4735parameters();

  if (usePageZero)  // when called from Mainscreen
    currentPage = 0;


  tft.setTextSize(2);
  while (true) {
    tft.fillRect(2, 50, 337, 240, TFT_BLACK);
    draw16Buttons();                           // buttons
    tft.fillRect(0, 295, 479, 24, TFT_BLACK);  // clear status line

    index = currentPage * 16;
    int x_positions[] = { 10, 93, 176, 262 };
    int y_positions[] = { 70, 128, 186, 244 };


    for (int i = 0; i < 4; i++) {  // display tiles and freq values
      for (int j = 0; j < 4; j++) {
        etft.setCursor(x_positions[j], y_positions[i]);

        FR = memoList[index].memoFreq;
        index++;

        etft.setTextColor(TFT_WHITE);
        etft.setTTFFont(Arial_14);

        if (index % 16 == 0 && index <= 112 && index != 0)
          etft.print(F(" NEXT"));
        else if (index % 16 == 13 && index <= 127)
          etft.print(F(" PAGE"));
        else if ((index % 16 == 15 && index <= 112) || index == 128)
          etft.print(F(" EXIT"));
        else if (((index % 16 == 14 && index <= 111) || index == 127) && isRead)
          etft.print(F(" SCAN"));
        else if (((index % 16 == 14 && index <= 111) || index == 127) && !isRead)
          etft.print(F(" N//A"));

        else {  // display memo frequencies
          etft.print(FR / 1000000);
          etft.print(F("."));
          etft.setTextColor(TFT_YELLOW);
          etft.setTTFFont(Arial_9);
          if (((FR % 1000000) / 1000) < 100)  // insert 0 if Freq < 100
            etft.print(F("0"));
          etft.printf("%ld", (FR % 1000000) / 1000);
          etft.setCursor(x_positions[j] + 40, y_positions[i] + 25);
          etft.setTTFFont(Arial_8);
          etft.setTextColor(TFT_SKYBLUE);
          etft.printf("%d", index);
          etft.setTextColor(textColor);
        }
      }
    }

    int result = 99;
    if (isRead)
      result = tuneMemo();

  if (tx > 345 && ty >= 80) {  // touch was in history area
    loadFreqFromHistory();
    return;
  }



    if (isRead && result == 1) {  // tuneMemo returns 1 for encoder pressed
      tx = ty = 999;              // readMemo() will interpret this as outside of touch area and return
      selected_band = -1;         // remove selected band
      return;
    }

    if (isRead && result == 2) {  // tuneMemo returns 2 for button pressed
      selected_band = -1;         // remove selected band
      tRel();
    }


    if (!isRead)  // Save Memo
      tDoublePress();

    getButtonID();

    if (row == 4 && column == 4 && currentPage == totalPages - 1) {  // Last page reached, or Exit pressed
      selected_band = -1;                                            //
      resetMainScreen();
      tRel();
      mainScreen();
      return;
    }
    if (result == -1) {  /// Encoder counterclockwise,  tuneMemo returned -1, load prev. page.
      currentPage -= 1;

    } else if ((row == 4 && column == 4) || result == 0) {  // "Next" button , or tuneMemo returned 0, load next page.
      currentPage = (currentPage + 1) % totalPages;

    }


    else if (row == 4 && column == 1)
      selectPage();

    else
      break;
  }
}
//##########################################################################################################################//


int tuneMemo() {  //tunes through memory bank with encoder.

  encLockedtoSynth = false;
  clw = false;
  cclw = false;
  static int dex = 0;  // index
  static int oldModType = -1;
  int oldDex = 0;
  long SAVE_FREQ = FREQ;
  dex = (currentPage * 16) + 1;


  do {
    pressed = get_Touch();


    delay(50);  // otherwise encoder will skip steps

    if (clw)
      dex++;
    if (cclw)
      dex--;
    if (dex < 1)
      dex = 1;
    if (dex > 128)
      dex = 1;

    if ((dex == 13 || dex == 29 || dex == 45 || dex == 61 || dex == 77 || dex == 93 || dex == 109 || dex == 125) && clw) {
      dex += 4;  // skip PAGE SCAN, EXIT and NEXT
      return 0;
    }

    if ((dex == 16 || dex == 32 || dex == 48 || dex == 64 || dex == 80 || dex == 96 || dex == 112 || dex == 128) && cclw) {

      return -1;  // load previous page
    }


    if (oldDex != dex) {
      drawMarker(oldDex % 16, TFT_NAVY);
      drawMarker(dex % 16, TFT_YELLOW);

      FREQ = memoList[dex - 1].memoFreq;
      modType = memoList[dex - 1].memoModType;
      bandWidth = memoList[dex - 1].memoBandwidth;
      printMemoName(dex - 1);
      displayFREQ(FREQ);


#ifdef TINYSA_PRESENT
      centerTinySA();
#endif

      if (!bandWidth || !modType)
        handleMissingData();

      if (oldModType != modType) {  // only reload modulation when modType is different. Will load SSB patch.
        oldModType = modType;
        loadSi4735parameters();
      }

      setFreq();
    }


    if (digitalRead(ENCODER_BUTTON) == LOW) {  // use encoder to load memo
      tft.fillRect(345, 2, 130, 20, TFT_BLACK);
      encLockedtoSynth = true;
      modType = oldModType;
      loadSi4735parameters();
      while (digitalRead(ENCODER_BUTTON) == LOW)
        ;
      return 1;
    }

    oldDex = dex;

    getRSSIAndSNR();
    readSquelchPot(false);  // do not draw position / room occupied by tiles
    setSquelch();
    clw = false;
    cclw = false;

#ifdef TV_TUNER_PRESENT
    if (FREQ > SHORTWAVE_MODE_UPPER_LIMIT)
      setTunerAGC(false);  // run tuner agc calculator
#endif

    if (!audioMuted)
      drawMarker(dex % 16, TFT_GREEN);  // draw green after loading (tinySA and SSB patch)
    else
      drawMarker(dex % 16, TFT_RED);  // draw green after loading (tinySA and SSB patch)

  }

  while (!pressed);


  encLockedtoSynth = true;
  tft.fillRect(345, 2, 130, 20, TFT_BLACK);
  drawMarker(dex % 16, TFT_NAVY);
  FREQ = SAVE_FREQ;
  FREQ_OLD = FREQ - 1;  // trigger  displayFREQ
  return 2;
}

//##########################################################################################################################//

void memoAction(bool isWrite) {  // reads button pressed and sets FREQ, bandwidth and modType

  int row, column;
  static int oldModType = 1;

  column = 1 + (tx / HorSpacing);  // get row and column
  row = 1 + ((ty - 40) / VerSpacing);

  if (row > 4 || column > 4) {
    tx = ty = 0;
    return;  // outside of key area
  }

  int buttonID = (currentPage * 16) + (row - 1) * 4 + (column - 1) + 1;

  if (buttonID >= 1 && buttonID <= totalPages * 16) {


    if ((buttonID % 16 == 13 && buttonID < 127)) {  // Page pressed
      selectPage();
      showMemo(isWrite, 0);
    }

    if ((buttonID % 16 == 15 && buttonID < 127) || buttonID == 128) {  // Exit pressed
      modType = oldModType;
      loadSi4735parameters();
      tft.fillRect(0, 295, 479, 24, TFT_BLACK);  // clear status line
      return;
    }

    if ((buttonID == 14 || buttonID == 30 || buttonID == 46 || buttonID == 62 || buttonID == 78 || buttonID == 94 || buttonID == 110 || buttonID == 127) && !isWrite) {  // Scan pressed
      memoScanner();
      return;
    }
    if (isWrite) {
      selected_band = -1;  // remove selected band
      updateMemoInfo(buttonID);
    }

    else {
      selected_band = -1;  // remove selected band
      use1MHzSteps = false;
      FREQ = memoList[buttonID - 1].memoFreq;  // buttonID starts with 1
      modType = memoList[buttonID - 1].memoModType;
      bandWidth = memoList[buttonID - 1].memoBandwidth;

#ifdef TINYSA_PRESENT
      adjustTSACenterToFREQ();
#endif


      displayFREQ(FREQ);
      resetSmeter = true;
      if (oldModType != modType) {  // only reload when modType is different
        oldModType = modType;
        loadSi4735parameters();
      }
    }
    tRel();
  }
}

//##########################################################################################################################//

void memoScanner() {  // SCAN was pressed, press memo buttons to enter frequencies and press SCAN again to start

  char buttonIDBuffer[14] = { 0 };
  int index = 0;
  int row, column;
  int oldCtr = 0;

#ifdef TINYSA_PRESENT
  bool once = false;
#endif

  tPress();
  selected_band = -1;  // remove selected band
  while (true) {

    tPress();
    tRel();

    column = 1 + (tx / 90);              // get row and column
    row = 1 + ((ty - 40) / VerSpacing);  // was ty - 20
    if (row > 4 || column > 4)
      return;                                                              // outside of key area
    int buttonID = (currentPage * 16) + (row - 1) * 4 + (column - 1) + 1;  // buttonID starts with 1

    if (buttonID == 14 && index == 0)  // scan pressed 2x by mistake
      return;

    if (buttonID == 14 || buttonID == 30 || buttonID == 46 || buttonID == 62 || buttonID == 78 || buttonID == 94 || buttonID == 110 || buttonID == 127)  //SCAN pressed again, start scanning
      break;
    if (buttonID == 15 || buttonID == 31 || buttonID == 47 || buttonID == 63 || buttonID == 79 || buttonID == 95 || buttonID == 111 || buttonID == 128)  // Exit pressed
      return;

    buttonIDBuffer[index] = buttonID;  // first buttonID at index 0

    index++;
  }  //endwhile select frequencies

  while (true) {  // run scan loop
    int idx = 0;
    while (idx < index) {

      FREQ = memoList[(buttonIDBuffer[idx] - 1)].memoFreq;
      modType = memoList[(buttonIDBuffer[idx] - 1)].memoModType;
      //bandWidth = memoList[(buttonIDBuffer[idx]- 1)].memoBandwidth;
      printMemoName(buttonIDBuffer[idx] - 1);


      if (!bandWidth || !modType)
        handleMissingData();

      if (modType == 1 || modType == 4 || modType == 5)
        loadSi4735parameters();  // Only non BFO modes,  loading the SSB patch would take too long

      displayFREQ(FREQ);
      setFreq();
      drawMarker(buttonIDBuffer[idx], TFT_RED);  // draws a red dot on the tile being scanned

      delay(50);                                  // this is the main delay for scanning
      drawMarker(buttonIDBuffer[idx], TFT_NAVY);  // and overwrite red dot


      int ctr = 0;


      while (true) {
        getRSSIAndSNR();
        readSquelchPot(false);  // do not draw position
        setSquelch();
        delay(15);
        pressed = get_Touch();

#ifdef FAST_TOUCH_HANDLER
        delay(10);  // need to slow down
#endif

        if (pressed) {
          tft.fillRect(345, 2, 130, 20, TFT_BLACK);
          ctr = 0;
          tx = ty = pressed = 0;
          return;
        }



        if ((signalStrength > currentSquelch) || (SNRSquelch && SNR)) {
          drawMarker(buttonIDBuffer[idx], TFT_GREEN);

          ctr = 100;

#ifdef TINYSA_PRESENT
          if (!once) {
            resetTSA();
            once = true;
          }
#endif
        }

        if ((signalStrength < currentSquelch) || (SNRSquelch && !SNR)) {  // countdown timer until scan continues
          ctr--;

#ifdef TINYSA_PRESENT
          once = false;
#endif
          if (ctr / 10 != oldCtr) {
            tft.setTextColor((TFT_RED));
            tft.fillRect(345, 3, 130, 20, TFT_BLACK);
            tft.setCursor(345, 6);
            tft.printf("%d", ctr / 10);
            oldCtr = ctr / 10;
          }
        }

        if (ctr <= 0) {
          drawMarker(buttonIDBuffer[idx], TFT_NAVY);
          tft.fillRect(345, 3, 130, 20, TFT_BLACK);
          tft.setTextColor((textColor));
          break;
        }
      }
      idx++;
    }
  }
}

//##########################################################################################################################//
void writeMemo() {
  memoAction(true);
}
//##########################################################################################################################//
void readMemo() {
  memoAction(false);
}
//##########################################################################################################################//


void drawMarker(int buttonID, uint16_t COLOR) {  // draws a circle on the memo tiles
  int idx = (buttonID - 1) % 16;
  int row = idx / 4;
  int column = idx % 4;
  int x_positions[] = { 10, 93, 176, 262 };
  int y_positions[] = { 75, 132, 191, 249 };
  tft.fillCircle(x_positions[column] + 20, y_positions[row] + 25, 6, COLOR);
}

//##########################################################################################################################//

void handleMissingData() {

  if (!modType)
    modType = 1;  // default to AM
  if (!bandWidth)
    si4735.setBandwidth(0, 1);  // default to 6KHz
}

//##########################################################################################################################//

void updateMemoInfo(int buttonID) {  // Save Memo , writes data to LittleFS and reloads file

  if (!LittleFS.begin(true)) {
    Serial_println("updateMemoInfo: An error has occurred while mounting LittleFS");
  } else {
    Serial_println("updateMemoInfo :LittleFS mounted successfully");
  }

  overwriteMemoInfoLine(buttonID);  // insert new values and write to LittleFS
  loadCSVtoStruct("MemoInfo.csv");  // reload file
  reloadMemoList();                 // reload vector
}


//##########################################################################################################################//

void selectPage() {

  draw16Buttons();  // only 8 buttons needed, so

  tft.fillRect(3, 61, 334, 112, TFT_BLACK);  // overwrite row 1+2



  etft.setTextColor(TFT_WHITE);
  etft.setTTFFont(Arial_18);
  etft.setCursor(10, 75);
  etft.print(F("Go to page:"));

  struct Button {
    int x;
    int y;
    const char *label;
  };

  const Button buttons[] PROGMEM = {
    { 30, 190, "1" }, { 120, 190, "2" }, { 200, 190, "3" }, { 283, 190, "4" }, { 30, 248, "5" }, { 120, 248, "6" }, { 200, 248, "7" }, { 283, 248, "8" }
  };

  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].label);
  }

  tx = ty = 0;
  tDoublePress();

  column = 1 + (tx / HorSpacing);      // get column
  row = 1 + ((ty - 40) / VerSpacing);  // get row

  if (row < 3)  // no page selected
    return;

  currentPage = column - 1;
  if (row == 4)
    currentPage += 4;

  etft.setCursor(10, 75);
  etft.printf("Go to page: %d", currentPage + 1);
  delay(500);
}



//##########################################################################################################################//

void showFreqHistory() {
  const int baseX = 345;
  const int baseY = 48;
  const int boxW = 130;
  const int boxH = 20;
  const int rowSpacing = 24;

  tft.fillRect(baseX, baseY, boxW, 242, TFT_BLACK);

  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(1);
  tft.setCursor(baseX + 15, baseY + 7);
  tft.print(F("Freq history."));
  tft.setCursor(baseX + 15, baseY + 20);
  tft.print(F("Touch to load."));

  tft.setTextSize(2);
  tft.setTextColor(TFT_YELLOW);

  uint8_t startPos = bufferFull ? bufferIndex : 0;
  uint8_t count = bufferFull ? 8 : bufferIndex;

  for (uint8_t i = 0; i < count; i++) {
    uint8_t entry = (startPos + i) % 8;
    uint32_t freq = buffer[entry].frequency;

    char freqStr[16];
    snprintf(freqStr, sizeof(freqStr), "%3lu.%03lu",
             freq / 1000000, (freq % 1000000) / 1000);

    int y = baseY + 32 + i * rowSpacing;

    tft.drawRect(baseX + 1, y, boxW - 2, boxH, TFT_WHITE);

    int halfHeight = (boxH - 2) / 2;

    tft.fillRectVGradient(baseX + 2, y + 1, boxW - 4, halfHeight, TFT_NAVY, TFT_BLUE);
    tft.fillRectVGradient(baseX + 2, y + 1 + halfHeight, boxW - 4, halfHeight, TFT_BLUE, TFT_NAVY);



    tft.drawString(freqStr, baseX + boxW / 2, y + 18);
  }

  loadFromHistory = true;
  tft.setTextColor(textColor);
}

//##########################################################################################################################//
void loadFreqFromHistory() {

  loadFromHistory = false;

  const int baseY = 80;
  const int rowSpacing = 24;
  const int boxH = 20;

  uint8_t startPos = bufferFull ? bufferIndex : 0;
  uint8_t count = bufferFull ? 8 : bufferIndex;

  for (uint8_t i = 0; i < count; i++) {
    int yTop = baseY + i * rowSpacing;
    int yBottom = yTop + boxH;

    if (ty >= yTop && ty < yBottom) {
      uint8_t entry = (startPos + i) % 8;
      FREQ = buffer[entry].frequency;
      setFreq();
      modType = buffer[entry].mode;
      loadSi4735parameters();

      return;
    }
  }
}

//##########################################################################################################################//

// PicoRX  station list loaders, loads from LittleFS, uses memory.csv compatible with PicoRX station list


//##########################################################################################################################//


void picoMenu() {

  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);
  tft.fillRect(3, 121, 333, 114, TFT_BLACK);
  hideBigBtns();
  if (timeSet) {
    timeOld = 0;
    tft.fillRect(340, 6, 116, 16, TFT_BLACK);  // overwrite last time
  }

  drawButton(8, 234, TILE_WIDTH, TILE_HEIGHT, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setTTFFont(Arial_14);
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(20, 254);
  etft.print(F("BACK"));

  etft.setCursor(102, 244);
  etft.print(F("Show"));
  etft.setCursor(102, 264);
  etft.print(F("Single"));

  etft.setCursor(185, 244);
  etft.print(F("Show"));
  etft.setCursor(185, 264);
  etft.print(F("Page"));

  etft.setCursor(10, 125);
  etft.print(F("This utility loads file memory.csv"));
  etft.setCursor(10, 145);
  etft.print(F("from LittleFS."));
  etft.setCursor(10, 165);
  etft.print(F("Each page (12 entries) should use"));
  etft.setCursor(10, 185);
  etft.print(F("consistently either Shortwave or"));
  etft.setCursor(10, 205);
  etft.print(F("VHF/UHF entries."));

  tRel();
  tPress();
  readPicoStationList();
}


//##########################################################################################################################//

void readPicoStationList() {  // use PicoRX format station list


  int buttonID = getButtonID();

  if (!buttonID)
    return;  // outside of area

  redrawMainScreen = true;                    // save freq for returning from waterfall
  tft.fillRect(135, 295, 92, 25, TFT_BLACK);  // overwrite frozen spectrum window
  switch (buttonID) {
    case 41:
      return;
    case 42:
      tuneSingleChannel();
      return;
    case 43:
      showChannelList();
      si4735.setHardwareAudioMute(false);
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
void tuneSingleChannel() {

  tft.fillScreen(TFT_BLACK);
  rebuildMainScreen(false);


  enum ChannelNav {
    next = 1,
    current = 0,
    last = -1,
    start = -2
  };


  const uint8_t button1X = 15;
  const uint8_t button2X = 95;
  const uint8_t button3X = 175;
  const uint8_t button4X = 250;
  const uint8_t buttonY = 233;
  drawSingleChannelButtons();


  static bool firstTime = true;
  if (firstTime) {
    load_channel(last, true, 1, false, true);
    firstTime = false;
  }

  else
    load_channel(current, true, 0, false, true);

  // void load_channel(int direction, bool reloadModType, int16_t addRows, bool fromStart, bool showEntryNumber)



  while (true) {

    if (digitalRead(ENCODER_BUTTON) == LOW) {  //return if encoder pressed
      while (digitalRead(ENCODER_BUTTON) == LOW)
        ;
      redrawMainScreen = true;
      return;
    }



    pressed = tft.getTouch(&tx, &ty);
    if (pressed && (tx >= button1X && tx <= (button1X + TILE_WIDTH) && ty >= buttonY && ty <= (buttonY + TILE_WIDTH)))  // go back to first row
      load_channel(start, false, 0, false, true);

    if (pressed && (tx >= button2X && tx <= (button2X + TILE_WIDTH) && ty >= buttonY && ty <= (buttonY + TILE_WIDTH))) {  // exit
      tRel();
      redrawMainScreen = true;
      return;
    }


    if (pressed && (tx >= button3X && tx <= (button3X + TILE_WIDTH) && ty >= buttonY && ty <= (buttonY + TILE_WIDTH))) {  // load from keypad


      drawNumPad();
      drawKeypadButtons();
      tft.fillRect(4, 4, 470, 40, TFT_BLACK);
      tft.setCursor(10, 20);
      tft.print(F("Enter memory slot + return:"));
      tPress();  // wait until pressed
      readKeypadButtons(true);
      tft.fillRect(4, 4, 470, 40, TFT_BLACK);
      drawSingleChannelButtons();
      redrawIndicators();
      drawBigBtns();
      load_channel(next, false, keyVal, true, true);
    }


    if (pressed && (tx >= button4X && tx <= (button4X + TILE_WIDTH) && ty >= buttonY && ty <= (buttonY + TILE_WIDTH)))  // add 20 rows
      load_channel(next, false, 20, false, true);



    if (clw || cclw) {

      if (clw)
        load_channel(next, false, 1, false, true);
      else if (cclw)
        load_channel(last, false, 1, false, true);

      clw = false;
      cclw = false;
    }

    playCurrentChannel();
  }
}

//##########################################################################################################################//

void playCurrentChannel() {

  if (FREQ != FREQ_OLD) {  // If FREQ changes, update Si5351A
    FREQCheck();           //check whether within FREQ range
    displayFREQ(FREQ);     // display new FREQ
    setFreq();             // and tune it in
    FREQ_OLD = FREQ;

#ifdef TINYSA_PRESENT
    resetTSA();
#endif
  }

  if (clw || cclw)
    return;


  audioSpectrum();
  getRSSIAndSNR();

#ifdef TV_TUNER_PRESENT
  setTunerAGC(false);  // use tuner AGC without printng values
#endif

  if (showRSSITrace == 1) {
    drawRollingGraph(signalStrength);
  }

  TSAdBmValue = 0;  // force to use RSSI for s meter
  calculateAndDisplaySignalStrength();
  readSquelchPot(true);  // true = read and draw position circle
  setSquelch();
  fineTune();  // read frequency potentiometer

#ifdef NBFM_DEMODULATOR_PRESENT
  getDiscriminatorVoltage();  // display Tuning meter

#else
  if (dBm < 0 && (!scanMode) && showMeters)
    plotNeedle(signalStrength, 2);  // update the SMeter needle
#endif

  displaySmeterBar(3);  // // update the SMeter bar,
  if (showMeters) {
    if (!audioMuted)
      plotNeedle2(currentVU, 3);
    else
      plotNeedle2(1, 0);
  }

  delay(10);
}



//##########################################################################################################################//
#define MAX_LINE_LENGTH 80

void load_channel(int action, bool reloadModType, int16_t addRows, bool fromStart, bool showEntryNumber) {
  static uint8_t bufI[100];  // static, keep values when reusing func.
  uint8_t bufO[80];
  static uint16_t offs = 0x5D;  // row zero does not contain valid data, so start with row 1
  static uint16_t ep = 0;
  int16_t i = 0;



  File f = LittleFS.open("/memory.csv", "r");
  if (!f) {
    tft.setCursor(10, 200);
    tft.println("Could not load memory.csv");
    delay(1000);
    return;
  }

  size_t fileSize = f.size();  // for check out of range



  if (action == -2) {  // return to start
    offs = 0;
    cRow = 0;
    addRows = 1;
  }

  if (fromStart) {
    offs = 0;
    cRow = 0;
  }


  if (action == -1) {  // cclw, go back 2 rows

    offs -= 2 * MAX_LINE_LENGTH;
    while (bufI[0] != 0x0A) {  // forward to next seperator
      f.seek(offs, SeekSet);
      f.read(bufI, 1);
      offs++;
    }
    cRow--;
    bufI[0] = 0;
  }



  for (int16_t s = 0; s < addRows; s++) {

    if (action == 1)
      cRow++;

    if (cRow < 1) {
      cRow = 1;
      offs = 0x5D;
    }


    if (offs >= fileSize - MAX_LINE_LENGTH) {  // wrap
      offs = 0x5D;
      cRow = 1;
      return;
    }


    if (!f.seek(offs, SeekSet)) {
      f.close();
      Serial_println("file seek error");
      return;
    }

    f.read(bufI, sizeof(bufI));
    while (bufI[i++] != 0x0A)
      ;  // looking for "." seperator

    ep = i;
    offs += ep;
    i = 0;
  }



  for (i = 0; i < ep; i++)  // load row into buffer
    bufO[i] = bufI[i];

  bufO[i + 1] = '\0';

  setChannel((char *)bufO, cRow, reloadModType, showEntryNumber);

  f.close();

  i = 0;
}


//##########################################################################################################################//
void setChannel(const char *buffer, int cRow, bool reloadModType, bool showEntryNumber) {
  int x = 30;
  int y = 130;

  // static storage of last values
  static char lastLabel[32] = "";
  static char lastFreq[32] = "";
  static char lastMode[32] = "";

  // Split CSV string into tokens
  char temp[128];
  strncpy(temp, buffer, sizeof(temp));
  temp[sizeof(temp) - 1] = '\0';

  char *tokens[10] = { 0 };
  int count = 0;
  char *p = strtok(temp, ",");
  while (p != NULL && count < 10) {
    tokens[count++] = p;
    p = strtok(NULL, ",");
  }


  tft.setTextColor(TFT_CYAN);
  tft.fillRect(x + 128, y, 66, 15, TFT_BLACK);
  tft.setCursor(x, y);
  if (showEntryNumber)
    tft.printf(" <- Entry: %d ->", cRow);


  y += 40;

  // Label
  if ((count > 0 && strcmp(tokens[0], lastLabel) != 0) || reloadModType == true) {
    tft.fillRect(x - 10, y, 300, 25, TFT_BLACK);  // clear old text
    tft.setCursor(25, y);
    tft.setTextSize(3);
    tft.setTextColor(TFT_SKYBLUE);
    tft.println(tokens[0]);
    tft.setTextColor(TFT_CYAN);
    strncpy(lastLabel, tokens[0], sizeof(lastLabel));
    tft.setTextSize(2);
  }
  y += 45;

  // Frequency
  if (count > 1 && strcmp(tokens[1], lastFreq) != 0) {
    tft.fillRect(x, y, 250, 18, TFT_BLACK);
    tft.setCursor(x, y);
    strncpy(lastFreq, tokens[1], sizeof(lastFreq));
    FREQ = atol((char *)lastFreq);
    tft.printf("Freq: %ldKHz", FREQ_TO_KHZ);
  }


  // Mode
  if ((count > 4 && strcmp(tokens[4], lastMode) != 0) || reloadModType == true) {  // this may give false positives, if a leading or trailing space exists
    tft.fillRect(250, 130, 83, 15, TFT_BLACK);
    tft.setTextColor(TFT_CYAN);
    tft.setCursor(250, 130);
    tft.println(tokens[4]);
    strncpy(lastMode, tokens[4], sizeof(lastMode));

    if (strstr(lastMode, "AM") != NULL) {
      modType = AM;
    }

    if (strstr(lastMode, "LSB") != NULL) {
      modType = LSB;
    }

    if (strstr(lastMode, "USB") != NULL) {
      modType = USB;
    }

    if (strstr(lastMode, "AMS") != NULL) {
      modType = SYNC;
    }

    if (strstr(lastMode, "CW") != NULL) {
      modType = CW;
    }
    if (strstr(lastMode, "FM") != NULL) {
      modType = NBFM;
    }
    redrawIndicators();
    static uint8_t lastModType = 0;

    if (lastModType != modType || reloadModType == true) {
      loadSi4735parameters();
      lastModType = modType;
    }
  }

  tft.setTextColor(textColor);
}



//##########################################################################################################################//

const int MAIN_X = 3;
const int MAIN_Y = 61;
const int MAIN_W = 334;

const int CURSOR_Y = 253;

// Labels
const char LABEL_FIRST[] = "First";
const char LABEL_EXIT[] = "Exit";
const char LABEL_KEYB[] = "Keyb.";
const char LABEL_PLUS20[] = "+20";

void drawSingleChannelButtons() {
  // overwrite the step indicator
  tft.fillRect(340, 26, 135, 18, TFT_BLACK);
  // clear lower bar
  tft.fillRect(0, 295, 480, 25, TFT_BLACK);

  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);

  tft.setTextColor(TFT_GREEN);
  tft.fillRect(MAIN_X, MAIN_Y, MAIN_W, 179, TFT_BLACK);

  etft.setTTFFont(Arial_14);

  etft.setCursor(25, CURSOR_Y);
  etft.print(LABEL_FIRST);

  etft.setCursor(105, CURSOR_Y);
  etft.print(LABEL_EXIT);

  etft.setCursor(185, CURSOR_Y);
  etft.print(LABEL_KEYB);

  etft.setCursor(275, CURSOR_Y);
  etft.print(LABEL_PLUS20);

  tft.setTextColor(textColor);
  redrawIndicators();
}

//##########################################################################################################################//
// displays a list of entries from memory.csv. runs play loop if a row gets touched

#define ENTRIES 12  // rows per page
#define VSPACING 22
uint32_t frequencies[ENTRIES];  // 12 entries per page fill screen nicely

void showChannelList() {
  uint16_t startEntry = 1;
  uint16_t lastStartEntry = 0;
  uint16_t pressedEntry = 0;
  uint32_t freqSav = FREQ;
  bool valid = true;  // eof check

  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_SKYBLUE);
  displayText(10, 266, 310, 16, "Move encoder to change page.");
  displayText(10, 286, 310, 16, "Touch row to listen.");
  displayText(10, 304, 310, 16, "Press encoder to return.");
  tft.setTextColor(TFT_GREEN);

  if (modType != AM) {  // must use AM demodulator
    modType = AM;
    loadSi4735parameters();
  }

  si4735.setHardwareAudioMute(true);

  while (true) {
    if (clw) {
      startEntry += ENTRIES;
      clw = false;
    }

    if (cclw && startEntry > ENTRIES) {
      startEntry -= ENTRIES;
      cclw = false;
    }


    if (lastStartEntry != startEntry) {
      valid = extractAndShowRows(startEntry, ENTRIES);
      lastStartEntry = startEntry;
    }

    if (valid)
      displaySignalBar();
    else {
      sineTone(800, 100);
      startEntry -= ENTRIES;  // eof reached
    }


    get_Touch();

    if (pressed && ty) {  // a row was touched

      ty = constrain(ty, 0, 11 * VSPACING);

      pressedEntry = startEntry + ty / VSPACING;

      rebuildMainScreen(false);
      load_channel(0, true, pressedEntry, true, false);

      tft.setTextColor(TFT_CYAN);
      tft.setCursor(10, 270);
      tft.print(F("Press encoder to return."));
      redrawIndicators();
      si4735.setHardwareAudioMute(false);

      while (true) {
        playCurrentChannel();

        if (digitalRead(ENCODER_BUTTON) == LOW) {
          freqSav = FREQ;  // use the current channel as future frequency
          break;
        }

        if (clw || cclw) {
          pressedEntry += clw ? 1 : -1;  // 1 up or down
          load_channel(0, true, pressedEntry, true, false);
          clw = false;
          cclw = false;
        }
      }

      extractAndShowRows(startEntry, ENTRIES);   // back to row screen rebuild rows
      tft.fillRect(0, 264, 480, 56, TFT_BLACK);  // overwrite artefacts
      tft.setCursor(10, 280);
      tft.print(F("Touch row to listen."));
      tft.setCursor(10, 300);
      tft.print(F("Press encoder to return."));
      if (modType != AM) {  // must be AM
        modType = AM;
        loadSi4735parameters();
      }
      si4735.setHardwareAudioMute(true);  // mute again
    }

    pressed = false;

    if (digitalRead(ENCODER_BUTTON) == LOW) {
      while (digitalRead(ENCODER_BUTTON) == LOW)
        ;
      FREQ = freqSav;
      return;
    }
  }
}

//##########################################################################################################################//
bool extractAndShowRows(uint16_t startEntry, uint16_t entries) {

  tft.fillRect(0, 0, 480, 264, TFT_BLACK);
  memset(frequencies, 0, sizeof(frequencies));

  File f = LittleFS.open("/memory.csv", "r");
  if (!f) {
    tft.setCursor(10, 200);
    tft.println("Could not load memory.csv");
    delay(1000);
    return false;
  }

  // Skip lines til startEntry
  for (uint16_t s = 0; s < startEntry; s++) {
    f.readStringUntil('\n');
  }

  // show rows
  for (uint16_t e = 0; e < entries; e++) {
    if (!f.available())
      return false;

    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0)
      return false;

    frequencies[e] = showRow(line.c_str(), e, startEntry + e);
  }

  f.close();

  return true;
}

//##########################################################################################################################//
uint32_t showRow(const char *buffer, uint16_t screenRow, uint16_t csvRowNumber) {  // show details of a single row
  const uint16_t vSpacing = VSPACING;
  const int xPos = 5;
  uint16_t yPos = screenRow * vSpacing + 1;

  char temp[81];
  char *tokens[10] = { 0 };

  strncpy(temp, buffer, sizeof(temp));
  temp[sizeof(temp) - 1] = '\0';

  int count = 0;
  char *p = strtok(temp, ",");
  while (p && count < 10) {
    tokens[count++] = p;
    p = strtok(NULL, ",");
  }

  // row number
  tft.setTextColor(TFT_SKYBLUE);
  tft.setCursor(xPos, yPos);
  tft.printf("%d ", csvRowNumber);

  // Name
  tft.setCursor(xPos + 30, yPos);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.print(tokens[0] ? tokens[0] : "");

  // Frequency
  uint32_t f = tokens[1] ? atol(tokens[1]) : 0;
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(xPos + 220, yPos);
  tft.printf(" %ld KHz", f / 1000);
  return f;
}

//##########################################################################################################################//
void displaySignalBar() {

  uint16_t dly = 100;

  clw = false;
  cclw = false;
  tx = 0;
  ty = 0;
  pressed = false;

  for (int s = 0; s < ENTRIES; s++) {



    FREQ = frequencies[s];
    setFreq();


    uint32_t st = millis();
    while (millis() < st + dly) {  // delay needed so that the SI5351 can settle if delta is big

      pressed = get_Touch();

      if (pressed || (digitalRead(ENCODER_BUTTON) == LOW) || clw || cclw) {
        return;
      }
    }


    si4735.getCurrentReceivedSignalQuality(0);
    SNR = si4735.getCurrentSNR();
    delay(10);
    si4735.getCurrentReceivedSignalQuality(0);
    signalStrength = si4735.getCurrentRSSI();

    tft.fillRect(380 + signalStrength, s * VSPACING, 100 - signalStrength, 20, TFT_GREY);
    tft.fillRect(380, s * VSPACING, signalStrength, 20, SNR ? 0x8ff6 : signalStrength << 5);  // 0x8ff6 = strong green when SNR, shade of green when signalStrength only
  }
}

//##########################################################################################################################//
