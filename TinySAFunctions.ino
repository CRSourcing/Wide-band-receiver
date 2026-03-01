void tinySAConfigScreen() {  // Config tinySA through SecondScreen button

  redrawMainScreen = true;

#ifndef TINYSA_PRESENT
  return;
#endif

#ifdef TINYSA_PRESENT
  drawTSABtns();
  readTSABtns();
#endif
}

//##########################################################################################################################//
#ifdef TINYSA_PRESENT

void drawTSABtns() {

  draw12Buttons(TFT_VIOLET, TFT_PURPLE);
  etft.setTTFFont(Arial_14);

  if (altStyle)
    etft.setTextColor(textColor);
  else
    etft.setTextColor(TFT_PINK);


  struct Button {
    const int x;
    const int y;
    const char* text;
  };

  Button buttons[] = {
    { 20, 132, "Span" }, { 20, 153, "10K" }, { 102, 132, "Span" }, { 102, 153, "50K" }, { 182, 132, "Span" }, { 182, 153, "200K" }, { 266, 132, "Span" }, { 270, 151, "500K" }, { 25, 190, "Span" }, { 25, 210, "1M" }, { 110, 190, "Span" }, { 110, 210, "2M" }, { 190, 190, "Span" }, { 190, 210, "10M" }, { 270, 200, "Listen" }, { 272, 210, "" }, { 276, 245, "Rbw" }, { 276, 268, "Auto" }, { 192, 245, "Rbw" }, { 195, 268, "10K" }, { 105, 245, "Water" }, { 110, 268, "Fall" }
  };


  for (int i = 0; i < 22; i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].text);
  }


  drawButton(8, 236, 75, 49, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(20, 255);
  etft.print("Back");
  etft.setTextColor(textColor);
  tDoublePress();
}


//##########################################################################################################################//

void readTSABtns() {


  if (!pressed) return;

  while (true) {  // run a loop, do not automatically return to main menu


    int buttonID = getButtonID();
    if (!buttonID)
       return;  // outside of area
    switch (buttonID) {
      case 21:

        Serial.println("rbw 3");
        delay(50);
        Serial.println("sweep span 10k");  // Span 10K
        span = 10000;
        tPress();
        break;

      case 22:
        Serial.println("rbw 3");
        delay(50);
        Serial.println("sweep span 50k");
        span = 50000;
        tPress();
        break;

      case 23:
        Serial.println("rbw 10");
        delay(50);
        Serial.println("sweep span 200k");
        span = 200000;
        tPress();
        break;

      case 24:
        Serial.println("rbw 11");
        delay(50);
        Serial.println("sweep span 500k");
        span = 500000;
        tPress();
        break;
      case 31:
        Serial.println("rbw 10");
        delay(50);
        Serial.println("sweep span 1M");
        span = 1000000;
        tPress();
        break;

      case 32:
        Serial.println("rbw 30");
        delay(50);
        Serial.println("sweep span 2M");
        span = 2000000;
        tPress();
        break;

      case 33:
        Serial.println("rbw 100");
        delay(50);
        Serial.println("sweep span 10M");
        span = 10000000;
        tPress();
        break;

      case 34:  // Listen menu. Disables SI4735 audio output and enables TinSA audio output
        listenToTinySA();
        break;
      case 41:
        redrawMainScreen = true;
        tx = ty = pressed = 0;
        return;
        break;
      case 42:                       // enable/disable waterfall.
        Serial.println("menu 6 2");  //
        tPress();
        break;

      case 43:
        Serial.println("rbw 10");  // Set resolution bandwidth 10KHz (slow)
        tPress();
        break;

      case 44:
        Serial.println("rbw auto");  // Set resolution bandwidth auto
        tPress();
        break;

      default:
        redrawMainScreen = true;
        tx = ty = pressed = 0;
        return;
    }
    redrawMainScreen = true;
    tRel();
  }
}

//##########################################################################################################################//

