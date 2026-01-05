void mainScreen() {  // main screen does not block loop(), all other screens do

  if (redrawMainScreen) {  // redraw main screen when coming from a function that overwrites it
    resetSmeter = true;
    drawBigBtns();
    drawMainButtons();
    DrawSmeterScale();
    printModulation();
    printBandWidth();
    showIFBandwidth();
    printAGC();
    readSquelchPot(true);  // read and draw position
    redrawMainScreen = false;
  }
  readMainBtns();
}


//##########################################################################################################################//
void redrawIndicators() {

  tft.fillRect(3, 52, 336, 30, TFT_BLACK);  // overwrite area for spectrum
  tft.fillRect(3, 82, 336, 41, TFT_BLACK);  // overwrite area for waterfall
  DrawSmeterScale();
  printModulation();
  printBandWidth();
  showIFBandwidth();
  printAGC();
  redrawMainScreen = true;  // this forces readSquelchPot() to redraw the squelch line and circle
  readSquelchPot(true);
  redrawMainScreen = false;
}


//##########################################################################################################################//

void drawMainButtons() {

  // Background
  if (!altStyle)
    tft.fillRect(2, 56, 337, 233, TFT_BLACK);
  else
     drawButton(2, 56, 337, 233, TFT_NAVY, TFT_DARKGREY);

    draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);

  struct Button {
    int x;
    int y;
    const char *label;
  };

  Button buttons[] = {
    { 275, 245, "Set" }, { 185, 245, "Select" }, { 110, 255, "Scan" },
    { 100, 198, "Bandw" }, { 25, 198, "Step" }, { 185, 188, "Save" },
    { 270, 188, "Load" }, { 188, 132, "Slow" }, { 270, 132, "Set" },
    { 185, 265, "Band" }, { 275, 265, "Freq" }, { 185, 208, "Memo" },
    { 267, 208, "Memo" }, { 185, 152, "Waterf." }, { 270, 152, "VFO" }
  };

  etft.setTTFFont(Arial_14);
  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.setTextColor(textColor);
    if (i == 2 && scanMode)
      etft.setTextColor(TFT_SKYBLUE);
    etft.print(buttons[i].label);
  }



  // "More" button
  drawButton(8, 234, TILE_WIDTH, TILE_HEIGHT, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(20, 254);
  etft.print("More");
  etft.setTextColor(textColor);

#ifdef TINYSA_PRESENT
  etft.setCursor(15, 132); etft.print("TinySA");
  etft.setCursor(14, 152); etft.print("Options");

#else  
  etft.setCursor(40, 132); etft.print("IF");
  etft.setCursor(18, 152); etft.print("Bandw");
#endif

#ifdef TV_TUNER_PRESENT
  etft.setCursor(100, 132);
  if (TVTunerActive) {
    etft.setTextColor(TFT_SKYBLUE);
    etft.print("Tun.");
    etft.setCursor(100, 152);
    etft.print("Attn.");
  } 
 #endif 
  
  else {
  
 #ifdef SW_ATTENUATOR_PRESENT
    etft.setTextColor(TFT_GREEN);
    etft.print("SW");
    etft.setCursor(100, 152);
    etft.print("Attn.");
    etft.setTextColor(textColor);
  #endif
  }



}


//##########################################################################################################################//

void readMainBtns() {

  if (!pressed)
    return;

  int buttonID = getButtonID();

  if (row > 4 || column > 4 || ty > 293 || ty < 121)
    return;                                   // outside of keypad area
  redrawMainScreen = true;                    // save freq for returning from waterfall
  tft.fillRect(135, 295, 92, 25, TFT_BLACK);  // overwrite frozen spectrum window
  switch (buttonID) {
    case 21:
#ifdef TINYSA_PRESENT
      tinySAScreen();
#endif
#ifndef TINYSA_PRESENT
      setIFBandwidth();
#endif
      tRel();
      break;

    case 22:
      hideBigBtns();
#ifdef TV_TUNER_PRESENT
      if (TVTunerActive)
        setInitialTunerGain();
#endif

#ifdef SW_ATTENUATOR_PRESENT
      if (!TVTunerActive)
        setRFAttenuatorMinAttn();
#endif

      break;
    case 23:
      selectWaterFall();
      tRel();
      break;
    case 24:
     vfoMenu(); 
     FREQ_OLD = FREQ - 1;    // trigger update
      return;
    case 31:
      hideBigBtns();
      setSTEP();  // use touchbuttons
      break;
    case 32:
      hideBigBtns();
      setBandwidth(0);  // use touchbuttons
      tRel();
      break;
    case 33:
      hideBigBtns();
      showMemo(false, 1);
      writeMemo();
      tRel();
      break;
    case 34:
      hideBigBtns();
      showMemo(true, 1);
      readMemo();
      tft.fillRect(0, 294, 374, 25, TFT_BLACK);  // overwrite remanents of station names
      displayDebugInfo = true;                   // restore debug info
      tRel();
      break;
    case 41:
      hideBigBtns();
      SecScreen();
      tRel();
      break;
    case 42:
      ScanMode();
      tRel();
      tx = ty = pressed = 0;
      break;
    case 43:
      hideBigBtns();
      setBand(false);  // Select Band
      tRel();
      break;
    case 44:
      hideBigBtns();
      freqScreen();
      break;
    default:
      tx = ty = pressed = 0;
      tRel();
      return;
  }
}

