#ifdef AUDIO_SQUAREWAVE_PRESENT

// EXPERIMENTAL SSTV decoder expects a square wave of the audio signal on PULSE_PIN
// Only Scottie and Martin modes implemented

#define LINE_WIDTH 320  // Width of the image
#define FREQ_MIN 1500   // Minimum frequency for pixel decoding
#define FREQ_MAX 2300   // Maximum frequency for pixel decoding 


int numLines = 256;
int sMode = 0;
int oldSmode = -1;
long syncInterval = 0;
bool imageLoop = false;
bool autoMode = false;
//##########################################################################################################################//


void SSTVINIT() {

  attachInterrupt(digitalPinToInterrupt(PULSE_PIN), handlePulse, RISING);  // Attach interrupt for SSTV frequency measurement

  sMode = autoMode = imageLoop = 0;

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  adjustFreq();  // to fine tune freq
  tft.fillScreen(TFT_BLACK);
  waitForVerticalSyncPulse();
  tft.setCursor(340, 280);
  tft.printf("F:%ld", FREQ);

  if (imageLoop == true) {
    while (true)
      decodeLines();
  } else
    decodeLines();
}


//##########################################################################################################################//


void decodeLines() {

  while (!sMode) {  // select decoder based on time between sync pulses, not VIS

    determineHorizontalSyncInterval();  // Calculates time btw 2 sync pulses
    delay(400);                         //
    syncInterval /= 1000;               // convert to ms


    if (syncInterval < 1000 && autoMode == true) {


      if (digitalRead(ENCODER_BUTTON) == LOW)
        return;

      tft.fillRect(350, 80, 130, 16, TFT_BLACK);
      tft.setCursor(350, 80);
      tft.print("Automode");
      tft.fillRect(350, 100, 130, 16, TFT_BLACK);
      tft.setCursor(350, 100);
      tft.printf("Sync:%ldms", syncInterval);


      if (syncInterval > 426 && syncInterval < 429)
        sMode = 1;  // Scottie 1 found

      if (syncInterval > 553 && syncInterval < 558)
        sMode = 3;  // Scottie 2 found

      if (syncInterval > 444 && syncInterval < 448)
        sMode = 2;  //Martin 1 found

      if (syncInterval > 448 && syncInterval < 454)
        sMode = 4;  //Martin 2 found
    }
  }

  showMode();

  for (uint16_t line = 0; line < numLines; line++) {  // / Decode and draw individual lines


    tft.fillCircle(322, line, 3, TFT_YELLOW); // line cursor

    if (sMode == 1 || sMode == 3)
      decodeLineS(line);  // Scottie
    else if (sMode == 2 || sMode == 4)
      decodeLineM(line);  // Martin

    tft.fillCircle(322, line, 3, TFT_BLACK);

    if (digitalRead(ENCODER_BUTTON) == LOW) {  // encoder pressed while building image

      if (imageLoop) {
        preferences.putBool("fB", true);
        ESP.restart();
      } else
        return;
    }
  }
  
  if (!imageLoop)
    while (digitalRead(ENCODER_BUTTON) == HIGH)  // display full image until pressed
      ;
}

//##########################################################################################################################/

void showMode() {

  tft.setCursor(350, 40);

  if (sMode == 1)
    tft.print("Scottie S1");

  if (sMode == 3)
    tft.print("Scottie S2");

  if (sMode == 2)
    tft.print("Martin 1");

  if (sMode == 4)
    tft.print("Martin 2");
}
//##########################################################################################################################/