void synctinySA() {  // sync with tuned FREQ and extract signal strength of marker 1


  if (syncEnabled == false || showAGCGraph) {  
    return;
  }


  static long OLDFREQ = FREQ;
  static bool setFREQtoMarker1 = false;
  static bool setFREQtoMarker2 = false;
  static bool blink = false;
  static bool init = false;
   

  blink = !blink;

  char buffer[50];                 // buffer for commands
  char marker1Buffer[50] = { 0 };  // buffer for marker 1 data received fromTSA
  char marker2Buffer[50] = { 0 };  // buffer for marker 2 data received fromTSA
  char extractedSS[50] = { 0 };    //  buffer for marker 1 extracted signal strength


  if (syncEnabled) {

    if (!init) {  // force TSA to init when program starts
      init = true;
      resetTSA();
      return;
    }

    Serial.println("marker 1");
    delay(1);
    Serial.readBytesUntil(0, marker1Buffer, 50);  // reads string from marker 1, readings get stuck when too frequent
    delay(1);
    Serial.println("marker 2");  // reads string from marker 2,  peak signal
    delay(1);
    Serial.readBytesUntil(0, marker2Buffer, 50);


    sscanf(marker1Buffer, "%*[^-] %[-0-9.eE+]", extractedSS);  // extract dBm from marker 1 string
    convertTodBm(extractedSS);

    long freqM1 = 0, freqM2 = 0;

    sscanf(marker1Buffer, "%*s %*s %*s %*s %ld -", &freqM1);  // extract frequency from marker 1 string
    sscanf(marker2Buffer, "%*s %*s %*s %*s %ld -", &freqM2);  // extract frequency from marker 2 string

  if (! (marker1Buffer[0] + marker2Buffer[0])) //
   
    if (serialDebugTSA) {  // debug frequency and markers
      tft.fillRect(0, 121, 336, 171, TFT_BLACK);
      tft.setTextColor(TFT_GREEN);
      tft.setCursor(0, 125);
      tft.println(marker1Buffer);
      tft.setCursor(0, 170);
      tft.println(marker2Buffer);
      if (digitalRead(ENCODER_BUTTON) == LOW)
        serialDebugTSA = false;
      delay(500);
    }


    if (freqM1 > MIN_FREQ) {
      // Marker 1 blink when in use
  
      if (blink && setFREQtoMarker1)
        tft.fillRoundRect(257, 300, 30, 19, 4, TFT_BLUE);

      if (!blink) {
      tft.fillRoundRect(257, 300, 30, 19, 4, TFT_GREEN);
      tft.fillRoundRect(233, 300, 15, 19, 4, TFT_ORANGE);       // Sync
      tft.fillRoundRect(295, 300, 30, 19, 4, TFT_RED);        // Marker 2
      tft.fillRoundRect(337, 300, 42, 19, 4, TFT_PINK);  // Marker 2 Follow
      tft.fillRoundRect(387, 300, 42, 19, 4, TFT_YELLOW);



      tft.setTextColor(TFT_BLACK);
      tft.setTextSize(2);
      tft.setCursor(235, 302);
      tft.print("R");
      tft.setCursor(262, 302);
      tft.print("M1");
      tft.setCursor(300, 302);
      tft.print("M2");
      tft.setCursor(340, 302);
      tft.print("M2F");
      tft.setCursor(392, 302);
      tft.print("Cfg");
      tft.setTextColor(textColor);
      }


      if (ty > 293 && tx >= 233 && tx <= 255) {  // manually sync the TSA
        resetTSA();
        sineTone(880, 100);
        sineTone(1000, 100);
        tx = ty = 0;
      }


      if (ty > 293 && tx >= 295 && tx <= 338) {
        encLockedtoSynth = true;

        if (use1MHzSteps) {
          use1MHzSteps = false;
          STEP = OLDSTEP;
        }
        sineTone(880, 100);
        tRel();
        FREQ = freqM2;  // set FREQ to marker2 (peak marker) once
        FREQ /= 5000;   //round down to nearest 5K
        FREQ *= 5000;
        tx = ty = 0;
      }



      if (ty > 293 && tx >= 337 && tx <= 377) {  // set FREQ to marker2 and follow marker when squelch is closed
        setFREQtoMarker2 = !setFREQtoMarker2;
        encLockedtoSynth = true;
        if (use1MHzSteps) {
          use1MHzSteps = false;
          STEP = OLDSTEP;
        }
        tRel();

        if (clw || cclw) {  // leave when encoder moved
          FREQ = freqM2;
          tx = ty = 0;
        }

        if (setFREQtoMarker2) {
          showAudioWaterfall = false;  // disable in tracking mode, needs squelch
          if (audioMuted == true) {    // only if squelch is closed
            FREQ = freqM2;
            FREQ /= 5000;  // round down to nearest 5K
            FREQ *= 5000;
            OLDFREQ = FREQ;
            tft.fillRoundRect(337, 300, 42, 19, 4, TFT_BLACK);
            tft.setTextColor(textColor);
          }
        }
        return;
      }

      if (ty > 293 && tx >= 257 && tx <= 287) {  // set FREQ to marker1 (after manually moving the marker)
        setFREQtoMarker1 = !setFREQtoMarker1;
        encLockedtoSynth = true;
        if (use1MHzSteps) {
          use1MHzSteps = false;
          STEP = OLDSTEP;
        }
        sineTone(880, 100);
        tRel();
        tx = ty = 0;
      }

      if (setFREQtoMarker1) {
        if (clw || cclw)
          setFREQtoMarker1 = false;
        FREQ = freqM1;
        FREQ /= 5000;  // round down to nearest 5K
        FREQ *= 5000;
        OLDFREQ = FREQ;
        tx = ty = 0;
        return;
      }
    }

    if (ty > 293 && tx >= 387 && tx < 450)
      cfgTSA();

  }  // endif tinySADataSource


  centerTinySA();

  long DELTA = FREQ - OLDFREQ;
  if (DELTA > 4000 || DELTA < -4000) {  // update when change of FREQ more than 4KHz, this is roughly 1 pixel of the tinySA using 1 MHz span
    if (tinySACenterMode == true) {
      sprintf(buffer, "sweep center %ld", FREQ);  // sync TSA center frequency with receiver frequency. This is slow!
      Serial.println(buffer);
    } else {
      sprintf(buffer, "marker 1 %ld", FREQ);  // set position marker one at the current frequency
      Serial.println(buffer);
    }
    OLDFREQ = FREQ;
  }
}
//##########################################################################################################################//
void centerTinySA() {  // centers TSA when FREQ moves outside window


  static long CENTER_FREQ = FREQ;

 if (FREQ >= 350000000l) // out of range, avoid TSA locking up
   return;


  char buffer[50];  // buffer for arguments

  if (selected_band == -1 && tinySACenterMode == false) {  // window mode and no band selected

    if (FREQ > (CENTER_FREQ + span / 2)) {  //  0.5 span reached, shift center frequency
      while (FREQ > (CENTER_FREQ + span / 2))
        CENTER_FREQ += span / 2;

      CENTER_FREQ += span / 2;
      CENTER_FREQ /= span;  // Round CENTER_FREQ down to nearest multiple of span
      CENTER_FREQ *= span;
      CENTER_FREQ += span / 2;
      sprintf(buffer, "sweep center %ld", CENTER_FREQ);
      Serial.println(buffer);  // shift center frequency
      delay(20);
      sprintf(buffer, "marker 1 %ld", FREQ);  // set position marker one at the current frequency
      Serial.println(buffer);
      return;
    }

    else if (FREQ < (CENTER_FREQ - span / 2)) {  //  0.5 span reached, shift center frequency
      while (FREQ < (CENTER_FREQ - span / 2))
        CENTER_FREQ -= span / 2;
      CENTER_FREQ -= span / 2;

      CENTER_FREQ /= span;  // Round CENTER_FREQ down to nearest multiple of span
      CENTER_FREQ *= span;
      CENTER_FREQ += span / 2;
      sprintf(buffer, "sweep center %ld", CENTER_FREQ);
      Serial.println(buffer);  // shift center frequency
      delay(100);
      sprintf(buffer, "marker 1 %ld", FREQ);  // set position marker one at the current frequency
      Serial.println(buffer);
      return;
    }
  }
}