//##########################################################################################################################//

void setBandwidth(int mode) {  // mode 0 = bandwidth selected from menu. mode -1 and 1 are called  by touching the indicator areas
  const int TEXT_Y = 195;
  const int positions[] = { 24, 106, 191, 272 };

  const uint8_t usbLsbBandWidths[] = { 0, 1, 2, 3 };
  const uint8_t amBandWidths[] = { 3, 2, 1, 0 };
  const uint8_t cwBandWidths[] = { 4, 5, 3, 3 };

  const char *amBandwidth[] = { "2KHz", "3KHz", "4KHz", "6KHz" };
  const char *ssbBandwidth[] = { "1.2KHz", "2.2KHz", "3KHz", "4KHz" };
  const char *cwBandwidth[] = { "0.5KHz", "1.0KHz", "", "" };
  const char **bandwidth = nullptr;



  if (modType == WBFM)
    return;

  if (mode == 0) {
    for (int j = 0; j < 4; j++) {
      drawButton(8 + j * 83, 235, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
      drawButton(8 + j * 83, 178, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
    }

    tft.fillRect(3, 119, 334, 58, TFT_BLACK);  // overwrite highest row

    etft.setTTFFont(Arial_14);
    etft.setTextColor(TFT_GREEN);

    if (modType == AM)
      bandwidth = amBandwidth;
    else if (modType == USB || modType == LSB || modType == SYNC)
      bandwidth = ssbBandwidth;
    else if (modType == CW)
      bandwidth = cwBandwidth;


    if (bandwidth != nullptr) {
      for (int i = 0; i < 4; ++i) {

        if (modType == AM)
          etft.setCursor(positions[i], TEXT_Y);
        if (modType == LSB || modType == USB || modType == SYNC || modType == CW)
          etft.setCursor(positions[i] - 10, TEXT_Y);  // text is wider

        etft.print(bandwidth[i]);
      }
      etft.setTextColor(textColor);  // Reset to default text color
    }


    tDoublePress();

    delay(10);  // Wait until pressed

    row = 1 + ((ty - 20) / vTouchSpacing);
    column = 1 + (tx / HorSpacing);


    if (row == 3) {

      if (modType == USB || modType == LSB || modType == SYNC) {
        bandWidth = usbLsbBandWidths[column - 1];
        lastSSBBandwidth = bandWidth;
      } else if (modType == AM) {
        bandWidth = amBandWidths[column - 1];
        lastAMBandwidth = bandWidth;
      } else if (modType == CW) {
        bandWidth = cwBandWidths[column - 1];
        lastSSBBandwidth = bandWidth;
      }
    }

  }  // endif mode == 0


  else {  // mode -1 and mode 1 are used when touching the indicator area on the left or right

    const uint8_t *bandWidths;
    int maxIndex = 3;
    int i = 0;
    bool isIncrement = (mode == 1);

    if (modType == AM) {
      bandWidths = amBandWidths;
    } else if (modType == USB || modType == LSB || modType == SYNC) {
      bandWidths = usbLsbBandWidths;
    } else if (modType == CW) {
      bandWidths = cwBandWidths;
      maxIndex = 1;
    } else {
      return;
    }

    // find index
    while (i < maxIndex && bandWidths[i] != bandWidth) {
      i++;
    }

    // modify index
    if (isIncrement && i < maxIndex) {
      i++;
    } else if (!isIncrement && i > 0) {
      i--;
    }

    // update bandwidth
    bandWidth = bandWidths[i];
  }
  // set bandwidth
  if (modType == LSB || modType == USB || modType == SYNC || modType == CW) {
    si4735.setSSBAudioBandwidth(bandWidth);
    if (bandWidth <= 2) {
      si4735.setSBBSidebandCutoffFilter(0);
    } else {
      si4735.setSBBSidebandCutoffFilter(1);
    }
  }

  if (modType == AM) {
    si4735.setBandwidth(bandWidth, 1);
  }
  printBandWidth();
}

//##########################################################################################################################//

void printBandWidth() {

  if (altStyle)
    tft.fillRoundRect(75, 95, 92, 22, 10, TFT_BLUE);

  else {
    tft.pushImage(75, 92, 92, 26, (uint16_t *)Oval92);
  }

  tft.setTextColor(TFT_GREEN);
  tft.setCursor(77, 98);

  switch (modType) {
    case WBFM:
      tft.print(" 300KHz");
      break;
    case NBFM:
      tft.print(" 10KHz");
      break;
    case CW:
      if (bandWidth == 4)
        tft.print(" 0.5KHz");
      if (bandWidth == 5)
        tft.print(" 1.0KHz");
      break;
    case USB:
    case LSB:
    case SYNC:
      tft.print(bandWidth == 0 ? " 1.2KHz" : bandWidth == 1 ? " 2.2KHz"
                                           : bandWidth == 3 ? " 4.0KHz"
                                                            : " 3.0KHz");
      break;
    case AM:
      tft.print(bandWidth == 0 ? " 6.0KHz" : bandWidth == 1 ? " 4.0KHz"
                                           : bandWidth == 2 ? " 3.0KHz"
                                                            : " 2.0KHz");
      break;
  }
}
//##########################################################################################################################//


void showIFBandwidth() {


  if (altStyle)
    tft.fillRoundRect(174, 95, 45, 22, 10, TFT_BLUE);

  else
    tft.pushImage(170, 92, 55, 26, (uint16_t *)Oval55);


  tft.setTextColor(TFT_GREEN);
  tft.setCursor(179, 98);

  if (wideIFFilter) {
    tft.setTextColor(TFT_YELLOW);
    tft.print("IFW");
  } else {
    tft.setTextColor(TFT_GREEN);
    tft.print("IFN");
  }

  tft.setTextColor(textColor);
}


//##########################################################################################################################//

void ScanMode() {

  scanMode = !scanMode;
}

//##########################################################################################################################//

int getButtonID(void) {


  if (ty > 293)  // no buttons there
    return 0;
  column = 1 + (tx / HorSpacing);  // get row and column
  row = 1 + ((ty - 35) / vTouchSpacing);
  int buttonID = row * 10 + column;
  return buttonID;
}

//##########################################################################################################################//

void setIFBandwidth() {

  wideIFFilter = !wideIFFilter;
  if (wideIFFilter)
    digitalWrite(IF_FILTER_BANDWIDTH_PIN, HIGH);
  else
    digitalWrite(IF_FILTER_BANDWIDTH_PIN, LOW);

  showIFBandwidth();
}


//###############################################################################################//

void pacM(bool jumpBack) {
  static bool openMouth = false;
  static bool movRight = true;
  static bool movingDown = true;
  const int movHor = 5;
  const int movVer = 15;
  static int xPos = 5;
  const int yPosStart = 85;
  const int yPosEnd = 275;
  static int yPos = yPosStart;
  const int spriteSize = 24;

  drawRandomObjects();

  // Clear prev pos
  tft.fillRect(xPos, yPos, spriteSize, spriteSize, TFT_BLACK);

  // Update pos
  xPos += movRight ? movHor : -movHor;

  // Draw sprite
  if (openMouth) {
    tft.pushImage(xPos, yPos, spriteSize, spriteSize,
                  movRight ? pr : pl);

    if (random(200) == 1 && !movRight && xPos < 280) {  // green gas
      tft.pushImage(xPos + 20, yPos + 10, 20, 10, grS);
      delay(100);
      tft.pushImage(xPos, yPos, spriteSize, spriteSize,
                    pcRed);
      delay(1000);
      tft.fillRect(xPos, yPos, spriteSize, spriteSize, TFT_BLACK);
      if (xPos >= 5 + 2 * movHor)
        xPos -= 2 * movHor;
    }

  }

  else {
    tft.pushImage(xPos, yPos, spriteSize, spriteSize,
                  movRight ? prc : plc);
  }

  openMouth = !openMouth;

  if (jumpBack && get_Touch()) {
    tx = ty = pressed = 0;
    tRel();
  }

  // Boundary check
  if (xPos >= 312 || xPos <= 5) {
    tft.fillRect(xPos, yPos, spriteSize, spriteSize, TFT_BLACK);
    movRight = !movRight;

    // Update vertical pos
    if (movingDown) {
      yPos += movVer;
    } else {
      yPos -= movVer;
    }

    // check direction limits
    if (yPos > yPosEnd) {
      movingDown = false;
      yPos -= movVer;
    } else if (yPos <= yPosStart) {
      movingDown = true;
      yPos += movVer;
    }
  }
}
//###############################################################################################//
void drawRandomObjects() {

  if (!funEnabled)
    return;

  const int yPosStart = 95;
  const int yShift = 24;

  if (1 == random(600))
    tft.pushImage(5 + random(290), yPosStart + random(6) * yShift, 30, 30, apple);
  if (1 == random(600))
    tft.pushImage(5 + random(290), yPosStart + random(6) * yShift, 30, 30, cherry);
  if (1 == random(600))
    tft.pushImage(5 + random(290), yPosStart + random(6) * yShift, 30, 30, strawberry);
  if (1 == random(600))
    tft.pushImage(5 + random(290), yPosStart + random(6) * yShift, 40, 30, radio);
  if (1 == random(600))
    tft.pushImage(5 + random(290), yPosStart + random(6) * yShift, 30, 40, dino);
  if (1 == random(600))
    tft.pushImage(5 + random(290), yPosStart + random(6) * yShift, 20, 30, mush);
  if (1 == random(600))
    tft.pushImage(5 + random(290), yPosStart + random(6) * yShift, 40, 30, cam);
}

//###############################################################################################//
void resetMainScreen(void) {

  redrawMainScreen = true;
  tx = ty = pressed = 0;

}
//###############################################################################################//

