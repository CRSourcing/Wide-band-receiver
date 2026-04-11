#ifdef SHOW_DEBUG_UTILITIES

void DebugScreen() {  // Contains debug routines and unfinished ideas


  if (!altStyle)  // clear  background
    tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  else
    drawButton(2, 61, 337, 228, TFT_NAVY, TFT_DARKGREY);  //Background
  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);
  etft.setCursor(10, 65);
  etft.print(F("Debug area, for testing."));

  redrawMainScreen = true;
  drawDbgBtns();
  readDbgBtns();
}

//##########################################################################################################################//

void drawDbgBtns() {

  hideBigBtns();
  displayLogsFromBuffer(348, 63);


  struct Button {
    const int x;
    const int y;
    const char* label;
  };

  const Button buttons[] PROGMEM = {
    { 183, 245, "SPI" },
    { 183, 268, "Noise" },
    { 100, 245, "AGC" },
    { 100, 268, "Rel." },
    { 20, 268, "" },
    { 20, 245, "" },
    { 190, 188, "I2C" },
    { 180, 210, "Speed" },
    { 270, 188, "L0 <" },
    { 270, 210, "IF" },
    { 20, 188, "No" },
    { 20, 210, "Mixer" },
    { 100, 188, "Sine" },
    { 100, 210, "Tone" },
    { 183, 132, "TSA" },
    { 183, 152, "Serial" },
    { 100, 132, "LO" },
    { 100, 152, "Drive" },
    { 20, 132, "Touch" },
    { 20, 152, "Coord." },
    { 270, 132, "Show" },
    { 270, 152, "ADCs" },
    { 270, 245, "Set" },
    { 270, 268, "DAC1" }
  };



  etft.setTTFFont(Arial_14);
  etft.setTextColor(TFT_YELLOW);


  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].label);
  }


  drawButton(8, 236, 75, 49, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(20, 255);
  etft.print(F("BACK"));
  etft.setTextColor(textColor);
  tDoublePress();
}
//##########################################################################################################################//

void readDbgBtns() {


  if (!pressed)
    return;
  int buttonID = getButtonID();

  if (!buttonID)  // outside of area
    return;

  switch (buttonID) {

    case 21:
      showTouchCoordinates = !showTouchCoordinates;
      break;
    case 22:
      setLOLevel();
      break;
    case 23:
#ifdef TINYSA_PRESENT
      serialDebugTSA = !serialDebugTSA;
#endif
      break;
    case 24:
      showADCs();
      tft.fillRect(0, 0, 479, 319, TFT_BLACK);
      redrawMainScreen = true;
      spriteBorder();
      break;
    case 31:
      noMixer = !noMixer;
      if (noMixer) {
        tft.setCursor(10, 105);
        tft.print(F("SI4732 direct tuning active"));
      }
      delay(1000);
      break;
    case 32:
      setDac2();
      break;
    case 33:
      setI2CBusSpeed();
      break;
    case 34:
      lowSideInjection = !lowSideInjection;
      if (lowSideInjection) {
        tft.setCursor(10, 105);
        tft.print(F("LO frequency below RF"));
        delay(1000);
      }
      FREQ_OLD--;  // trigger display update
      break;
    case 41:
      return;
    case 42:
      setAGCReleaseRate();
      break;
    case 43:
      tRel();
      while (true) {  // produce noise on the SPI bus
        long x = random(480);
        long y = random(320);
        uint32_t val = random(0xFFFF);
        tft.drawPixel(x, y, val);
      }
      break;
    case 44:
      setDac1();
      break;
    default:
      resetMainScreen();
      return;
  }

  redrawMainScreen = true;
  tRel();
}


//##########################################################################################################################//

