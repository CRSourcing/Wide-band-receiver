//##########################################################################################################################//
// Next functions are to draw a slow scan waterfall, resolution is limited to 240 * 220 due to amount of framebuffer needed
// IlI9488 display does not allow to read back from the hardware framebuffer when touchscreen is enabled, so a sw buffer is needed.
// Uses fine tune potentiometer to adjust colors

#define RANGE_MULTIPLIER 3  // converts signal into a pleasant color ranhe

void waterFall(bool useKeypad) {
  if (useKeypad) {
    if (!getLimits()) return;
  }

  pressed = false;
  long endPoint = lim1;
  long startPoint = lim2;
#ifdef TINYSA_PRESENT
  long midPoint = (endPoint + startPoint) / 2;  // calculate midpoint, needed to sync the tinySA
#endif
  float span = (float)(endPoint - startPoint);

  spr2.deleteSprite();  // free mem from rolling RSSI trace
  sprite2Init = false;


  frameBuf = (uint8_t*)malloc(FRAMEBUFFER_240 * WATERFALL_SCREEN_HEIGHT);  // roughly 52K of unfragmented heap needed
  if (!frameBuf) {
    buffErr();
    return;
  }


  tft.fillScreen(TFT_BLACK);

  wIntro(startPoint, endPoint);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(5, 255);
  tft.print(F("While scanning, touch bar below to listen.Touch area above to scan again."));
  tft.setTextSize(2);
  tft.setTextColor(textColor);



#ifdef TINYSA_PRESENT  // sync tinySA with watefall
  synctinySAWaterfall(midPoint, startPoint, endPoint);
#endif

  int xPos = 0;
  uint16_t accum = 0;
  long offset = 0;
  long total = 0;
  long strength = 0;
  long lowest = 999999;  // lowest signal strength, must start above accum
  uint32_t startTime;
  uint16_t resolution = 2000;  // 2KHz
  bool drawingTrace = true;
  long cellSize = (endPoint - startPoint) / FRAMEBUFFER_240;  // cellSize is the frequency range that gets scanned for 1 pixel
  uint16_t stp = cellSize / 25;                               // can't change filter bandwith, need to scan each cell. 25 steps in every cellSize sufficient up to 30 MHz range

  if (stp < resolution)  // stepsize below resolution makes no sense, so we change it to resolution
    stp = resolution;
  if (stp > 5000)
    stp = 5000;  // for wide scans > 30MHz



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

  tft.setSwapBytes(true);  // set to big endian for pushing an entire line to the display
  memset(frameBuf, 0x00, FRAMEBUFFER_240 * WATERFALL_SCREEN_HEIGHT);


  // loop that reads RSSI and feeds the framebuffer
  while (drawingTrace) {

    if (listenToWaterfall(endPoint, startPoint))  // returns true when SET was touched
      return;

    if (!xPos) {
      startTime = millis();  // Start the timer
      tft.fillRect(330, 300, 150, 19, TFT_BLACK);
      tft.setCursor(330, 303);
      tft.setTextColor(TFT_RED);
      tft.print(F("SCANNING"));
    }


    uint16_t mult = 5 + (analogRead(FINETUNE_PIN) / 200);  // mult range from 5 to 25


    uint32_t corrFactor = (cellSize / 2000);     // ajust accum to cellsize
    corrFactor = constrain(corrFactor, 1, 50l);  // Limit corrFactor to 50

    while (offset < cellSize) {
      FREQ = startPoint + total + offset;
      setFreq();
      si4735.getCurrentReceivedSignalQuality(0);
      delayMicroseconds(500);
      strength = si4735.getCurrentRSSI();  // Read RSSI

      accum += (strength * mult) / (corrFactor);

      offset += stp;
    }

    if ((accum < lowest)) {  //  calculate background noise
      lowest = accum;
    }


    uint16_t cVal = (accum - lowest) * RANGE_MULTIPLIER;  // substract noise, multiply to extend range

    cVal = constrain(cVal, 0, 2047);

    uint16_t clr = valueToWaterfallColor(cVal);  // convert to RGB565

    tft.fillRect(2 * xPos, 265, 2, 6, clr);  // draw indicator bar


    uint8_t clr8 = cVal >> 3;  // shift raw value

    transferBuffer[xPos] = clr8;  // and fill transfer buffer


    xPos++;

    accum = 0;
    offset = 0;
    total += cellSize;

    if (xPos >= 239) {
      lowest = 999999;  // set above accum
      tft.setTextColor(TFT_BLUE);
      tft.fillRect(330, 300, 150, 19, TFT_BLACK);
      tft.setCursor(330, 303);
      tft.print(F("DRAWING"));

      addLineToFramebuffer(transferBuffer);

      // Update screen time
      uint32_t lineTime = millis() - startTime;
      tft.fillRect(5, 300, 180, 19, TFT_BLACK);
      tft.setTextColor(TFT_YELLOW);
      tft.setCursor(5, 300);
      tft.printf("%ld min/screen", lineTime * WATERFALL_SCREEN_HEIGHT / 1000 / 60);

      total = 0;
      xPos = 0;
    }

    // Check for exit loop
    if (digitalRead(ENCODER_BUTTON) == LOW || clw || cclw)
      break;
  }

  clw = false;
  cclw = false;
  FREQ = FREQ_OLD;
  free(frameBuf);
  frameBuf = nullptr;
  tft.setSwapBytes(false);
  rebuildMainScreen(1);  // rebuild the main screen
}

