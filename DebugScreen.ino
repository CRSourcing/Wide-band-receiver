#ifdef SHOW_DEBUG_UTILITIES

void DebugScreen() {  // Contains debug routines and unfinished ideas


  if (!altStyle)  // clear  background
    tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  else
    drawButton(2, 61, 337, 228, TFT_NAVY, TFT_DARKGREY);  //Background
  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);

  etft.setCursor(10, 65);
  etft.print("Debug area, for testing.");

  redrawMainScreen = true;
  drawDbgBtns();
  readDbgBtns();
}

//##########################################################################################################################//

void drawDbgBtns() {

  hideBigBtns();
  displayLogsFromBuffer(348, 63);


  if (altStyle)
    etft.setTextColor(textColor);
  else
    etft.setTextColor(TFT_ORANGE);

  struct Button {
    int x;
    int y;
    const char* label;
  };

  Button buttons[] = {
    { 183, 245, "SPI" },
    { 183, 268, "Noise" },
    { 100, 245, "AGC" },
    { 100, 268, "Rel." },
    { 20, 268, "" },
    { 20, 245, "" },
    { 190, 188, "I2C" },
    { 180, 210, "Speed" },
    { 270, 188, "Play" },
    { 270, 210, ".wav" },
    { 20, 188, "No" },
    { 20, 210, "Mixer" },
    { 100, 188, "AGC" },
    { 100, 210, "Graph" },
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
  etft.print("BACK");
  etft.setTextColor(textColor);
  tDoublePress();
}
//##########################################################################################################################//

void readDbgBtns() {


  if (!pressed) return;
  int buttonID = getButtonID();

  if (row < 2 || row > 4 || column > 4)
    return;  // outside of area

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
        tft.print("SI4732 direct tuning active");
      }
      delay(1000);
      break;
    case 32:
      showAGCGraph = !showAGCGraph;
      if (showAGCGraph) {
        tft.setCursor(10, 105);
        tft.print("AGC trace enabled");
        delay(500);
      }
      break;
    case 33:
      setI2CBusSpeed();
      break;
    case 34:
      wavPlayer();
      break;
    case 41:
      return;
    case 42:
      setAGCReleaseRate();
      break;
    case 43:
      tRel();
      while (1) {  // produce noise on the SPI bus
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

  Button buttons[] = {
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

    if (row < 2 || row > 4 || column > 4)
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

void drawByteTrace(char bVal, char maxBVal) {
  // displays bVal as trace. slowly auto adjusts to max bVal
  if (!showAGCGraph)
    return;



  const int startX = 230;
  const int endX = 440;
  const int minY = 294;
  const int maxY = 319;
  const int maxPixelHeight = 25;
  const int bufferSize = endX - startX + 1;  // Number of pixels in graph width

  // Circular buffer for prev values
  static uint8_t valueBuffer[215] = { 0 };  // 450-240+1 = 211 elements
  static int pixBuffer[480] = { 0 };
  static int bufferIndex = 0;
  static bool bufferFull = false;

  static char maxByteValue = maxBVal;
  static unsigned long lastDecayTime = 0;
  const unsigned long decayInterval = 1000;
  const int decayRate = 1;

  // Update peak if current value is >
  if (bVal > maxByteValue) {
    maxByteValue = bVal;
  }

  // Slow decay
  unsigned long currentTime = millis();
  if (currentTime - lastDecayTime > decayInterval) {
    lastDecayTime = currentTime;
    if (maxByteValue > bVal) {
      maxByteValue -= decayRate;
      if (maxByteValue < bVal) {
        maxByteValue = bVal;
      }
    }
  }

  // Store value in circular buffer
  valueBuffer[bufferIndex] = bVal;
  bufferIndex++;

  // Wrap when buffer full
  if (bufferIndex >= bufferSize) {
    bufferIndex = 0;
    bufferFull = true;
  }


  // Clear the graph area
  tft.fillRect(startX, minY, endX - startX + 1, maxY - minY + 1, TFT_BLACK);

  // Draw rolling graph
  int x = startX;
  int valuesToDraw = bufferFull ? bufferSize : bufferIndex;
  int startDrawIndex = bufferFull ? bufferIndex : 0;

  for (int i = 0; i < valuesToDraw; i++) {
    // Calculate circular buffer index
    int valueIndex = (startDrawIndex + i) % bufferSize;
    char currentValue = valueBuffer[valueIndex];

    // Calculate height for recent value
    int differenceFromMax = maxByteValue - currentValue;
    int compressedHeight = 0;

    if (differenceFromMax > 0) {
      // lin scaling for small difference
      if (differenceFromMax <= 5) {
        compressedHeight = differenceFromMax * 2;  // 0->0, 1->2, 2->4, 3->6, 4->8, 5->10 pixels
      }
      // log scaling for bigger difference
      else {
        static const uint8_t logScale[] = { 10, 12, 14, 16, 18, 20, 21, 22, 23, 24, 25 };
        int logIndex = differenceFromMax - 6;  // Map 6->0, 7->1,..
        if (logIndex < 0) logIndex = 0;
        if (logIndex <= 10) {
          compressedHeight = logScale[logIndex];
        } else {
          compressedHeight = maxPixelHeight;
        }
      }
    }

    // Draw pix

    pixBuffer[x] = maxY - compressedHeight;

    if (pixBuffer[x - 1] > minY)
      tft.drawLine(x - 1, pixBuffer[x - 1], x, pixBuffer[x], TFT_GREEN);
    x++;
  }

  // Update numeric display every 5x
  static char dCounter = 0;
  dCounter++;
  if (dCounter >= 5) {
    tft.fillRect(450, 300, 29, 19, TFT_BLACK);
    tft.setCursor(450, 310);
    tft.setTextSize(1);
    tft.printf("%d", bVal);
    tft.setTextSize(2);
    dCounter = 0;
  }
}

//##########################################################################################################################//
// shows GPIO 36 + 39 in an oscilloscope style
void showADCs() {
  const int width = 400;
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
  tft.print("2.5V");
  tft.setCursor(50 + width + 4, yPos1 - trH + 16);
  tft.print("2.1V");
  tft.setCursor(50 + width + 4, yPos1 - trH + 32);
  tft.print("1.7V");
  tft.setCursor(50 + width + 4, yPos1 - trH + 48);
  tft.print("1.2V");
  tft.setCursor(50 + width + 4, yPos1 - trH + 63);
  tft.print("0.8V");


  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(50 + width + 4, yPos2 - trH + 0);
  tft.print("3.3V");
  tft.setCursor(50 + width + 4, yPos2 - trH + 16);
  tft.print("2.5V");
  tft.setCursor(50 + width + 4, yPos2 - trH + 32);
  tft.print("1.7V");
  tft.setCursor(50 + width + 4, yPos2 - trH + 48);
  tft.print("0.8V");
  tft.setCursor(50 + width + 4, yPos2 - trH + 63);
  tft.print("0V");

  tft.setTextSize(2);
  tft.setTextColor(textColor);
  tft.setSwapBytes(true);
  tft.setCursor(0, 0);
  tft.print("GPIO 36+39, ~5ms/div, move enc. to leave");

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

    // Write and use Besenham algorithm to connect
    int prevY1 = trH - 1 - ((analogRead(36) - dcOffset) >> 5) - 32;
    int prevY2 = trH - 1 - (analogRead(39) >> 6);

    for (int x = 1; x < width; x++) {
      int currY1 = trH - 1 - ((analogRead(36) - dcOffset) >> 5) - 32;  // use double vertical resolution
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

  // needs reboot since buffers use memory already allocated for sprites!?
  preferences.putBool("fB", true);
  ESP.restart();
}

//##########################################################################################################################//

void setDac1() {  // set tuner gain control manually, DAC1 is connected to tuner AGC pin
  int oldDAC = 0;
  static int DAC = 200;
  encLockedtoSynth = false;

  tft.fillCircle(155, 162, 3, TFT_BLACK);  // overwrite strength indicator
  tft.setTextColor(textColor);
  tft.fillRect(10, 62, 325, 55, TFT_BLACK);
  tft.setCursor(10, 65);
  tft.print("Set DAC1");

  while (digitalRead(ENCODER_BUTTON) == HIGH) {

    if (DAC != oldDAC) {
      tft.fillCircle(155, 162, 3, TFT_BLACK);

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

    if (DAC >= 255)
      DAC = 255;
    if (DAC < 0)
      DAC = 0;


    clw = false;
    cclw = false;
  }

  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;
  encLockedtoSynth = true;
  clearStatusBar();
}

#endif