//#######################################################################################################//


void synctinySAWaterfall(long midPoint, long startPoint, long endPoint) {  // syncs TSA waterfall with waterfall()

  char buffer[50];
  delay(100);
  Serial.println("load 0");  // preset 0
  delay(100);
  sprintf(buffer, "sweep center %ld", midPoint);
  Serial.println(buffer);
  delay(100);
  sprintf(buffer, "sweep span %ld", (endPoint - startPoint));
  Serial.println(buffer);
  delay(100);
}

//##########################################################################################################################//

void initTSA() {

  char buffer[50];
  
  Serial.println("load 0");  // preset 0
  delay(100);
  sprintf(buffer, "sweep center %ld", FREQ);
  Serial.println(buffer);  // set the tinySA center to current FREQ
  delay(100);
  centerTinySA();
  delay(100);
  sprintf(buffer, "sweep span %ld", span);
  Serial.println(buffer);  // change span to default
  delay(100);
  Serial.println("rbw 16");  // default resolution bandwidth
  delay(100);
  sprintf(buffer, "marker 1 %ld", FREQ);  // set marker 1 on current freq
  Serial.println(buffer);
}
//##########################################################################################################################//

void listenToTinySA() {  // Activate "Listen menu"

  si4735.setAudioMute(true);
  si4735.setHardwareAudioMute(false);
  Serial.println("rbw 100");
  delay(50);
  Serial.println("menu 4 7");
  tPress();
  Serial.println("reset");
  delay(1000);
  Serial.println("rbw 10");
  si4735.setAudioMute(false);
  redrawMainScreen = true;
  tx = ty = pressed = 0;
}