//##########################################################################################################################//

void addLineToFramebuffer(uint16_t* transferBuffer) {
  for (int x = 0; x < FRAMEBUFFER_240; x++) {
    frameBuf[currentLine * FRAMEBUFFER_240 + x] = transferBuffer[x];  // from transfer to framebuf still 8 bit
  }
  updateDisplay();
  currentLine = (currentLine + 1) % WATERFALL_SCREEN_HEIGHT;
}

//##########################################################################################################################//


void updateDisplay() {
  const int yShiftDown = 22;


  for (int y = WATERFALL_SCREEN_HEIGHT - 1; y > 0; y--) {
    if (digitalRead(ENCODER_BUTTON) == LOW) return;

    for (int x = 0; x < FRAMEBUFFER_240; x++) {
      uint8_t val = frameBuf[((currentLine + y) % WATERFALL_SCREEN_HEIGHT) * FRAMEBUFFER_240 + x];

      // Expand horizontally and shift back to 16bit

      uint16_t cVal = valueToWaterfallColor(val << 3);
      if (cVal < 0x0010)  // supress lowesv values (dark blue background)
        cVal = 0;

      if ((x % (WATERFALL_SCREEN_WIDTH / 10) == 0) && (x != 0)) {
        if ((y % (WATERFALL_SCREEN_HEIGHT / 15) == 0) && (y != 0)) {
          transferBuffer[2 * x] = TFT_WHITE;  // reuse transferBuffer[], fill with orientation dots
          transferBuffer[2 * x + 1] = TFT_WHITE;
        } else {
          transferBuffer[2 * x] = TFT_BLACK;  // orientation dots
          transferBuffer[2 * x + 1] = TFT_BLACK;
        }
      } else {

        transferBuffer[2 * x] = cVal;
        transferBuffer[2 * x + 1] = cVal;
      }
    }

    int drawY = WATERFALL_SCREEN_HEIGHT - y + yShiftDown;
    tft.pushImage(0, drawY, WATERFALL_SCREEN_WIDTH, 1, transferBuffer);
  }
}
//##########################################################################################################################//

