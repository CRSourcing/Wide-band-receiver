//##########################################################################################################################//
// Next functions are to draw a slow scan waterfall, resolution is limited to 240 * 231 due to amount of framebuffer needed
// IlI9488 display does not allow to read back from the hardware framebuffer when touchscreen is enabled, so a sw buffer is needed.
// Typical draw time is btw. 10 minutes and several hours for one screen
// Uses fine tune potentiometer to adjust colors

void waterFall(bool useKeypad) {


  if (useKeypad) {
    if (!getFreqLimits())  // exit, keypad return was pressed
      return;
  }

  pressed = false;
  long endPoint = lim1;
  long startPoint = lim2;
#ifdef TINYSA_PRESENT
  long midPoint = (endPoint + startPoint) / 2;  // calculate midpoint, needed to sync the tinySA
#endif
  float span = (float)(endPoint - startPoint);


  // allocate 2 framebuffers, 120 * 300, can't allocate 1 framebuffer big enough
  framebuffer1 = (uint16_t*)malloc(FRAMEBUFFER_HALF_WIDTH * FRAMEBUFFER_HEIGHT * sizeof(uint16_t));
  framebuffer2 = (uint16_t*)malloc(FRAMEBUFFER_HALF_WIDTH * FRAMEBUFFER_HEIGHT * sizeof(uint16_t));

  if (framebuffer1 == NULL || framebuffer2 == NULL)
    buffErr();

  tft.fillScreen(TFT_BLACK);

  wIntro(startPoint, endPoint);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(5, 255);
  tft.print("While scanning, touch bar below to listen.Touch area above to scan again.");
  tft.setTextSize(2);
  tft.setTextColor(textColor);
  // Initialize framebuffers
  memset(framebuffer1, 0, FRAMEBUFFER_HALF_WIDTH * FRAMEBUFFER_HEIGHT * sizeof(uint16_t));
  memset(framebuffer2, 0, FRAMEBUFFER_HALF_WIDTH * FRAMEBUFFER_HEIGHT * sizeof(uint16_t));


#ifdef TINYSA_PRESENT  // sync tinySA with watefall
  synctinySAWaterfall(midPoint, startPoint, endPoint);
#endif

  tft.setTextColor(textColor);


  int xPos = 0;
  float accum = 0;
  long offset = 0;
  long total = 0;
  long strength = 0;
  uint32_t startTime;
  uint16_t resolution = 2000;  // 2KHz
  bool drawingTrace = true;
  long cellSize = (endPoint - startPoint) / FRAMEBUFFER_FULL_WIDTH;  // cellSize is the frequency range that gets scanned for 1 pixel
  uint16_t stp = cellSize / 25;                                      // can't change filter bandwith, need to scan each cell. 25 steps in every cellSize sufficient up to 30 MHz range

  if (stp < resolution)  // stepsize below resolution makes no sense, so we change it to resolution
    stp = resolution;
  if (stp > 5000)
    stp = 5000;  // for wide scans > 30MHz

  displayDebugInfo = false;  // not show infobar
  showFREQ = false;          // do not show FREQ display



  // draws a scale and 10 frequency marks on top of the screen
  float scale = 0;
  tft.setTextSize(1);
  tft.drawFastHLine(0, 0, WATERFALL_SCREEN_WIDTH, TFT_WHITE);
  tft.drawFastVLine(WATERFALL_SCREEN_WIDTH - 1, 0, 5, TFT_WHITE);
  for (int i = 0; i < WATERFALL_SCREEN_WIDTH; i += WATERFALL_SCREEN_WIDTH / 10) {
    tft.drawFastVLine(i, 0, 5, TFT_WHITE);
    tft.setCursor(i, 8);
    if (i < WATERFALL_SCREEN_WIDTH)
      tft.printf("%.2f", (scale + startPoint) / 1000000.0);
    scale += span / 10;
  }

  tft.setTextSize(2);



  /// loop that reads RSSI and feeds the framebuffers
  while (drawingTrace) {

    if (listenToWaterfall(endPoint, startPoint))  // returns true when SET was touched
      return;

    if (!xPos) {
      startTime = millis();  // Start the timer
      tft.fillRect(330, 300, 150, 19, TFT_BLACK);
      tft.setCursor(330, 303);
      tft.setTextColor(TFT_RED);
      tft.print("SCANNING");
    }


    int mult = (analogRead(FINETUNE_PIN) / wfSensitivity) - 1;
    long corrFactor = (cellSize / 2000); // ajust accum to cellsize
    corrFactor = min(corrFactor, (long)50);  // Limit corrFactor to 50

    while (offset < cellSize) {
      FREQ = startPoint + total + offset;
      setLO();
      si4735.getCurrentReceivedSignalQuality(0);
      strength = si4735.getCurrentRSSI();  // Read RSSI

      accum += abs((strength * mult) / (corrFactor + 1));
      offset += stp;
    }

    uint16_t clr = valueToWaterfallColor(((int)accum - 600) * 3);  // Convert to RGB565
    tft.fillCircle(470, 312, 4, clr);                              // Lower right corner indicator
    tft.fillRect(2 * xPos, 265, 2, 6, clr);                        // indicator bar
    tft.drawPixel(2 * xPos, 0, TFT_BLUE);                          //  upper bar current position
    tft.drawPixel(2 * xPos + 1, 0, TFT_BLUE);

    newLine[xPos] = clr;  // Fill framebuffer line
    xPos++;

    if (xPos <= 239)

      accum = 0;
    offset = 0;
    total += cellSize;

    if (xPos >= 239) {
      tft.setTextColor(TFT_BLUE);
      tft.fillRect(330, 300, 150, 19, TFT_BLACK);
      tft.setCursor(330, 303);
      tft.print("DRAWING");
      tft.drawFastHLine(0, 0, WATERFALL_SCREEN_WIDTH, TFT_WHITE);
      // Update display from framebuffer
      addLineToFramebuffer(newLine);

      // Update screen time
      uint32_t lineTime = millis() - startTime;
      tft.fillRect(5, 300, 180, 19, TFT_BLACK);
      tft.setTextColor(TFT_YELLOW);
      tft.setCursor(5, 300);
      tft.printf("%ld min/screen", lineTime * WATERFALL_SCREEN_HEIGHT / 1000 / 60);

      total = 0;
      xPos = 0;
    }

    // Check for button press to exit loop
    if (digitalRead(ENCODER_BUTTON) == LOW)
      break;
  }

  FREQ = FREQ_OLD;
  rebuildMainScreen(1);  // rebuild the main screen
  return;
}