void setLOLevel() {

  displayText(10, 70, 300, 25, "Select SI5351 output level:");

  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);

  struct Button {
    int x;
    int y;
    const char* label;
  };

  const Button buttons[] PROGMEM = {
    { 203, 245, "6" },
    { 193, 265, "dBm" },
    { 120, 245, "3" },
    { 110, 265, "dBm" },
    { 290, 245, "10" },
    { 280, 265, "dBm" },
    { 20, 188, "" },
    { 20, 210, "" },
    { 20, 132, "" },
    { 20, 152, "" },
    { 20, 255, "BACK" }
  };


  etft.setTTFFont(Arial_14);
  etft.setTextColor(TFT_YELLOW);

  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].label);
  }


  while (true) {
    tDoublePress();
    int buttonID = getButtonID();
    if (!buttonID)
      return;  // outside of area

    switch (buttonID) {
      case 41:
        resetMainScreen();
        return;
      case 42:
        si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA);
        si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_2MA);
        si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_2MA);
        displayText(10, 95, 300, 20, "3 dBm set ");
        break;
      case 43:
        si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_4MA);
        si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_4MA);
        si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_4MA);
        displayText(10, 95, 300, 20, "6 dBm set ");
        break;
      case 44:
        si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
        si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);
        si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_8MA);
        displayText(10, 95, 300, 20, "10 dBm set");
        break;

      default:
        resetMainScreen();
        return;
    }
  }
}



//##########################################################################################################################//
void setI2CBusSpeed() {

  encLockedtoSynth = false;
  clearStatusBar();
  displayText(10, 80, 0, 0, "Set I2C bus speed");


  while (digitalRead(ENCODER_BUTTON) == HIGH) {


    if (clw + cclw) {

      delay(50);
      if (clw)
        I2C_BUSSPEED += 100000;
      if (cclw)
        I2C_BUSSPEED -= 100000;

      Wire.end();
      delay(20);
      Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL, I2C_BUSSPEED);
      tft.fillRect(5, 105, 320, 17, TFT_BLACK);
      tft.setCursor(5, 105);
      tft.printf("Busspeed: %ld KHz", I2C_BUSSPEED / 1000);


      clw = false;
      cclw = false;
    }
  }

  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;

  encLockedtoSynth = true;
  clearStatusBar();
}


//##########################################################################################################################//


void setAGCReleaseRate() {

  encLockedtoSynth = false;
  clearStatusBar();
  displayText(5, 80, 0, 0, "Set AM AGC Release Rate");
  int val = preferences.getInt("amagc", 4);

  while (digitalRead(ENCODER_BUTTON) == HIGH) {


    if (clw + cclw) {

      delay(50);
      if (clw)
        val += 4;
      if (cclw)
        val -= 4;

      if (val < 4)
        val = 4;

      if (val > 248)
        val = 248;


      si4735.setAmAgcReleaseRate(val);

      tft.fillRect(5, 105, 320, 17, TFT_BLACK);
      tft.setCursor(5, 105);
      tft.printf("AM AGC Release Rate %d", val);

      clw = false;
      cclw = false;
    }
  }


  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;

  //preferences.putInt("amagc", val);
  encLockedtoSynth = true;
  clearStatusBar();
}



//##########################################################################################################################//
void initRollingGraphSprite() {

  spr2.setColorDepth(16);
  spr2.createSprite(251, 26);
  spr2.fillSprite(TFT_BLACK);
  tft.setTextSize(1);
  spr2.print(F("Signal Strength history. Tap to disable."));
  tft.setTextSize(2);
  sprite2Init = true;
}


//##########################################################################################################################//
void RSSITrace() {

  static uint8_t interval = 0;
  if (tx > 240 && ty > 293 && !syncEnabled) {
    showRSSITrace++;
    if (showRSSITrace >= 2)
      showRSSITrace = 0;
    sprite2Init = false;
    spr2.deleteSprite();
    preferences.putChar("showT", showRSSITrace);
    tft.fillRect(230, 294, 250, 26, TFT_BLACK);
    tRel();
    tx = ty = 0;
    pressed = false;
  }

  if (syncEnabled)
    return;


  if (showRSSITrace && ++interval == 3) {
    interval = 0;
    drawRollingGraph(signalStrength);
  }
}

//##########################################################################################################################//