void adjustFreq() {  // helper function to fine tune the the frequency and see whether the sync pulse is at 1200Hz.

  int frequency;
  int lineX = 0;
  int dly = 430;    // a nice value for the graph
  int prevX = 0;    // Prev X position
  int prevY = 320;  // Prev Y position
  bool syncPulseDetected = false;
  const int startX = 350;
  const int startY = 5;
  const int width = 120;
  const int height = 30;
  const int spacing = 40;
  const char* labels[] = {
    "Auto", "Martin M1", "Martin M2", "Scottie S1", "Scottie S2", "Nonstop", "", ""
  };

  sMode = autoMode = 0;

  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(1);
  tft.setCursor(330, 270);
  tft.print("1K");
  tft.drawFastHLine(0, 270, 320, TFT_DARKGREY);
  tft.setCursor(330, 220);
  tft.print("2K");
  tft.drawFastHLine(0, 220, 320, TFT_DARKGREY);
  tft.setCursor(330, 170);
  tft.print("3K");
  tft.drawFastHLine(0, 170, 320, TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(10, 280);
  tft.print("Adjust to see white sync mark (1200Hz) at left corner");
  tft.setTextSize(2);


  for (int i = 0; i < 8; i++) {  // labes for mode on the right side
    int y = startY + (i * spacing);
    tft.fillRect(startX, y, width, height, TFT_GREY);
    tft.setCursor(startX + 5, y + 5);
    tft.print(labels[i]);
  }

  displayFREQ(FREQ);



  while (true) {

    sMode = getSSTVMode();
    if ((autoMode == true) || sMode)
      break;


    while (!newPulse)  // wait for new rising pulse
      ;

    newPulse = false;                             // Clear  flagk
    volatile long currentTime = micros();         // Get timestamp
    pulseInterval = currentTime - lastPulseTime;  // Calculate time elapsed
    lastPulseTime = currentTime;

    if (pulseInterval > 0) {
      frequency = 1000000 / pulseInterval;  // calc. frequency in Hz (f = 1 / T)


      if (frequency < 2950 && frequency > 1050) {
        int currentY = 320 - (int)frequency / 20;  // calculat Y pos based on freq

        // color based on freq
        uint16_t color = TFT_BLACK;
        if (frequency >= 1550 && frequency <= 2300) {
          color = TFT_DARKGREEN;  // in range
        } else if (frequency >= 1150 && frequency <= 1250) {
          color = TFT_WHITE;
          syncPulseDetected = true;  // line sync pulse
          tft.fillCircle(10, 50, 5, TFT_YELLOW);
        }

        else if (frequency >= 1450 && frequency <= 1550) {
          color = TFT_NAVY;             // Sync porch
        } else if (frequency > 2300) {  //above
          color = TFT_MAROON;
        } else if (frequency < 1200) {
          color = TFT_MAROON;  // beelow
        }


        if (syncPulseDetected) {
          // Draw a line from the prev pos to the current pos
          if (lineX)
            tft.drawLine(prevX, prevY, lineX, currentY, color);

          // Update previous pos
          prevX = lineX;
          prevY = currentY;

          // Increment X position
          lineX++;
        }

        delayMicroseconds(dly);
      }



      if (lineX == 320) {
        tft.fillCircle(10, 50, 5, TFT_BLACK);
        syncPulseDetected = false;
        delay(125);
        lineX = 0;
        prevX = 0;    // Reset
        prevY = 320;  // Reset
        tft.fillRect(0, 171, 320, 98, TFT_BLACK);
        tft.fillRect(200, 125, 155, 40, TFT_BLACK);
        tft.setCursor(200, 125);
        tft.printf("%dHz", frequency);
        tft.drawFastHLine(0, 220, 320, TFT_DARKGREY);
      }


      if (clw) {     // encoder clockwise
        FREQ += 25;  // fine tune in 25Hz steps
        displayFREQ(FREQ);
        setLO();
        clw = false;
      }

      if (cclw) {
        FREQ -= 25;
        displayFREQ(FREQ);
        setLO();
        cclw = false;
      }
    }
  }
}
//##########################################################################################################################/

int getSSTVMode() {
  int decoMode = 0;

  get_Touch();

  if (!pressed)
    return 0;

  pressed = false;

  if (tx >= 330) {

    if (ty >= 0 && ty <= 30)
      autoMode = true;

    if (ty >= 40 && ty <= 75)
      decoMode = 2;  // Martin 1
    if (ty >= 85 && ty <= 120)
      decoMode = 4;  // Martin 2
    if (ty >= 120 && ty <= 160)
      decoMode = 1;  // Scottie 1
    if (ty >= 160 && ty <= 200)
      decoMode = 3;  // Scottie 2
    if (ty >= 200 && ty <= 240) {
      imageLoop = true;  //
      tft.setCursor(350, 300);
      tft.print("Loop:ON");
    }

    tx = ty = 0;

    return decoMode;
  }

  return 0;
}


//##########################################################################################################################/

unsigned int pixelDurationS = 439;  // Default for Scottie S1 time used for frequency measurement

void decodeLineS(uint16_t line) {
  uint8_t bufG[330] = { 0 };
  uint8_t bufB[330] = { 0 };
  uint8_t bufR[330] = { 0 };
  uint16_t lineBuffer[320];  // Buffer for one line

  int frq = 0;
  uint8_t* currentBuf = nullptr;

  if (sMode == 3)
    pixelDurationS = 281;  // if Scottie S2 found

  // Display line number (Debugging)
  tft.fillRect(350, 10, 80, 20, TFT_BLACK);
  tft.setCursor(350, 10);
  tft.printf("L:%d", line);

  waitForHorizontalSyncPulse();
  delayMicroseconds(1500);  // Sync pulse delay

  encoderModifyFreq();  // For debug

  for (uint16_t pass = 0; pass < 3; ++pass) {
    currentBuf = (pass == 0) ? bufR : (pass == 1) ? bufG
                                                  : bufB;  // Pre-calculate buffer pointer
    unsigned long pixelStartTime = micros();

    for (uint16_t x = 0; x < 320; ++x) {
      unsigned long targetTime = pixelStartTime + pixelDurationS;

      while (micros() < targetTime) {  // Wait until the pixel duration elapses
        if (newPulse) {
          newPulse = false;
          unsigned long currentTime = micros();
          unsigned long pulseInterval = currentTime - lastPulseTime;
          lastPulseTime = currentTime;

          if (pulseInterval) {
            frq = 1000000 / pulseInterval;
          }
        }
      }

      frq = constrain(frq, 1500, 2300);
      *(currentBuf + x) = map(frq, 1500, 2300, 0, 255);  // Pointer arithmetic for speed
      pixelStartTime = targetTime;                       // Prepare for the next pixel
    }
  }

  // Prepare the buffer for DMA transfer
  uint16_t* lineBufferPtr = lineBuffer;
  for (uint16_t x = 15; x < 320; ++x) {
    uint16_t red = (bufR[x] >> 3) << 11;
    uint16_t green = (bufG[x] >> 2) << 5;
    uint16_t blue = (bufB[x] >> 3);
    *lineBufferPtr++ = red | green | blue;
  }

  // Push the buffer to the display
  tft.startWrite();
  tft.setAddrWindow(0, line, 305, 1);  // x, y, width, height
  tft.pushColors(lineBuffer, 305, true);
  tft.endWrite();
}



//##########################################################################################################################/



void decodeLineM(uint16_t line) {  // Martin decoder

  static uint16_t pixelDurationM;
  if (sMode == 4)
    pixelDurationM = 450;  // Martin M2
  else
    pixelDurationM = 454;  // M1

  uint8_t bufG[330] = { 0 };
  uint8_t bufB[330] = { 0 };
  uint8_t bufR[330] = { 0 };
  uint16_t lineBuffer[320];  // Buffer for one line


  uint16_t x = 0;
  uint16_t pass = 0;
  int frq = 0;


  // Display line number (Debugging)
  tft.fillRect(350, 10, 80, 20, TFT_BLACK);
  tft.setCursor(350, 10);
  tft.printf("L:%d", line);

  waitForHorizontalSyncPulse();
  delayMicroseconds(2100);  // Sync pulse delay

  while (pass < 3) {
    encoderModifyFreq();
    unsigned long pixelStartTime = micros();  // Start time for the pixel

    while (x < 320) {
      // Process the pixel for the current duration
      while (micros() - pixelStartTime < pixelDurationM) {
        if (newPulse) {
          newPulse = false;
          unsigned long currentTime = micros();
          unsigned long pulseInterval = currentTime - lastPulseTime;
          lastPulseTime = currentTime;

          if (pulseInterval) {
            frq = 1000000 / pulseInterval;  // Frequency in Hz
          }
        }
      }

      pixelStartTime += pixelDurationM;

      // Limit and store the frequency value in the buffer
      frq = constrain(frq, 1500, 2300);
      uint8_t color = map(frq, 1500, 2300, 0, 255);



      if (sMode == 2) {
        if (pass == 0) {
          bufG[x + 3] = color;
        } else if (pass == 1) {
          bufB[x + 2] = color;
        } else if (pass == 2) {
          bufR[x] = color;
        }

        x++;

      }

      else if (sMode == 4) {

        if (pass == 0) {
          bufG[x + 4] = color;
          bufG[x + 5] = color;
        } else if (pass == 1) {
          bufB[x + 2] = color;
          bufB[x + 3] = color;
        } else if (pass == 2) {
          bufR[x] = color;
          bufR[x + 1] = color;
        }

        x += 2;
      }

    }  // end while x < 320

    // Reset for the next pass
    x = 0;
    pass++;
    delayMicroseconds(1200);  // Sync porch - time needed to draw pixels
  }

  // Fix Color Mapping and prepare the buffer
  for (x = 15; x < 320; x++) {
    uint16_t red = (bufR[x] >> 3) << 11;
    uint16_t green = (bufG[x] >> 2) << 5;
    uint16_t blue = (bufB[x] >> 3);
    lineBuffer[x - 15] = red | green | blue;
  }

  // Set the address window and push colors
  tft.startWrite();
  tft.setAddrWindow(0, line, 305, 1);
  tft.pushColors(lineBuffer, 305, true);
  tft.endWrite();
}


//##################################################################################################################/

void waitForHorizontalSyncPulse() {
 volatile uint32_t currentTime = 0;
  uint16_t ctr = 0;
  while (true) {



    if ((ctr & 0x0F) == 0) {  // encoder read
      if (!digitalRead(ENCODER_BUTTON)) return;
    }


    if (newPulse) {
      newPulse = false;
      currentTime = micros();
      unsigned long pulseInterval = currentTime - lastPulseTime;
      lastPulseTime = currentTime;
      ctr++;
      if (pulseInterval) {
        frq = 1000000 / pulseInterval;
      }
    }

    if ((frq <= 1250 && frq >= 1150) || ctr > 60 ) {  // break also  if no sync pulse detected so that screen does not get stuck. Ctr 60 is around 440 ms syncInterval.
      break;
    }
  
  }
}

//##########################################################################################################################/


void waitForVerticalSyncPulse() {  // Wait for transmission start

  uint16_t ctr = 0;


  tft.setCursor(10, 280);
  tft.print("Waiting for vertical sync...");
  tft.setCursor(10, 300);
  tft.print("Press encoder to skip");

  while (1) {



    if (newPulse) {
      newPulse = false;
      unsigned long currentTime = micros();
      unsigned long pulseInterval = currentTime - lastPulseTime;
      lastPulseTime = currentTime;

      if (pulseInterval > 0) {
        frq = 1000000 / pulseInterval;  // Frequency in Hz
      }
    }

    if (frq > 1880 && frq < 1920) {  // vertical start freq detected
      delayMicroseconds(100);
      ctr++;
      //tft.fillRect(400, 280, 40, 15, TFT_BLACK);
      //tft.setCursor(400, 280);
      // tft.printf("%d", ctr);
    }

    else if (millis() % 50 == 0 && ctr && (frq < 1880 || frq > 1920))  // decrease counter over time to eliminate false positives
      ctr--;

    if ((ctr > 10) || (digitalRead(ENCODER_BUTTON) == LOW)) {  // 10 * consecutive 1900Hz freq detected, assume this is a SSTV transmission
      tft.fillRect(10, 280, 400, 16, TFT_BLACK);
      tft.fillRect(10, 300, 400, 16, TFT_BLACK);
      tft.fillRect(400, 280, 40, 15, TFT_BLACK);

      tft.setCursor(10, 280);
      tft.print("Use encoder for fine tune.");
      tft.setCursor(10, 300);
      tft.print("Press encoder to leave.");
      break;
    }
  }
}
//##########################################################################################################################/



uint16_t frequencyToColorComponent(uint16_t greenFreq, uint16_t blueFreq, uint16_t redFreq) {
  // Map frequency ranges to colors
  uint16_t color = (redFreq >> 3) << 11 | (greenFreq >> 2) << 5 | (blueFreq >> 3);
  return color;
}

//##########################################################################################################################/


void encoderModifyFreq() {  // debug function


  if (clw || cclw) {

    if (clw) {
      FREQ += 10;
      clw = false;
    }

    if (cclw) {
      FREQ -= 10;
      cclw = false;
    }

    tft.fillRect(340, 280, 130, 16, TFT_BLACK);
    tft.setCursor(340, 280);
    tft.printf("F:%ld", FREQ);
    setLO();
  }
}

//##########################################################################################################################/

void determineHorizontalSyncInterval() {


  uint16_t ctr = 0;
  static long lastTime;
  unsigned long currentTime = 0;
  while (true) {


    if ((ctr & 0x0F) == 0) {  // encoder read
      if (!digitalRead(ENCODER_BUTTON)) return;
    }


    if (newPulse) {
      newPulse = false;
      currentTime = micros();
      unsigned long pulseInterval = currentTime - lastPulseTime;
      lastPulseTime = currentTime;
      ctr++;
      if (pulseInterval) {
        frq = 1000000 / pulseInterval;
      }
    }

    if (frq <= 1250 && frq >= 1150) {
      syncInterval = currentTime - lastTime;  // syncInterval in microseconds
      lastTime = currentTime;
      break;
    }
  }
}

#endif