//##########################################################################################################################//

void rebuildMainScreen(bool freebuf) {

  if (freebuf) {
    free(framebuffer1);
    free(framebuffer2);
  }
  tft.setTextSize(2);
  tft.setTextColor(textColor);
  tRel();
  drawFrame();
  drawBigBtns();  // redraw buttons
  clockDisplay();
  displaySTEP(true);  // have step display update since step may have changed
  showFREQ = true;
  displayFREQ(FREQ);
  redrawMainScreen = true;
  displayDebugInfo = true;
  si4735.setAudioMute(false);
  tx = ty = pressed = 0;
#ifdef TINYSA_PRESENT
  centerTinySA();
#endif
}

//##########################################################################################################################//

void wIntro(long startPoint, long endPoint) {

  modType = AM;
  loadSi4735parameters();
  si4735.setAudioMute(true);
  displayDebugInfo = false;
  etft.setTTFFont(Arial_13);
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(0, 45);
  tft.fillRect(0, 0, DISP_WIDTH, DISP_HEIGHT, TFT_BLACK);
  etft.println("Slow waterfall for band monitoring.\n");
  etft.println("Use fine tune pot to adjust colors.\n");
  etft.println("Touch lower bar to listen.\n");
  etft.println("Touch waterfall to scan again.\n");
  etft.println("Touch SET to set frequency and leave\n");
  etft.println("Or press encoder to leave.\n");
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(5, 280);
  tft.printf("%.2fMHz-%.2fMHz", startPoint / 1000000.0, endPoint / 1000000.0);
}

//##########################################################################################################################//
void buffErr() {

  tft.fillRect(0, 0, DISP_WIDTH, DISP_HEIGHT, TFT_BLACK);
  tft.setCursor(0, 50);
  tft.println("No memory for framebuffers");
  tft.printf("\nFree heap:%ld", ESP.getFreeHeap());
  tft.setCursor(0, 70);
  delay(3000);
  ESP.restart();
}
//##########################################################################################################################//

bool listenToWaterfall(long endPoint, long startPoint) {

  static long OF = FREQ;  // old frequency
  pressed = get_Touch();

  if (pressed && ty >= 265 && ty < 278) {  // lower bar pressed
    si4735.setAudioMute(false);
    tft.fillRect(220, 300, 259, 19, TFT_BLACK);
    tft.setCursor(330, 300);
    tft.setTextColor(TFT_GREEN);
    tft.print("Listening");

    tft.drawRect(220, 288, 60, 31, TFT_BLUE);
    tft.setCursor(232, 295);
    tft.setTextColor(TFT_SKYBLUE);
    tft.print("SET");
    tft.setTextColor(TFT_WHITE);

    while (1) {
      pressed = get_Touch();
      if (ty < 260) {  // back to SCAN mode
        tft.fillRect(220, 288, 60, 31, TFT_BLACK);
        si4735.setAudioMute(false);
        tRel();
        break;
      }


      long spanHz = (endPoint - startPoint);
      long touchFREQ = endPoint - ((480 - tx) * spanHz / 480);  // calculate frequency that corresponds to the touch coordinates

      if (OF != touchFREQ) {

        if (ty > 260 && ty < 285) {  //touchbar touched
          FREQ = touchFREQ;
          tft.fillRect(300, 280, 179, 17, TFT_BLACK);
          tft.setCursor(330, 280);
          FREQ /= 1000;  // snap to 1KHz
          FREQ *= 1000;
          tft.printf("%ld KHz", FREQ / 1000);
          setLO();
          OF = touchFREQ;
        }
      }

      if (clw) {  // use encoder to tune
        FREQ += STEP;
        tft.fillRect(300, 280, 179, 17, TFT_BLACK);
        tft.setCursor(330, 280);
        tft.printf("%ld KHz", FREQ / 1000);
        setLO();
        clw = false;
      } else if (cclw) {
        tft.fillRect(300, 280, 179, 17, TFT_BLACK);
        tft.setCursor(330, 280);
        tft.printf("%ld KHz", FREQ / 1000);
        setLO();
        FREQ -= STEP;
        cclw = false;
      }


      if (tx >= 220 && tx <= 280 && ty >= 288) {
        rebuildMainScreen(1);
        //Serial_printf("Freq: %ld\n", FREQ / 1000);
        return 1;  // return to main menu
      }
    }
    si4735.setAudioMute(true);
  }

  return 0;  // stay in the waterfall loop
}



//##########################################################################################################################//
uint16_t interpolate(uint16_t color1, uint16_t color2, float factor) {
  uint8_t r1 = (color1 >> 11) & 0x1F;
  uint8_t g1 = (color1 >> 5) & 0x3F;
  uint8_t b1 = color1 & 0x1F;

  uint8_t r2 = (color2 >> 11) & 0x1F;
  uint8_t g2 = (color2 >> 5) & 0x3F;
  uint8_t b2 = color2 & 0x1F;

  uint8_t r = r1 + factor * (r2 - r1);
  uint8_t g = g1 + factor * (g2 - g1);
  uint8_t b = b1 + factor * (b2 - b1);

  return (r << 11) | (g << 5) | b;
}


//##########################################################################################################################//