void drawRollingGraph(char bVal)  // draw a rolling RSSI graph in lower right corner.  Slowly autoadjusts to range.
{


  if (!showRSSITrace)
    return;

  if (!sprite2Init)
    initRollingGraphSprite();

  const int startX = 230;
  const int endX = 480;
  const int minY = 294;
  const int maxY = 319;

  static int minTracked = 50;
  static int maxTracked = 100;


  const int width = endX - startX + 1;
  const int height = maxY - minY + 1;


  static bool init = false;
  static int lastY = height - 1;


  if (!init) {
    minTracked = bVal;
    maxTracked = bVal;
    init = true;
  }


  // quick adjust to new hi/low
  if (bVal < minTracked) minTracked--;
  if (bVal > maxTracked) maxTracked++;

  // slow update for current signal

  if (minTracked < bVal)
    minTracked += (bVal - minTracked) >> 5;  // slow upward drift

  if (maxTracked > bVal)
    maxTracked -= (maxTracked - bVal) >> 5;  // slow downward drift

  int displayBottomMargin = 2;

  int usableHeight = height - displayBottomMargin;

  int range = maxTracked - minTracked;
  if (range < 1)
    range = 1;  // avoid later divide by zero

  // scale current value
  int scaled = (bVal - minTracked) * usableHeight / range;

  // invert
  int newY = (height - displayBottomMargin) - scaled;

  //scroll sprite lef
  spr2.scroll(-1, 0);

  // clear right column
  spr2.drawLine(width - 1, 0, width - 1, height - 1, TFT_BLACK);

  // connect

  spr2.drawPixel(width - 1, lastY - 1, TFT_GREEN);
  spr2.drawLine(width - 2, lastY + 1, width - 1, maxY, TFT_BLUE);

  lastY = newY;

  // push it
  spr2.pushSprite(startX, minY);
}

//##########################################################################################################################//

//GPIO 36 + 39 in an oscilloscope configuration


// shows GPIO 36 + 39 in an oscilloscope style
void showADCs() {
  const int width = 300;
  const int trH = 64;
  const int yPos1 = 100;
  const int yPos2 = 256;

  // Allocate buffers
  uint16_t* screenBuf1 = (uint16_t*)malloc(width * trH * sizeof(uint16_t));
  uint16_t* screenBuf2 = (uint16_t*)malloc(width * trH * sizeof(uint16_t));

  if (!screenBuf1 || !screenBuf2) {
    Serial.println("Malloc failed!");
    free(screenBuf1);
    free(screenBuf2);
    return;
  }

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);

  // labels
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(50 + width + 4, yPos1 - trH + 0);
  tft.print(F("3.3V"));
  tft.setCursor(50 + width + 4, yPos1 - trH + 16);
  tft.print(F("2.5V"));
  tft.setCursor(50 + width + 4, yPos1 - trH + 32);
  tft.print(F("1.7V"));
  tft.setCursor(50 + width + 4, yPos1 - trH + 48);
  tft.print(F("0.8V"));
  tft.setCursor(50 + width + 4, yPos1 - trH + 63);
  tft.print(F("0V"));


  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(50 + width + 4, yPos2 - trH + 0);
  tft.print(F("3.3V"));
  tft.setCursor(50 + width + 4, yPos2 - trH + 16);
  tft.print(F("2.5V"));
  tft.setCursor(50 + width + 4, yPos2 - trH + 32);
  tft.print(F("1.7V"));
  tft.setCursor(50 + width + 4, yPos2 - trH + 48);
  tft.print(F("0.8V"));
  tft.setCursor(50 + width + 4, yPos2 - trH + 63);
  tft.print(F("0V"));

  tft.setTextSize(2);
  tft.setTextColor(textColor);
  tft.setSwapBytes(true);
  tft.setCursor(0, 0);
  tft.print(F("GPIO 36+39, ~5ms/div, move enc. to leave"));

  while (!(clw + cclw)) {  // leave when encoder moved
    memset(screenBuf1, 0, width * trH * sizeof(uint16_t));
    memset(screenBuf2, 0, width * trH * sizeof(uint16_t));

    // Grids
    for (int y = 0; y <= trH - 1; y += 16) {
      for (int x = 0; x < width; x += 2) {
        screenBuf1[y * width + x] = TFT_DARKGREY;
        screenBuf2[y * width + x] = TFT_DARKGREY;
      }
    }
    for (int x = 0; x <= width - 1; x += 50) {
      for (int y = 0; y < trH; y += 2) {
        screenBuf1[y * width + x] = TFT_DARKGREY;
        screenBuf2[y * width + x] = TFT_DARKGREY;
      }
    }

    for (int x = 0; x < width; x += 2) {
      screenBuf1[(trH - 1) * width + x] = TFT_DARKGREY;
      screenBuf2[(trH - 1) * width + x] = TFT_DARKGREY;
    }
    for (int y = 0; y < trH; y += 2) {
      screenBuf1[y * width + (width - 1)] = TFT_DARKGREY;
      screenBuf2[y * width + (width - 1)] = TFT_DARKGREY;
    }

    // Write and use Besenham to connect
    int prevY1 = trH - 1 - (analogRead(36) >> 6);
    int prevY2 = trH - 1 - (analogRead(39) >> 6);

    for (int x = 1; x < width; x++) {
      int currY1 = trH - 1 - (analogRead(36) >> 6);
      int currY2 = trH - 1 - (analogRead(39) >> 6);

      int dy1 = abs(currY1 - prevY1);
      int sy1 = (prevY1 < currY1) ? 1 : -1;
      int y1 = prevY1;
      for (int i = 0; i <= dy1; i++) {
        if (y1 >= 0 && y1 < trH)
          screenBuf1[y1 * width + x] = TFT_WHITE;
        y1 += sy1;
      }

      int dy2 = abs(currY2 - prevY2);
      int sy2 = (prevY2 < currY2) ? 1 : -1;
      int y2 = prevY2;
      for (int i = 0; i <= dy2; i++) {
        if (y2 >= 0 && y2 < trH)
          screenBuf2[y2 * width + x] = TFT_GREEN;
        y2 += sy2;
      }

      prevY1 = currY1;
      prevY2 = currY2;
    }


    // Push buffs
    tft.pushImage(20, yPos1 - trH, width, trH, screenBuf1);
    tft.pushImage(20, yPos2 - trH, width, trH, screenBuf2);
  }

  // reboot, avoid memory fragmentation
  preferences.putBool("fB", true);
  ESP.restart();
}