void rebuildMainScreen(bool freebuf) {

  if (freebuf) {
    free(frameBuf1);
  }
  tft.setTextSize(2);
  tft.setTextColor(textColor);
  tRel();
  drawFrame();
  drawBigBtns();      // redraw buttons
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
  etft.setCursor(0, 35);
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
  tft.println("No memory for framebuffer");
  tft.printf("\nFree heap:%ld\n", ESP.getFreeHeap());
  tft.println();
  tft.println("Try after reboot");
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
    tft.print(F("Listening"));

    tft.drawRect(220, 288, 60, 31, TFT_BLUE);
    tft.setCursor(232, 295);
    tft.setTextColor(TFT_SKYBLUE);
    tft.print(F("SET"));
    tft.setTextColor(TFT_WHITE);

    while (true) {
      pressed = get_Touch();
      if (ty < 260) {  // back to SCAN mode
        tft.fillRect(220, 288, 60, 31, TFT_BLACK);
        si4735.setAudioMute(false);
        tRel();
        break;
      }


      long spanHz = (endPoint - startPoint);
      long touchFREQ = endPoint - ((WATERFALL_SCREEN_WIDTH - tx) * spanHz / WATERFALL_SCREEN_WIDTH);  // calculate frequency that corresponds to the touch coordinates

      if (OF != touchFREQ) {

        if (ty > 260 && ty < 285) {  //touchbar touched
          FREQ = touchFREQ;
          tft.fillRect(300, 280, 179, 17, TFT_BLACK);
          tft.setCursor(330, 280);
          FREQ /= 1000;  // snap to 1KHz
          FREQ *= 1000;
          tft.printf("%ld KHz", FREQ_TO_KHZ);
          setFreq();
          OF = touchFREQ;
        }
      }

      if (clw) {  // use encoder to tune
        FREQ += STEP;
        tft.fillRect(300, 280, 179, 17, TFT_BLACK);
        tft.setCursor(330, 280);
        tft.printf("%ld KHz", FREQ_TO_KHZ);
        setFreq();
        clw = false;
      } else if (cclw) {
        tft.fillRect(300, 280, 179, 17, TFT_BLACK);
        tft.setCursor(330, 280);
        tft.printf("%ld KHz", FREQ_TO_KHZ);
        setFreq();
        FREQ -= STEP;
        cclw = false;
      }


      if (tx >= 220 && tx <= 280 && ty >= 288) {
        tft.setSwapBytes(false);
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

uint16_t valueToWaterfallColor(uint16_t value) {
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
  int transitionPoints[] = { 0, 400, 800, 1200, 1600, 1900, 20000 };

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


void drawScale(int xCursorStart, int traceWidth, uint32_t startPoint, float div, int height) {  // draws a little scale on top of the screen
  //

  float scale = 0;
  tft.fillRect(xCursorStart, height, traceWidth + xCursorStart, 18, TFT_BLACK);  // overwrite last scale
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.drawFastHLine(xCursorStart, height, traceWidth + xCursorStart, TFT_WHITE);  // top lines
  tft.drawFastHLine(xCursorStart, height + 1, traceWidth + xCursorStart, TFT_WHITE);
  for (int i = xCursorStart; i < traceWidth + xCursorStart; i += traceWidth / 10) {  //draw ticks
    tft.drawFastVLine(i + 1, height + 2, 5, TFT_WHITE);
    tft.setCursor(i, height + 9);
    if (i < traceWidth && startPoint >= 0)
      tft.printf("%.1f", (float)(scale + startPoint) / 1000000.0);  // labels
    scale += div / 10;
  }
  tft.setTextSize(2);
}

//##########################################################################################################################//


void panoramaScan(bool show1MhzSegment) {  // shows panorama and waterfall while no activity on current FREQ (squelch is closed)
                                           //  show1MhzSegment will show a 1 MHz segment that can be changed with the encoder
  uint32_t startPoint;
  const uint16_t stp = defaultSpan / 333;  // 3 KHz per pixel
  const uint16_t endX = 337;
  const uint16_t startY = 61;
  const uint16_t endY = 293;
  const uint16_t startWfY = 165;  // start waterfall y
  const uint16_t yPos = 161;
  uint16_t xPos = 2;
  uint16_t startX = xPos;     // start x
  uint32_t tFREQ = 0;         // temporary frequency holder
  char amplBuf[355] = { 0 };  // amplitude buffer to overwrite previous panorama
  const int wWidth = endX - startX;
  const int wHeight = 100;

  bool squelchOpen = false;
  bool centered = false;
  bool restart = false;
  int minSig = 127;  // to calculate min signal (noise level0
  int noise = 1;     // holds noise level from last run
  int lastSS = 0;
  uint32_t ctr = 1;
  int16_t adjustedSS = 0;
  long savFreq = FREQ;


  if (show1MhzSegment == false && FREQ >= defaultSpan / 2)
    startPoint = FREQ - (defaultSpan / 2);  // +- 500KHz

  else {

    if (modType != AM || modType != NBFM) {
      modType = AM;  // force AM demodulation, SSB not working for scan
      loadSi4735parameters();
    }

    si4735.setAudioMute(true);
    startPoint = FREQ_TO_MHZ;  //start with full MHz
  }

  FREQ = startPoint;
  ttx = tx = tty = ty = pressed = 0;  // eliminate prev touch coordinates


  initPanoramaScan(wWidth, wHeight, startX, startY, endX, endY);

  drawScale(xPos, 332, FREQ, defaultSpan, 65);   // scale
  tft.drawFastVLine(endX / 2, 61, 15, TFT_RED);  // center marker

  while (true) {

    if (ctr == 1 && xPos == 2) {
      tft.setCursor(10, 100);
      tft.setTextColor(textColor);
      tft.print(F("Measuring noise floor"));
    }


    if (ctr == 2 && xPos == 2)
      tft.fillRect(10, 100, 320, 16, TFT_BLACK);  // overwrite notification

    if (!centered) {  // only calculate when not tFREQ = FREQ;
      lastSS = signalStrength;
      si4735.getCurrentReceivedSignalQuality(0);  // fetch signal strength
      delayMicroseconds(500);
      signalStrength = si4735.getCurrentRSSI();
    }

    else {
      centered = false;
      if (!show1MhzSegment)
        signalStrength = lastSS;  // fill with "dummy" previous value
    }


    if (minSig > signalStrength)  // get lowest
      minSig = signalStrength;

    adjustedSS = constrain(signalStrength - noise, 1, 80);  // substract average background noise and limit


    if (ctr > 1) {

      uint16_t clr = valueToWaterfallColor(adjustedSS * 50);  // Convert to RGB565

      frameBuf1[xPos] = clr;  // fill buffer

      tft.drawFastVLine(xPos, yPos - amplBuf[xPos] - 3, amplBuf[xPos] + 2, TFT_BLACK);  // Overwrite last amplitude values

      tft.fillRectVGradient(xPos, yPos - adjustedSS, 1, adjustedSS / 2, clr, TFT_BLUE);  //blue to peak color in upper half

      tft.fillRectVGradient(xPos, yPos - adjustedSS / 2, 1, adjustedSS / 2, TFT_BLUE, TFT_NAVY);  // darkblue to blue in lower half

      tft.drawFastVLine(xPos, yPos - adjustedSS - 2, 2, TFT_GREEN);  // draw a green crest
    }

    amplBuf[xPos] = adjustedSS;
    xPos++;
    FREQ += stp;
    setFreq();  // move to next frequency step

    if ((xPos % 30 == 0) && (show1MhzSegment == false)) {  // roughly every 200ms poll the tuned freq
      tFREQ = FREQ;
      centered = true;

      do {
        if (FREQ != savFreq) {
          FREQ = savFreq;
          setFreq();
        }

        delay(5);                                   // this is mandatory, otherwise RSSI may fail
        si4735.getCurrentReceivedSignalQuality(0);  // fetch signal strength
        signalStrength = si4735.getCurrentRSSI();
        readSquelchPot(true);  // read'n draw
        delay(5);              // this is mandatory, otherwise SNR squelch may fail
        setSquelch();

        if (signalStrength > currentSquelch) {
          squelchOpen = true;
          free(frameBuf1);
          tft.setSwapBytes(false);
          redrawMainScreen = true;
          mainScreen();
          return;
        }
      }

      while (signalStrength > currentSquelch || SNR);



      if (squelchOpen) {
        squelchOpen = false;
        si4735.setAudioMute(true);
        si4735.setHardwareAudioMute(true);
        tft.fillRect(2, 61, endX, 60, TFT_BLACK);
        xPos = endX;  // reset trace when squelch closes
      }
      FREQ = tFREQ;
    }  // endif  xPos % 30 == 0



    uint16_t z = tft.getTouchRawZ();  // fast sample
    if (z > 400) {                    // yes, touched, get coordinates
      get_Touch();


      if ((ty > (startWfY + wHeight) || tx > endX) && show1MhzSegment == false) {  // touch outside waterfall
        FREQ = savFreq;
        resetMainScreen();
        show1MhzSegment = false;  // disable the 1 MHz segment mode
        tPress();
        tRel();
        free(frameBuf1);
        tft.setSwapBytes(false);
        mainScreen();
        return;
      }



      if (show1MhzSegment && tx < endX) {  // we are in 1MHz mode
        si4735.setAudioMute(false);
        while (true) {  // loop to tune to the peak that was touched
          delay(20);

          // Break if encoder moved (load new segment)
          if (clw || cclw) {
            si4735.setAudioMute(true);
            break;
          }
          get_Touch();


          // Leave segment
          if (ty >= (endY - 30) && tx > (startX + 100) && tx < (startX + 155)) {
            show1MhzSegment = false;
            free(frameBuf1);
            tft.setSwapBytes(false);
            FREQ = savFreq;
            FREQ_OLD = -1;  // force display update
            return;
          }

          // restart and continue drawing waterfall
          else if (ty >= (endY - 30) && tx > (startX + 165) && tx < (startX + 215)) {  // restart wf
            restart = true;
            si4735.setAudioMute(true);
            tRel();
            tx = ty = z = 0;
            pressed = false;
            break;
          }

          // --- Touch inside waterfall area: tune frequency ---
          else if (ty < (startWfY + 50) && (tx < endX)) {
            uint32_t offset = (tx - startX) * 3 * 1000;
            FREQ = startPoint + offset;
          }

          // --- Set & Leave ---
          else if (ty >= (endY - 30) && tx > (startX + 10) && tx < (startX + 75)) {
            show1MhzSegment = false;
            free(frameBuf1);
            tft.setSwapBytes(false);
            FREQ /= STEP;
            FREQ *= STEP;  // round down to STEP
            return;
          }

          //set to freq when pressed
          if (pressed) {
            FREQCheck();
            FREQ /= STEP;
            FREQ *= STEP;       // round down to STEP
            displayFREQ(FREQ);  // display new FREQ
            setFreq();
            si4735.setAudioMute(false);
            si4735.setHardwareAudioMute(false);
            pressed = false;
          }
        }  // endwhile true
      }
    }  //endif z > 400



    if ((clw || cclw || restart) && show1MhzSegment) {

      if (clw) {

        FREQ += 1000000;  // up
      } else if (cclw) {
        FREQ -= 1000000;  // down
      }


      restart = false;
      FREQ = FREQ_TO_MHZ;  // round down
      startPoint = FREQ;
      displayFREQ(FREQ);  // display new FREQ

#ifdef TINYSA_PRESENT
      char buffer[50];
      sprintf(buffer, "sweep center %ld", FREQ + defaultSpan / 2);  // sync TSA center frequency with the next 1MHz step
      Serial.println(buffer);
#endif

      tft.fillRect(2, startY, endX, 200, TFT_BLACK);  // overwrite window
      drawScale(2, 332, FREQ, defaultSpan, 65);
      memset(frameBuf1, 0, wWidth * wHeight * sizeof(uint16_t));  // clear buffer
      xPos = startX;
      ctr = 1;
      clw = false;
      cclw = false;
      tx = ty = pressed = 0;
    }



    if (xPos == endX) {

      pushPanoramaBuf(wHeight, wWidth, startX, startWfY);

      xPos = startX;
      FREQ = startPoint;
      noise = minSig;
      minSig = 127;  // reset to max. possible value
      ctr++;
    }
  }
}


//##########################################################################################################################//

void pushPanoramaBuf(int wHeight, int wWidth, int startX, int startWfY) {


  // Scroll framebuffer down one line
  for (int yCopy = wHeight - 1; yCopy > 0; yCopy--) {
    for (int xCopy = startX; xCopy < wWidth; xCopy++) {
      frameBuf1[yCopy * wWidth + xCopy] =
        frameBuf1[(yCopy - 1) * wWidth + xCopy];
    }
  }

  tft.setSwapBytes(true);                                                    // needed for pushimage
  tft.pushImage(startX, startWfY, wWidth, wHeight - 1, &frameBuf1[wWidth]);  // push next screen
  tft.setSwapBytes(false);
}

//##########################################################################################################################//


void initPanoramaScan(int wWidth, int wHeight, int startX, int startY, int endX, int endY) {

  tft.fillRect(0, 294, 480, 26, TFT_BLACK);  // overwrite lower area

  if (!useNixieDial)
    tft.fillRect(330, 4, 145, 22, TFT_BLACK);  // overwrite microvolts

  tft.fillRect(startX, startY, endX, 230, TFT_BLACK);  // overwrite window


  if (show1MhzSegment == false) {
    tft.setTextSize(1);
    tft.setCursor(startX + 10, endY - 20);
    tft.print(F("Open squelch to return"));

  }

  else {
    tft.setTextColor(TFT_WHITE);

    tft.drawRoundRect(startX + 10, endY - 25, 75, 20, 3, TFT_GREEN);
    tft.drawRoundRect(startX + 100, endY - 25, 55, 20, 3, TFT_GREEN);
    tft.drawRoundRect(startX + 165, endY - 25, 50, 20, 3, TFT_GREEN);


    tft.setTextSize(1);
    tft.fillRect(345, 48, 130, 242, TFT_BLACK);  // overwrite big button/meters panel
    tft.setCursor(350, 80);
    tft.print(F("Encoder changes MHz."));
    tft.setCursor(350, 100);
    tft.print(F("Tap peak to listen."));

    tft.setCursor(startX + 15, endY - 20);
    tft.print(F("Set & Leave"));

    tft.setCursor(startX + 110, endY - 20);
    tft.print(F("Leave"));

    tft.setCursor(startX + 170, endY - 20);
    tft.print(F("Restart"));

    tft.setTextColor(textColor);
  }

  tft.setTextSize(2);

  tRel();

  // allocate framebuffer
  frameBuf1 = (uint16_t*)malloc(wWidth * wHeight * sizeof(uint16_t));
  if (frameBuf1 == NULL)
    buffErr();  // exit graciously

  memset(frameBuf1, 0, wWidth * wHeight * sizeof(uint16_t));  // black background
}



//##########################################################################################################################//



void loadPanoramaScan(bool show1MhzSegment) {  // helper that blocks calling panoramaScan() for interval ms if it was left through touch or encoder moved
  // mode 1 = show +- 500KHz

  if ((modType == LSB || modType == USB || modType == SYNC || modType == CW || modType == WBFM) && show1MhzSegment == false)  // only works AM or NBFM, RSSI too slow in SSB?
    return;



  if (audioMuted && showPanorama) {
    static bool blocked = false;
    static unsigned long previousMillis = 0;
    const unsigned long interval = 2000;  // 2000ms interval


    if (!blocked) {
      panoramaScan(show1MhzSegment);
      blocked = true;
      previousMillis = millis();  // Store time when blocking begins
      if (clw)
        FREQ += STEP;
      if (cclw)
        FREQ -= STEP;
      clw = false;
      cclw = false;
      setFreq();
      redrawIndicators();
    }

    // Check interval
    if (blocked && (millis() - previousMillis >= interval)) {
      blocked = false;
    }
  }
}

//##########################################################################################################################//
void selectWaterFall() {  // select waterfall mode


  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);

  struct Button {
    int x;
    int y;
    const char* label;
  };

  const Button buttons[] PROGMEM = {
    { 193, 245, "Use" },
    { 193, 265, "Band" },
    { 100, 245, "Use" },
    { 100, 265, "Range" },
    { 270, 245, "Show" },
    { 270, 265, "1MHz" },
    { 20, 188, "" },
    { 20, 210, "" },
    { 20, 132, "" },
    { 20, 152, "" },
    { 20, 255, "BACK" }
  };


  etft.setTTFFont(Arial_14);
  tft.fillRect(2, 61, 337, 173, TFT_BLACK);  // overwrite buttons 1-8
  tft.fillRect(0, 296, 480, 24, TFT_BLACK);  // clear infobar

  if (timeSet) {
    timeOld = 0;
    tft.fillRect(340, 6, 116, 16, TFT_BLACK);  // overwrite last time
  }

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
        waterFall(true);  // use keypad for waterfall
        resetMainScreen();
        return;
      case 43:
        setBand(true);  // use band for waterfall
        resetMainScreen();
        return;

      case 44:
        {
          show1MhzSegment = true;
          uint8_t oldMT = modType;
          panoramaScan(show1MhzSegment);  // neeeds SI4732 AM mode
          if (oldMT != modType) {
            modType = oldMT;
            loadSi4735parameters();
            redrawIndicators();
          }
          resetMainScreen();
        }
        show1MhzSegment = false;
        return;
      default:
        resetMainScreen();
        return;
    }
  }
}