uint16_t valueToWaterfallColor(int value) {
  // Define RGB565 color values for transitions

  uint16_t colors0[] = {
    //smooth color profile using shades of blue
    0x20d1,  //  (0-400) // darkblue
    0x6b16,  //  (401-800)
    0xa4dd,  //  (801-1200)
    0xc5fe,  //  (1201-1600)
    0xffff,  // (1601-2000) white
    0xffe0   // (2001-max) clear yellow
  };
  /*
  uint16_t colors0[] = {
    //smooth color profile using shades of green
    TFT_BLACK,  //  (0-400) // darkblue
    TFT_DARKGREEN,  //  (401-800)
    TFT_GREEN,  //  (801-1200)
    TFT_GREENYELLOW,  //  (1201-1600)
    TFT_YELLOW,  // (1601-2000) white
    0xffe0   // (2001-max) clear yellow
  };
*/


  uint16_t colors1[] = {
    //agressive color profile
    TFT_NAVY,     //  (0-400)
    TFT_BLUE,     //  (401-800)
    TFT_SKYBLUE,  //  (801-1200)
    TFT_WHITE,    //  (1201-1600)
    TFT_ORANGE,   // (1601-2000)
    TFT_RED       // (2001-max)
  };


  // Define transition points
  int transitionPoints[] = { 0, 400, 800, 1200, 1600, 2000, 20000 };

  // Find the right transition
  for (int i = 0; i < 6; i++) {
    if (value >= transitionPoints[i] && value <= transitionPoints[i + 1]) {
      float factor = float(value - transitionPoints[i]) / (transitionPoints[i + 1] - transitionPoints[i]);

      if (smoothColorGradient)
        return interpolate(colors0[i], colors0[i + 1], factor);
      else
        return interpolate(colors1[i], colors1[i + 1], factor);
    }
  }

  // value out of range, return closest color
  if (smoothColorGradient)
    return value < 0 ? colors0[0] : colors0[5];
  else
    return value < 0 ? colors1[0] : colors1[5];
}



//##########################################################################################################################//


void addLineToFramebuffer(uint16_t* newLine) {
  // Copy the new line into framebuffers
  memcpy(&framebuffer1[currentLine * FRAMEBUFFER_HALF_WIDTH], newLine, FRAMEBUFFER_HALF_WIDTH * sizeof(uint16_t));
  memcpy(&framebuffer2[currentLine * FRAMEBUFFER_HALF_WIDTH], newLine + FRAMEBUFFER_HALF_WIDTH, FRAMEBUFFER_HALF_WIDTH * sizeof(uint16_t));

  // Displayframebuffer
  updateDisplay();

  // Move to the next liine
  currentLine = (currentLine + 1) % FRAMEBUFFER_HEIGHT;
}


//##########################################################################################################################//

void updateDisplay() {
  // Scroll screen content down
  const int yShiftDown = 22;

  for (int y = WATERFALL_SCREEN_HEIGHT - 1; y > 0; y--) {
    if (digitalRead(ENCODER_BUTTON) == LOW)
      return;
    for (int x = 0; x < WATERFALL_SCREEN_WIDTH; x++) {
      // Interpolate color from the framebuffers
      uint16_t color = getInterpolatedColor(x, (currentLine + y) % FRAMEBUFFER_HEIGHT);

      if ((x % (WATERFALL_SCREEN_WIDTH / 10) == 0) && (x != 0)) {
        if ((y % (WATERFALL_SCREEN_HEIGHT / 15) == 0) && (y != 0)) {
          tft.drawPixel(x, WATERFALL_SCREEN_HEIGHT - y + yShiftDown, TFT_WHITE);  // Draw white dots as orientation help
        } else {
          tft.drawPixel(x, WATERFALL_SCREEN_HEIGHT - y + yShiftDown, TFT_BLACK);  // over black lines
        }
      } else {
        tft.drawPixel(x, WATERFALL_SCREEN_HEIGHT - y + yShiftDown, color);
      }
    }
  }

  // Draw new line on the top
  for (int x = 0; x < WATERFALL_SCREEN_WIDTH; x++) {
    uint16_t color = getInterpolatedColor(x, currentLine);
    tft.drawPixel(x, yShiftDown, color);
  }
}

//##########################################################################################################################//

uint16_t getInterpolatedColor(int x, int y) {
  // Calculate the scale factor
  float scaleFactorX = (float)FRAMEBUFFER_HALF_WIDTH / (WATERFALL_SCREEN_WIDTH / 2);

  // calculate fraction
  int originalX1 = (int)((x % (WATERFALL_SCREEN_WIDTH / 2)) * scaleFactorX);
  int originalX2 = min(originalX1 + 1, FRAMEBUFFER_HALF_WIDTH - 1);
  float fractionX = ((x % (WATERFALL_SCREEN_WIDTH / 2)) * scaleFactorX) - originalX1;

  uint16_t color1, color2;
  if (x < WATERFALL_SCREEN_WIDTH / 2) {
    color1 = framebuffer1[y * FRAMEBUFFER_HALF_WIDTH + originalX1];
    color2 = framebuffer1[y * FRAMEBUFFER_HALF_WIDTH + originalX2];
  } else {
    color1 = framebuffer2[y * FRAMEBUFFER_HALF_WIDTH + originalX1];
    color2 = framebuffer2[y * FRAMEBUFFER_HALF_WIDTH + originalX2];
  }

  // interpolate colors
  return interpolateColor(color1, color2, fractionX);
}

//##########################################################################################################################//
uint16_t interpolateColor(uint16_t color1, uint16_t color2, float fraction) {
  // Extract RGB
  uint8_t r1 = (color1 >> 11) & 0x1F;
  uint8_t g1 = (color1 >> 5) & 0x3F;
  uint8_t b1 = color1 & 0x1F;

  uint8_t r2 = (color2 >> 11) & 0x1F;
  uint8_t g2 = (color2 >> 5) & 0x3F;
  uint8_t b2 = color2 & 0x1F;

  // Interpolate
  uint8_t r = r1 + fraction * (r2 - r1);
  uint8_t g = g1 + fraction * (g2 - g1);
  uint8_t b = b1 + fraction * (b2 - b1);

  // Combine into a 16-bit color
  return (r << 11) | (g << 5) | b;
}

//##########################################################################################################################//

