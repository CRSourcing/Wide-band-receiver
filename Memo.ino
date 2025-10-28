


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

  tft.fillRect(330, 4, 145, 22, TFT_BLACK);  // Clear No tinySA message
  tft.fillRect(0, 295, 479, 25, TFT_BLACK);  // clear status line

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
          etft.print(" NEXT");
        else if (index % 16 == 13 && index <= 127)
          etft.print(" PAGE");
        else if ((index % 16 == 15 && index <= 112) || index == 128)
          etft.print(" EXIT");
        else if (((index % 16 == 14 && index <= 111) || index == 127) && isRead)
          etft.print(" SCAN");
        else if (((index % 16 == 14 && index <= 111) || index == 127) && !isRead)
          etft.print(" N//A");

        else {  // display memo frequencies
          etft.print(FR / 1000000);
          etft.print(".");
          etft.setTextColor(TFT_YELLOW);
          etft.setTTFFont(Arial_9);
          if (((FR % 1000000) / 1000) < 100)  // insert 0 if Freq < 100
            etft.print("0");
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

    column = 1 + (tx / HorSpacing);      // get column
    row = 1 + ((ty - 40) / VerSpacing);  // get row

    if (row == 4 && column == 4 && currentPage == totalPages - 1) {  // Last page reached, or Exit pressed
      selected_band = -1;                                            //
      redrawMainScreen = true;
      tx = ty = pressed = 0;
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
      char buffer[50];
      sprintf(buffer, "sweep center %ld", FREQ);
      Serial_println(buffer);
#endif


      if (!bandWidth || !modType)
        handleMissingData();

      if (oldModType != modType) {  // only reload modulation when modType is different. Will load SSB patch.
        oldModType = modType;
        loadSi4735parameters();
      }

      setLO();
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



  if (tx > 345 && loadHistory) {  // touch was in big button area, load history
    loadFreqFromHistory();
    tRel();
    tx = 0;
    return;
  }

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
      selected_band = -1;                      // remove selected band
      FREQ = memoList[buttonID - 1].memoFreq;  // buttonID starts with 1
      modType = memoList[buttonID - 1].memoModType;
      bandWidth = memoList[buttonID - 1].memoBandwidth;


#ifdef PRINT_STORAGE_DEBUG_MESSAGES
      tft.fillRect(400, 303, 80, 17, TFT_BLACK);
      tft.setCursor(400, 303);
      tft.printf("Memoaction: M%d B%d", modType, bandWidth);
#endif

      displayFREQ(FREQ);
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
      setLO();
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
  etft.print("Go to page:");

  struct Button {
    int x;
    int y;
    const char *label;
  };

  Button buttons[] = {
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

  
  
  tft.fillRect(345, 48, 130, 242, TFT_BLACK);
  tft.setTextColor(TFT_GREEN);

  tft.setTextSize(1);
  tft.setCursor(362, 55);
  tft.print("History panel");
  tft.setCursor(362, 68);
  tft.print("Touch to load:");
  tft.setTextSize(2);
  tft.setTextColor(TFT_SKYBLUE);

  uint8_t startPos = bufferFull ? bufferIndex : 0;  // Oldest entry if full
  uint8_t count = bufferFull ? 10 : bufferIndex;    // Total entries

  for (uint8_t i = 0; i < count; i++) {
    uint8_t entry = (startPos + i) % 10;  // Circular buffer
    uint32_t freq = buffer[entry].frequency;

    char freqStr[15];

    sprintf(freqStr, "%3lu.%03lu", freq / 1000000, (freq % 1000000) / 1000);

    tft.drawRect(346, 80 + (20 * i), 130, 20, TFT_BLUE);

    tft.drawString(freqStr, 409, (20 * i + 20) + 80);  // FREQ in KHz
  }
  loadHistory = true;
}

//##########################################################################################################################//

void loadFreqFromHistory() {

  loadHistory = false;

  uint8_t startPos = bufferFull ? bufferIndex : 0;
  uint8_t count = bufferFull ? 10 : bufferIndex;
  for (uint8_t i = 0; i < count; i++) {
    if (ty >= 80 + (20 * i) && ty < 80 + (20 * (i + 1))) {

      uint8_t entry = (startPos + i) % 10;
      FREQ = buffer[entry].frequency;  // Update  FREQ
      setLO();                         //load it
      modType = buffer[entry].mode;    // update modType
      loadSi4735parameters();
      return;
    }
  }
}