//##########################################################################################################################//

void convertTodBm(char* extractedSS) {

  static int t = 0;
  t++;

  if (t == 3) {

    TSAdBm = (int)atof(extractedSS);

    //Serial.printf("tinySA dBm: %d\n", TSAdBm);
    if (syncEnabled && TSAdBm) {  // we have a valid dBm read from the TSA
      
      if (tinySAfound == false) {
        tft.fillRect(230, 296, 244, 24, TFT_BLACK); // erase "No sync..."
        tinySAfound = true;
      }  
     calculateAndDisplaySignalStrength();
    }


    if (syncEnabled && !TSAdBm) { 
      tinySAfound = false;
      tft.fillRect(230, 296, 244, 24, TFT_BLACK);
      tft.setCursor(235, 302);
      tft.setTextColor(TFT_RED);
      tft.print("No sync with tinySA!");
    }

    t = 0;
  }
}

//##########################################################################################################################//

void resetTSA() {  //forces TSA to center when needed, or by pressing orange "R" button

  if (tinySACenterMode == true)
    return;

  long CENTER_FREQ = FREQ;
  char buffer[50] = { 0 };  // buffer for arguments


  CENTER_FREQ /= span;
  CENTER_FREQ *= span;
  CENTER_FREQ += span / 2;
  span = DEFAULT_SPAN;


  sprintf(buffer, "sweep center %ld", CENTER_FREQ);
  Serial.println(buffer);  // set center frequency
  delay(150);

  sprintf(buffer, "sweep span %ld", span);
  Serial.println(buffer);
  delay(100);
  
  Serial.println("rbw 10");  // reset resolution bandwidth
  delay(100);
  sprintf(buffer, "marker 1 %ld", FREQ);  // set marker 1 on current freq
  Serial.println(buffer);
  delay(50);
}


//##########################################################################################################################//