void touchTuner() {

  // function draws a spectrum display. Listen to a frequency by touching it at.
  // If  graphicMode == 0  draws a 2D or 3D waterfall instead


  int oldModType = modType;

  if (touchTune == false || modType == WBFM)
    return;

  if (modType != NBFM) {
    modType = AM;
    loadSi4735parameters();  // 3d works only in AM
  }

  const uint16_t xCursorStart = 3;
  uint16_t xCursor = xCursorStart;
  const uint16_t traceWidth = 333;
  long OF = FREQ;
  long ctr = 1;
  float div = tSpan / 10;
  long encoderStepSize = tSpan / 2;  // 500KHz
  float accum = 0;
  int strength = 0;
  long sum = 0;
  int minSig = 127;  // to substract average background noise
  int avgBgN = 1;    // average background noise

  static bool drawingTrace = true;
  static bool clear = false;
  char graphicMode = preferences.getChar("tGr", 1);

#ifdef TINYSA_PRESENT
  char buffer[50];
#endif

  bool threeD = true;  // draw in 3D

  float endPoint = FREQ + (tSpan / 2);
  float startPoint = FREQ - (tSpan / 2);
  startPoint = (long)startPoint / (tSpan / 2) * (tSpan / 2);  // round down
  endPoint = (long)endPoint / (tSpan / 2) * (tSpan / 2);
  uint16_t stp = tSpan / 333;  // Hz per pixel
  FREQ = startPoint;
  displayFREQ(startPoint + tSpan / 2);  // display midpoint

  si4735.setAudioMute(true);
  audioMuted = true;
  tft.fillRect(0, 296, 479, 23, TFT_BLACK);        // clear infobar
  tft.fillRect(330, 8, 135, 15, TFT_BLACK);        // clear microvolt indicator area
  tft.fillRect(Xsmtr, Ysmtr, 325, 70, TFT_BLACK);  // S meter area

  if (!clear) {
    tft.fillRect(3, 120, 336, 172, TFT_BLACK);  // clear  area for trace

    for (int j = 0; j < 4; j++) {
      drawButton(8 + j * 83, 121, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);  //  draw 3rd row of empty buttons on top
      clear = true;
    }
  }

  if (drawTrapezoid == false)
    drawScale(xCursorStart, traceWidth, startPoint, div, 189);
  else {
    graphicMode = 0;                                 // 0 =  draw a 2D/3D tRAPEZOID
    tft.fillRect(Xsmtr, Ysmtr, 325, 66, TFT_BLACK);  // fill S Meter area
    drawScale(xCursorStart, traceWidth, startPoint, div, 100);
  }

  tft.setTextSize(2);
  //tft.fillRect(18, 130, 58, 38, TFT_BLACK);  // draw buttons
  etft.setCursor(25, 140);
  etft.setTextColor(TFT_GREEN);
  etft.setTTFFont(Arial_14);
  etft.print("Back");

  etft.setCursor(110, 140);
  etft.print("Cont.");
  etft.setCursor(195, 140);
  etft.print("Set");

  if (graphicMode) {
    etft.setCursor(265, 140);
    etft.print("Graph");
  }

  else {
    etft.setCursor(265, 142);
    etft.print("3D/2D");
  }


#ifdef TINYSA_PRESENT
  sprintf(buffer, "sweep center %ld", (long)startPoint + tSpan / 2);
  Serial_println(buffer);  // initial draw
  delay(50);
#endif

hideBigBtns();

  while (drawingTrace) {



    if (clw || cclw) {  // encoder moved, draw new scale and reset parameters
      if (clw) {
        startPoint += encoderStepSize;
        endPoint += encoderStepSize;
        clw = false;
      } else if (cclw) {
        startPoint -= encoderStepSize;
        endPoint -= encoderStepSize;
        cclw = false;
      }


      if (startPoint < 0)
        startPoint = 0;

      tft.fillRect(xCursorStart, 196, traceWidth + xCursorStart, 96, TFT_BLACK);  // clear area

      if (drawTrapezoid == false)
        drawScale(xCursorStart, traceWidth, startPoint, div, 189);

      else {
        initBuffer(1);  // clear buffer
        tft.fillRect(xCursorStart, 109, traceWidth + xCursorStart, 11, TFT_BLACK);
        drawScale(xCursorStart, traceWidth, startPoint, div, 100);  // draw scale above buttons
      }

      xCursor = xCursorStart;  // reset parameters
      FREQ = startPoint;
      offset = 0;
      accum = 0;
      avgBgN = 1;  // reset the background noise substractor
      sum = 0;
      ctr = 1;
      minSig = 127;

      displayFREQ((startPoint + tSpan / 2));       // display midpoint
      drawSignalPeaks(0, 0, 0, 0, 0, 0, 0, 0, 1);  // reset 3D variables

#ifdef TINYSA_PRESENT
      sprintf(buffer, "sweep center %ld", (long)(startPoint + tSpan / 2));
      Serial_println(buffer);  // load new center frequency
      delay(50);
#endif
    }  // end if (clw || cclw)


    setLO();
    si4735.getCurrentReceivedSignalQuality(0);
    strength = si4735.getCurrentRSSI();  // Read RSSI
   

    if (ctr > 1)  // 1st run is to calculate background noise floor, do not display
      accum = (strength - avgBgN) * 30;


    if (accum < 0)
      accum = 0;

    if (minSig > strength)  // get background noise level
      minSig = strength;

    int mult = (analogRead(FINETUNE_PIN) / 250) - 1;
    uint16_t clr = valueToWaterfallColor((int)accum * mult);


    if (graphicMode)
      accum = constrain(accum, 60, 3500);
    else {
      accum *= mult / 3;
      accum = constrain(accum, 0, 8000);
    }



    if (xCursor < (traceWidth + xCursorStart)) {

      if (!drawTrapezoid)
        tft.fillRect(xCursor, 179, 1, 5, clr);  // Indicator bar
      else
        tft.fillRect(xCursor, 85, 1, 5, clr);  // Indicator bar higher


      drawSignalPeaks(xCursor, accum, 70, 1, clr, graphicMode, traceWidth, threeD, 0);


      xCursor++;
      FREQ += stp;
      accum = 0;
    }


    // Reset conditions for next trace
    else {
      xCursor = xCursorStart;
      if (!drawTrapezoid)
        tft.fillRect(xCursorStart, 179, traceWidth + xCursorStart + 1, 5, TFT_BLACK);  // Clear bar
      else
        tft.fillRect(xCursorStart, 85, traceWidth + xCursorStart + 1, 5, TFT_BLACK);
      FREQ = startPoint;
      accum = 0;
      sum += minSig;
      minSig = 127;        // reset to highest possible value (127dBuV)
      avgBgN = sum / ctr;  // average background noise
      ctr++;
    }

    // touch logic
    pressed = get_Touch();
    if (pressed) {
      while (true) {
        pressed = get_Touch();
        int buttonID = getButtonID();

        if (buttonID == 24) {  // Graph
          if (drawTrapezoid) {
            threeD = !threeD;

            tft.fillRect(265, 141, 65, 16, TFT_BLACK);
            etft.setTTFFont(Arial_14);
            etft.setTextColor(TFT_YELLOW);
            etft.setCursor(275, 135);
            etft.print("Draw");
            etft.fillRect(280, 155, 40, 14, TFT_BLACK);
            etft.setCursor(280, 155);
            if (threeD)
              etft.print("2D");
            else
              etft.print("3D");
            tRel();
            break;
          }

          graphicMode = preferences.getChar("tGr", 0);
          graphicMode++;
          if (graphicMode > 3)
            graphicMode = 1;
          preferences.putChar("tGr", graphicMode);
          pressed = false;
          xCursor = xCursorStart;
          tft.fillRect(xCursorStart, 205, traceWidth + xCursorStart + 1, 86, TFT_BLACK);  // Clear when changing mode
          FREQ = startPoint;
          accum = 0;
          tRel();
          break;
        }

        if (buttonID == 22) {
          pressed = false;
          break;  // Continue
        }

        long F = FREQ;
        // Touch is in  area
        if ((pressed && tx > 2 && tx <= traceWidth && ty > 179 && ty < 295 && graphicMode) || (pressed && tx > 2 && tx <= traceWidth && ty > 70 && ty < 90 && !graphicMode)) {

          long touchFREQ = startPoint + tSpan - ((343 - tx) * 1000000 / 360);
          //  This should be: long touchFREQ = startPoint + tSpan - ((traceWidth  - tx) * spanHz / traceWidth ); ????
          // but that produces misalignment at both extremes due to touchscreen calibration errors
          FREQ = touchFREQ / 1000 * 1000;  // round to 1KHz
         audioMuted = false;
          si4735.setAudioMute(false);
         si4735.setHardwareAudioMute(false);
        }

        if (clw) {
          FREQ += STEP;
          clw = false;

        } else if (cclw) {
          FREQ -= STEP;
          cclw = false;
        }

        if (F != FREQ) {
          FREQCheck();
          displayFREQ(FREQ);
          setLO();
          getRSSIAndSNR();
          strength = signalStrength;
          if (graphicMode)
           calculateAndDisplaySignalStrength();
          F = FREQ;
        }

        if (buttonID == 21 || buttonID == 23) {
          if (buttonID == 21) {  // Back
            FREQ = OF;
            if (!graphicMode) {
              preferences.putBool("fastBoot", true);
              ESP.restart();  // restart to have unfragmented memory, otherwise not enough memory for waterfall available.
            }
          }

          pressed = false;
          touchTune = false;
          clear = false;
          modType = oldModType;
          loadSi4735parameters();
          displayFREQ(FREQ);
          setLO();
          tRel();
          redrawMainScreen = true;
          tft.fillRect(18, 130, 58, 38, TFT_BLACK);  // Redraw TouchTune button
          etft.setTTFFont(Arial_14);
          etft.setTextColor(textColor);
          etft.setCursor(20, 132);
          etft.print("Touch");
          etft.setCursor(20, 153);
          etft.print("Tune");
          return;
        }
      }
      si4735.setAudioMute(true);  // Mute after  touch
      pressed = false;
    }
  }
}