//##########################################################################################################################//




void setDac1() {  // set tuner gain control manually, DAC1 is connected to tuner AGC pin
  uint8_t oldDAC = 0;
  uint8_t DAC = 200;
  encLockedtoSynth = false;

  tft.setTextColor(textColor);
  tft.fillRect(3, 62, 330, 230, TFT_BLACK);
  tft.setCursor(10, 65);
  tft.print(F("Set DAC1"));

  while (digitalRead(ENCODER_BUTTON) == HIGH) {

    if (DAC != oldDAC) {
      tft.fillRect(10, 107, 323, 20, TFT_BLACK);
      tft.setCursor(10, 107);
      tft.printf("DAC1: %d", DAC);
      dac1.outputVoltage((uint8_t)DAC);  // set the dac
      oldDAC = DAC;
    }

    delay(50);
    if (clw)
      DAC += 5;
    if (cclw)
      DAC -= 5;

    clw = false;
    cclw = false;
  }

  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;
  encLockedtoSynth = true;
  clearStatusBar();
}

//##########################################################################################################################//
void setDac2() {  //generate sine tone for audio testing
  uint16_t oldF = 0;
  uint16_t F = 800;
  encLockedtoSynth = false;
  dac2.enable();

  tft.setTextColor(textColor);
  tft.fillRect(3, 62, 330, 230, TFT_BLACK);
  tft.setCursor(10, 65);
  tft.print(F("Sine tone for audio test"));
  while (digitalRead(ENCODER_BUTTON) == HIGH) {

    if (F != oldF) {
      tft.fillRect(10, 130, 323, 16, TFT_BLACK);
      tft.setCursor(10, 130);
      tft.printf("FREQ: %d", F);

      dac2.outputCW(F);

      oldF = F;
    }

    delay(50);
    if (clw)
      F += 100;
    if (cclw)
      F -= 100;

    clw = false;
    cclw = false;
  }

  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;
  encLockedtoSynth = true;
  dac2.disable();
  clearStatusBar();
}


#endif