void cfgTSA() {  // Touch "Cfg" in lower right corner to open Config Menu


  char buffer[50];  // argument buffer
  int dBScale[] = { 1, 2, 5, 10, 20 };
  int rbwScale[] = { 3, 10, 30, 100, 300, 600 };
  static int dBIndex = 2;   // scale 5dB default
  static int rbwIndex = 1;  // rbw 10Khz default
  int rbw = 10;             // 10kHz default


  sineTone(880, 100);
  sprintf(buffer, "rbw %d", rbw);  // set to default
  Serial.println(buffer);
  delay(100);

  tft.fillRect(0, 296, 479, 23, TFT_BLACK);  // clear infobar
  
  if(! useNixieDial)
    tft.fillRect(330, 3, 135, 20, TFT_BLACK);  // clear microvolt indicator area
  
  tft.fillRect(3, 61, 336, 230, TFT_BLACK);  // clear  area for buttons

  drawButton(35, 95, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);   //div +
  drawButton(235, 95, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);  //div-

  drawButton(35, 162, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);   // span -
  drawButton(235, 162, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);  // span +

  drawButton(135, 162, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);  // center

  drawButton(35, 230, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);   // span -
  drawButton(235, 230, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);  // span +

  tft.setTextColor(TFT_GREEN);

  tft.setCursor(45, 115);
  tft.print("Div -");
  tft.setCursor(245, 115);
  tft.print("Div +");

  tft.setCursor(45, 182);
  tft.print("Span-");
  tft.setCursor(245, 182);
  tft.print("Span+");


  tft.setCursor(45, 250);
  tft.print("Rbw -");
  tft.setCursor(245, 250);
  tft.print("Rbw +");

  tft.setTextColor(TFT_RED);
  tft.setCursor(150, 182);
  tft.print("Back");
  tft.setTextColor(TFT_GREEN);

  tx = ty = 0;


  tft.fillRect(5, 65, 333, 16, TFT_GREY);
  tft.setCursor(10, 65);
  tft.printf("Sp:%ldK  Div:%ddB  Bw:%dK", span / 1000, dBScale[dBIndex], rbwScale[rbwIndex]);

  while (true) {
    pressed = false;
    tx = ty = 0;
    tPress();


    if (ty > 159 && ty < 215 && tx > 147 && tx < 222)
      break;


    if (ty > 159 && ty < 215 && tx > 35 && tx < 110) {  // zoom span in
      span /= 2;
      if (span < 15625)
        span = 15625;
      delay(300);
      sprintf(buffer, "sweep span %ld", span);
      Serial.println(buffer);
      delay(300);
    }


    if (ty > 159 && ty < 215 && tx > 235 && tx < 330) {  // zoom span out
      span *= 2;
      if (span > 16000000)
        span = 16000000;
      delay(300);
      sprintf(buffer, "sweep span %ld", span);
      Serial.println(buffer);
      delay(300);
    }



    if (ty > 85 && ty < 145 && ((tx > 35 && tx < 110) || (tx > 235 && tx < 330))) {  // change dB scale

      dBIndex += (tx > 235 && tx < 330) ? 1 : -1;
      dBIndex = (dBIndex < 0) ? 0 : (dBIndex > 4 ? 4 : dBIndex);

      Serial.println("menu 4 2");
      delay(100);
      sprintf(buffer, "text %d", dBScale[dBIndex]);
      Serial.println(buffer);
      delay(100);
    }


    if (ty > 230 && ty < 285 && tx > 35 && tx < 110) {  // zoom rbw in
      rbwIndex--;
      if (rbwIndex < 0)
        rbwIndex = 0;
      delay(300);
      sprintf(buffer, "rbw %d", rbwScale[rbwIndex]);
      Serial.println(buffer);
      delay(300);
    }

    if (ty > 230 && ty < 285 && tx > 235 && tx < 330) {  // zoom rbw out
      rbwIndex++;
      if (rbwIndex > 5)
        rbwIndex = 5;
      delay(300);
      sprintf(buffer, "rbw %d", rbwScale[rbwIndex]);
      Serial.println(buffer);
      delay(300);
    }

    tft.fillRect(5, 65, 333, 16, TFT_GREY);
    tft.setCursor(10, 65);
    tft.printf("Sp:%ldK  Div:%ddB  Rbw:%dK", span / 1000, dBScale[dBIndex], rbwScale[rbwIndex]);

  }  // end while (true)

  delay(100);
  sprintf(buffer, "marker 1 %ld", FREQ);  // reset marker 1 to tuned frequency, needed after span has changed
  Serial.println(buffer);
  delay(100);

  tRel();
  pressed = false;
  tx = ty = 0;
  redrawMainScreen = true;
  mainScreen();
}



//##########################################################################################################################//

void showSelectedBand() {

  BandInfo& sel = bandList[selected_band];

  if (tinySACenterMode == false) {
    char buffer[50];
    long midPoint = (sel.stopFreqKHz + sel.startFreqKHz) / 2;
    sprintf(buffer, "sweep center %ld", midPoint * 1000);
    Serial.println(buffer);
    delay(100);
    sprintf(buffer, "marker 1 %ld", FREQ);  // set marker 1 on tuned frequency
    Serial.println(buffer);
    delay(100);
    sprintf(buffer, "sweep span %ld", (sel.stopFreqKHz - sel.startFreqKHz) * 1000);  // display selected band
    Serial.println(buffer);
    delay(100);
    Serial.println("marker 2 peak");
    delay(100);
    sprintf(buffer, "rbw %d", (int)(sel.stopFreqKHz - sel.startFreqKHz) / 100);  // set resolution to /100 of span
    Serial.println(buffer);
  }
}

#endif