//##########################################################################################################################//


// Buffer for 3D
static uint16_t* columnBuffer = nullptr;
void initBuffer(bool clearBuffer) {

  // Allocate mem if buffer == null
  if (columnBuffer == nullptr) {
    columnBuffer = (uint16_t*)malloc(NUM_COLUMNS * COLUMN_HEIGHT * sizeof(uint16_t));  // allocate
    memset(columnBuffer, 0, NUM_COLUMNS * COLUMN_HEIGHT * sizeof(uint16_t));           // Initialize to 0
  }

  if (clearBuffer) {
    memset(columnBuffer, 0, NUM_COLUMNS * COLUMN_HEIGHT * sizeof(uint16_t));
  }
}


//##########################################################################################################################//

void drawSignalPeaks(int xCursor, float accum, int maxHeight, int width, uint16_t clr, char graphicMode, int traceWidth, bool threeD, bool resetThreeD) {

  int baseHeight = 270;

  float scaledAccum = pow(accum / 3, 1.8) / 50;  //to expand range
  int peakHeight = baseHeight - min((int)scaledAccum / 20, maxHeight);


  peakHeight = constrain(peakHeight, 210, 270);

  int rectTop = constrain(peakHeight, 200, 290);
  int rectBottom = constrain(baseHeight - (peakHeight), 0, 78);

  if (graphicMode)
    tft.fillRect(xCursor, 207, width + 2, 84, TFT_BLACK);  // overwrite last drawing with 2px margin to the right and the bottom

  if (graphicMode == 1) {  // white blue alernate pattern
    static bool alternate = false;
    alternate = !alternate;
    if (alternate)
      tft.fillRectVGradient(xCursor, rectTop, width, rectBottom + 20, TFT_BLUE, TFT_WHITE);
    else
      tft.fillRectVGradient(xCursor, rectTop, width, rectBottom + 20, TFT_WHITE, TFT_BLUE);

    tft.fillRect(xCursor, peakHeight, width, 3, TFT_YELLOW);  // yellow band on top
  }


  if (graphicMode == 2) {  // blue white blue

    int rectHeight = baseHeight - rectTop + 20;  //height gradient
    int midHeight = rectTop + rectHeight / 2;
    tft.fillRectVGradient(xCursor, rectTop, width, midHeight - rectTop, TFT_BLUE, TFT_WHITE);
    tft.fillRectVGradient(xCursor, midHeight, width, rectTop + rectHeight - midHeight, TFT_WHITE, TFT_BLUE);
  }


  if (graphicMode == 3) {  // blue with colored peaks

    baseHeight = 284;
    int peakHeight = baseHeight - min((int)accum / 21, maxHeight);
    tft.fillTriangle(xCursor, peakHeight, xCursor + width, peakHeight,
                     xCursor + width / 2, peakHeight + 10, clr);  // Draw a peak

    // Draw gradient rectangle
    tft.fillRectVGradient(xCursor, peakHeight + 10, width, baseHeight - (peakHeight + 10), clr, TFT_NAVY);

    tft.fillRect(xCursor, peakHeight - 3, width, 3, TFT_GREY);  // Shadow at the top
  }



  if (graphicMode == 0) {  // experimental 3d rhomboid

    static int currentColumn = NUM_COLUMNS - 1;
    static float tw = 0;

    if (resetThreeD) {
      currentColumn = NUM_COLUMNS - 1;
      tw = 0;
      return;
    }

    initBuffer(0);

    columnBuffer[(currentColumn * COLUMN_HEIGHT) + (int)round(tw / 4)] = (uint16_t)(accum);  //read accum into buffer
    tw++;

    if (tw >= 333) {  // traceWidth
      // Shift all columns left by one
      tw = 0;
      for (int col = 0; col < NUM_COLUMNS - 1; col++) {
        for (int row = 0; row < COLUMN_HEIGHT; row++) {
          columnBuffer[(col * COLUMN_HEIGHT) + row] = columnBuffer[((col + 1) * COLUMN_HEIGHT) + row];
        }
      }

      // Clear right column
      for (int row = 0; row < COLUMN_HEIGHT; row++) {
        columnBuffer[((NUM_COLUMNS - 1) * COLUMN_HEIGHT) + row] = TFT_BLACK;  // Clear the last column
      }

      float shortenFactor = 0.8;    // Shorten trace on right side
      float anglecorrection = 1.2;  // more = simulates looking from higher above
      int leftLift = 30;            // lift trace on the left side
      static float persp = 0.0005;  // makes left side smaller

      // draw buffer
      for (int col = 0; col < NUM_COLUMNS; col++) {
        for (int row = 0; row < COLUMN_HEIGHT; row++) {

          int x = col * shortenFactor + 3;  // shift 3 right to avoid touching border

          // adjust left lift
          int yPos = 290 - row - ((NUM_COLUMNS - col) * leftLift / NUM_COLUMNS);

          float curveFactor = 0.0001 * pow(col - NUM_COLUMNS / 1.5, 2);  // Add a slight curve
          yPos += curveFactor;

          x += (290 - yPos) * anglecorrection - 27;  // correct viewing angle empirically

          yPos += (290 - yPos) * persp * (NUM_COLUMNS - col);  // make left side smaller (further away)

          uint16_t sStrength = columnBuffer[(col * COLUMN_HEIGHT) + row];  // read signal

          uint16_t cl3d = valueToWaterfallColor((sStrength - 600));  // calculate color

          if (threeD) {
            tft.drawPixel(x, yPos, TFT_NAVY);
            tft.fillRect(x - (sStrength / 1000 + 1), yPos - sStrength / 300, sStrength / 1000 + 1, sStrength / 300, cl3d);  // simulates 3D by writing small vertical rectangles instead of pixels
            //tft.fillRectVGradient(x - (sStrength / 1000 + 1), yPos - sStrength /100, sStrength / 1000 + 1, sStrength / 100, cl3d, TFT_BLACK);// nicer image than tft.fillRect, but slow
          } else
            tft.drawPixel(x, yPos, cl3d);
          if (row == COLUMN_HEIGHT - 1)
            tft.fillRect(x, 174, 1, 19 + x / 25, TFT_BLACK);  // overwrite rectangles that exceed area

          if (!col)
            tft.fillRect(4, yPos, x - row / 10, 1, TFT_BLACK);  // overwrite rectangles that exceed area
        }
      }
    }
  }
}
//##########################################################################################################################//


void drawScale(int xCursorStart, int traceWidth, int startPoint, float div, int height) {  // draws a little scale on top of the screen
  //

  float scale = 0;
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.drawFastHLine(xCursorStart, height, traceWidth + xCursorStart, TFT_WHITE);
  tft.drawFastHLine(xCursorStart, height + 1, traceWidth + xCursorStart, TFT_WHITE);
  tft.drawFastVLine(traceWidth - 1, height + 1, 5, TFT_WHITE);
  for (int i = xCursorStart; i < traceWidth + xCursorStart; i += traceWidth / 10) {
    tft.drawFastVLine(i, height + 2, 5, TFT_WHITE);
    tft.setCursor(i, height + 9);
    if (i < traceWidth)
      tft.printf("%.1f\n", (scale + startPoint) / (float)tSpan);
    scale += div;
  }
  tft.setTextSize(2);
}

//##########################################################################################################################//

/*
void panoramaScan() {  // shows panoramadisplay while no activity on current FREQ, tunes current FREQ when activity (squelch opens)

  float startPoint = FREQ - (tSpan / 2);  // +- 500KHz
  const uint16_t stp = tSpan / 332;       // 3 KHz per pixel
  const uint16_t endX = 335;
  const uint16_t yPos = 121;
  uint16_t xPos = 2;       // start x
  uint32_t tFREQ = 0;      // temporary frequency holder
  char lBuf[350] = { 0 };  // line buffer to overwrite previous values
  bool squelchOpen = false;
  bool centered = false;
  int minSig = 127;  // to substract average background noise
  int avgBgN = 1;    // average background noise
  int lastSS = 0;
  uint32_t ctr = 1, sum = 0;
  int16_t adjustedSS = 0;

  long savFreq = FREQ;
  FREQ = (long)startPoint;
  ttx = tx = tty = ty = pressed = 0;  // destroy previous touch coordinates

  tft.fillRect(0, 294, 227, 26, TFT_BLACK);  // overwrite clock and  mute indicator
  tft.fillRect(330, 4, 145, 22, TFT_BLACK);  // overwrite microvolts
  tft.fillRect(2, 61, endX, 62, TFT_BLACK);  // overwrite window


  if (showMeters) {
    plotNeedle(1, 0);
    plotNeedle2(1, 0);
  }

  while (true) {

    if (ctr == 1 && xPos == 2) {
      tft.setCursor(10, 100);
      tft.setTextColor(textColor);
      tft.print("Measuring noise floor...");
    }


    if (ctr == 2 && xPos == 2) {
      tft.fillRect(10, 100, 320, 16, TFT_BLACK);
      tft.drawFastVLine((endX + xPos) / 2, 61, 60, TFT_RED);  // center marker
    }

    if (!centered) {  // only calculate when not tFREQ = FREQ;
      lastSS = signalStrength;
      si4735.getCurrentReceivedSignalQuality(0);  // fetch signal strength
      signalStrength = si4735.getCurrentRSSI();
    }

    else {
      centered = false;
      signalStrength = lastSS;  // fill with "dummy" previous value
    }


    if (minSig > signalStrength)  // get noise floor
      minSig = signalStrength;

    adjustedSS = constrain(signalStrength - avgBgN, 1, 60);  // substract average background noise


    if (ctr > 1) {
      tft.drawFastVLine(xPos, yPos - lBuf[xPos] - 3, lBuf[xPos] + 2, TFT_BLACK);  // Overwrite old values

      static bool alternate = false;
      alternate = !alternate;
      if (alternate)
        tft.fillRectVGradient(xPos, yPos - adjustedSS, 1, adjustedSS, TFT_BLUE, TFT_WHITE);  // white blue alernate pattern
      else
        tft.fillRectVGradient(xPos, yPos - adjustedSS, 1, adjustedSS, TFT_WHITE, TFT_BLUE);

      tft.drawFastVLine(xPos, yPos - adjustedSS - 3, 3, TFT_GREEN);  // draw a crest


      if (xPos > 2)
        tft.fillRect(xPos, yPos - 1, 1, 2, TFT_WHITE);  // draw the tracer
    }

    lBuf[xPos] = adjustedSS;
    xPos++;
    FREQ += stp;
    setLO();  // move to next step


    uint16_t z = tft.getTouchRawZ();
    if (z > 300) {
      get_Touch();
      ttx = tx;  // use temporary value holders since tx and ty will get destroyed in the main loop
      tty = ty;

      if (ty > 120 || ty < 50 || tx > 335) {
        FREQ = savFreq;
        return;
      }


      else {  // center the peak that was touched

        if (tx > 167)
          startPoint += (tx - 167) * 3 * 1000;
        else
          startPoint -= (167 - tx) * 3 * 1000;

        startPoint /= 1000;
        startPoint *= 1000;

        startPoint /= STEP;
        startPoint *= STEP;


        savFreq = startPoint + (tSpan / 2);

        savFreq /= 1000;
        savFreq *= 1000;

        FREQ = savFreq;
        displayFREQ(FREQ);  // display new FREQ
        setLO();
        xPos = endX;

        tRel();
        tx = ty = pressed = 0;
      }
    }



    if (xPos % 30 == 0) {  // roughly every 200ms scan the tuned freq
      tFREQ = FREQ;
      centered = true;

      do {
        if (FREQ != savFreq) {
          FREQ = savFreq;
          setLO();
        }

        if (clw || cclw) {
          return;
        }

        delay(10); // this is mandatory, otherwise RSSI may fail
        si4735.getCurrentReceivedSignalQuality(0);  // fetch signal strength
        signalStrength = si4735.getCurrentRSSI();
        readSquelchPot(true);  // read'n draw
        delay(10); // this is mandatory, otherwise SNR squelch may fail  
        setSquelch();                              
 
        if (! audioMuted) {
          squelchOpen = true;
          return;
        }
      }

      while (signalStrength > currentSquelch || SNR)
         ;



      if (squelchOpen) {
        squelchOpen = false;
        si4735.setAudioMute(true);
        si4735.setHardwareAudioMute(true);
        tft.fillRect(2, 61, endX, 60, TFT_BLACK);
        xPos = endX;  // reset trace when squelch closes
      }
      FREQ = tFREQ;
    }  // endif  xPos % 30 == 0


    if (xPos == endX) {
      xPos = 2;
      tft.fillRect(2, yPos - 1, endX, 2, TFT_BLACK);
      tft.drawFastVLine((endX + xPos) / 2, 61, 60, TFT_RED);  // center marker
      FREQ = startPoint;
      sum += minSig;
      minSig = 127;        // reset to highest possible value (127dBuV)
      avgBgN = sum / ctr;  // average background noise
      ctr++;
    }
  }
}



*/



void panoramaScan() {  // shows panoramadisplay while no activity on current FREQ, tunes current FREQ when activity (squelch opens)

  float startPoint = FREQ - (tSpan / 2);  // +- 500KHz
  const uint16_t stp = tSpan / 333;       // 3 KHz per pixel
  const uint16_t endX = 335;
  const uint16_t yPos = 121;
  uint16_t xPos = 2;       // start x
  uint32_t tFREQ = 0;      // temporary frequency holder
  char lBuf[350] = { 0 };  // line buffer to overwrite previous values
  bool squelchOpen = false;
  bool centered = false;
  int minSig = 127;  // to substract average background noise
  int avgBgN = 1;    // average background noise
  int lastSS = 0;
  uint64_t ctr = 1, sum = 0;
  int16_t adjustedSS = 0;

  long savFreq = FREQ;
  FREQ = (long)startPoint;
  ttx = tx = tty = ty = pressed = 0;  // destroy previous touch coordinates

  tft.fillRect(0, 294, 227, 26, TFT_BLACK);  // overwrite clock and  mute indicator
  tft.fillRect(330, 4, 145, 22, TFT_BLACK);  // overwrite microvolts
  tft.fillRect(2, 61, endX, 62, TFT_BLACK);  // overwrite window
  drawScale(xPos, 332, FREQ, tSpan / 10, 61); // scale
  tft.drawFastVLine(endX  / 2, 61, 15, TFT_RED);  // center marker



  if (showMeters) {
    plotNeedle(1, 0);
    plotNeedle2(1, 0);
  }

  while (true) {

    if (ctr == 1 && xPos == 2) {
      tft.setCursor(10, 100);
      tft.setTextColor(textColor);
      tft.print("Measuring noise floor");
    }


    if (ctr == 2 && xPos == 2) 
      tft.fillRect(10, 100, 320, 16, TFT_BLACK);

    if (!centered) {  // only calculate when not tFREQ = FREQ;
      lastSS = signalStrength;
      si4735.getCurrentReceivedSignalQuality(0);  // fetch signal strength
      signalStrength = si4735.getCurrentRSSI();
    }

    else {
      centered = false;
      signalStrength = lastSS;  // fill with "dummy" previous value
    }


    if (minSig > signalStrength)  // get noise floor
      minSig = signalStrength;

    adjustedSS = constrain(signalStrength - avgBgN, 1, 40);  // substract average background noise and limit


    if (ctr > 1) {
      tft.drawFastVLine(xPos, yPos - lBuf[xPos] - 3, lBuf[xPos] + 2, TFT_BLACK);  // Overwrite old values

      static bool alternate = false;
      alternate = !alternate;
      if (alternate)
        tft.fillRectVGradient(xPos, yPos - adjustedSS, 1, adjustedSS, TFT_BLUE, TFT_WHITE);  // white blue alernate pattern
      else
        tft.fillRectVGradient(xPos, yPos - adjustedSS, 1, adjustedSS, TFT_WHITE, TFT_BLUE);

      tft.drawFastVLine(xPos, yPos - adjustedSS - 3, 3, TFT_GREEN);  // draw a crest


      if (xPos > 2)
        tft.fillRect(xPos, yPos - 1, 1, 2, TFT_WHITE);  // draw the tracer
    }

    lBuf[xPos] = adjustedSS;
    xPos++;
    FREQ += stp;
    setLO();  // move to next step


    uint16_t z = tft.getTouchRawZ();
    if (z > 300) {
      get_Touch();
      ttx = tx;  // use temporary value holders since tx and ty will get destroyed in the main loop
      tty = ty;

      if (ty > 120 || ty < 50 || tx > 335) {
        FREQ = savFreq;
        return;
      }


      else {  // center the peak that was touched
       drawScale(2, 332, FREQ, tSpan / 10, 61); // redraw the scale

        if (tx > 167)
          startPoint += (tx - 167) * 3 * 1000;
        else
          startPoint -= (167 - tx) * 3 * 1000;

        startPoint /= 1000;
        startPoint *= 1000;

        startPoint /= STEP;
        startPoint *= STEP;


        savFreq = startPoint + (tSpan / 2);

        savFreq /= 1000;
        savFreq *= 1000;

        FREQ = savFreq;
        roundFreqToSTEP();
        displayFREQ(FREQ);  // display new FREQ
        setLO();
        xPos = endX;

        tRel();
        tx = ty = pressed = 0;
      }
    }



    if (xPos % 30 == 0) {  // roughly every 200ms scan the tuned freq
      tFREQ = FREQ;
      centered = true;

      do {
        if (FREQ != savFreq) {
          FREQ = savFreq;
          setLO();
        }

        if (clw || cclw) {
          return;
        }

        delay(10); // this is mandatory, otherwise RSSI may fail
        si4735.getCurrentReceivedSignalQuality(0);  // fetch signal strength
        signalStrength = si4735.getCurrentRSSI();
        readSquelchPot(true);  // read'n draw
        delay(10); // this is mandatory, otherwise SNR squelch may fail  
        setSquelch();                              
 
        if (! audioMuted) {
          squelchOpen = true;
          return;
        }
      }

      while (signalStrength > currentSquelch || SNR)
         ;



      if (squelchOpen) {
        squelchOpen = false;
        si4735.setAudioMute(true);
        si4735.setHardwareAudioMute(true);
        tft.fillRect(2, 61, endX, 60, TFT_BLACK);
        xPos = endX;  // reset trace when squelch closes
      }
      FREQ = tFREQ;
    }  // endif  xPos % 30 == 0


    if (xPos == endX) {
      xPos = 2;
      tft.fillRect(2, yPos - 1, endX, 2, TFT_BLACK);
      FREQ = startPoint;
     
      sum += minSig;
      minSig = 127;        // reset to highest possible value (127dBuV)
      avgBgN = sum / ctr;  // average background noise
      ctr++;
    }
  }
}


//##########################################################################################################################//

void PanoramaScanner() {  // helper that blocks calling panoramaScan() for interval ms if it was left through touch or encoder moved
 
if (modType == LSB || modType == USB || modType == SYNC || modType == CW || modType == WBFM) // only works AM or NBFM, RSSI too slow in SSB?
   return;
 
 
 
 if (audioMuted && showPanorama) {
    static bool blocked = false;
    static unsigned long previousMillis = 0;
    const unsigned long interval = 2000;  // 2000ms interval


    if (!blocked) {
      panoramaScan();
      blocked = true;
      previousMillis = millis();  // Store time when blocking begins
      if (clw)
        FREQ += STEP;
      if (cclw)
        FREQ -= STEP;
      clw = 0;
      cclw = 0;
      setLO();
      rebuildIndicators();
    }

    // Check interval
    if (blocked && (millis() - previousMillis >= interval)) {
      blocked = false;
    }
  }
}

//##########################################################################################################################//
void selectWaterFall(){


  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);

  struct Button {
    int x;
    int y;
    const char* label;
  };

  Button buttons[] = {
    { 193, 245, "Use" }, 
    { 193, 265, "Band" }, 
    { 100, 245, "Use" }, 
    { 100, 265, "Range" }, 
    { 290, 245, "" }, 
    { 280, 265, "" }, 
    { 20, 188, "" }, 
    { 20, 210, "" }, 
    { 20, 132, "" }, 
    { 20, 152, "" }, 
    { 20, 255, "BACK" }
  };


  etft.setTTFFont(Arial_14);
  tft.fillRect(2, 61, 337, 173, TFT_BLACK); // overwrite buttons 1-8

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
        redrawMainScreen = true;
        tx = ty = pressed = 0;
        return;
      case 42:
        waterFall(true);  // use keypad for waterfall
         redrawMainScreen = true;
        tx = ty = pressed = 0;
        return;
        break;
      case 43:
        setBand(true);  // use band for waterfall
         redrawMainScreen = true;
        tx = ty = pressed = 0;
        return;
        break;
      default:
        redrawMainScreen = true;
        tx = ty = pressed = 0;
        return;
    }
  }
}

