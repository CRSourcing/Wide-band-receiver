void ConfigScreen2() {

  if (!altStyle)  // clear  background
    tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  else
    drawButton(2, 61, 337, 228, TFT_NAVY, TFT_DARKGREY);  //Background
  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);


  redrawMainScreen = true;
  drawCf2Btns();
  readCf2Btns();
}

//##########################################################################################################################//

void drawCf2Btns() {


  struct Button {
    int x;
    int y;
    const char* label;
  };

  Button buttons[] = {
    { 20, 132, "Touch" },
    { 20, 153, "Calibr." },
    { 100, 130, "Touch" },
    { 100, 151, "Sound" },
    { 185, 132, "Audio" },
    { 180, 151, "Waterf." },
    { 265, 132, "Plain/" },
    { 265, 151, "Sprite" },
    { 20, 188, "Sprite" },
    { 20, 210, "Style" },
    { 100, 188, "Snap" },
    { 100, 210, "Freq" },
#ifndef NBFM_DEMODULATOR_PRESENT
    { 185, 190, "NBFM" },
    { 188, 210, "Shift" },
#endif
#ifdef NBFM_DEMODULATOR_PRESENT
    { 185, 190, "Zero" },
    { 188, 210, "Discr." },
#endif
    { 262, 190, "Waterf" },
    { 262, 210, "Colors" },
#ifdef TV_TUNER_PRESENT
    { 100, 245, "Tuner" },
    { 100, 265, "+-ppm" },
#endif
    { 185, 245, "RFgain" },
    { 185, 265, "Calibr." },
    { 270, 245, "FFT" },
    { 270, 268, "Gain" }

  };

  etft.setTextColor(TFT_PINK);

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

void readCf2Btns() {

  if (!pressed) return;
  int buttonID = getButtonID();

  if (row < 2 || row > 4 || column > 4)
    return;  // outside of area

  switch (buttonID) {
    case 21:
      touchCal();
      rebuildMainScreen(1);
      break;
    case 22:
      pressSound = !pressSound;
      preferences.putBool("pressSound", pressSound);
      break;
    case 23:
      showAudioWaterfall = !preferences.getBool("audiowf", 0);
      preferences.putBool("audiowf", showAudioWaterfall);
      tft.setCursor(10, 65);
      tft.printf("Audio Waterfall: %s\n", showAudioWaterfall ? "Active" : "OFF");
      if (showAudioWaterfall) {
        tft.setCursor(10, 83);
        tft.printf("after %ds inactivity", TIME_UNTIL_ANIMATIONS
);
      }
      delay(1000);
      break;
    case 24:
      altStyle = !altStyle;  // change between plain and sprite style
      preferences.putBool("lastStyle", altStyle);
      drawBigBtns();  // redraw with new style
      break;
    case 31:
      selectButtonStyle();
      break;
    case 32:
      roundToStep = !roundToStep;
      tft.setCursor(10, 75);
      tft.printf("Snap Freq to Step: %s\n", roundToStep ? "True" : "False");
      // round  FREQ up or down to the next STEP when STEP or modulation gets changed
      delay(1000);
      break;
    case 33:
#ifdef NBFM_DEMODULATOR_PRESENT
      zeroDiscriminator();
#endif
#ifndef NBFM_DEMODULATOR_PRESENT
      setNBFMOffset();
#endif

      break;
    case 34:
      smoothColorGradient = !smoothColorGradient;
      preferences.putBool("smoothWF", smoothColorGradient);
      tft.setCursor(10, 75);
      tft.printf("Smooth waterf. colors: %s", smoothColorGradient ? "Yes" : "No");
      delay(1000);
      break;
    case 41:
      break;
    case 42:
#ifdef TV_TUNER_PRESENT
      correctTunerOffset();
#endif
      break;
    case 43:
      setRFGainCorrection();
      break;
    case 44:
      setFFTGain();
      break;
    default:
      redrawMainScreen = true;
      tx = ty = pressed = 0;
      return;
  }

  redrawMainScreen = true;
  tRel();
}



//##########################################################################################################################//

void touchCal() {

  tft.fillScreen(TFT_BLACK);               // Clear the screen
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Set text color to white with black background
  tft.setTextSize(2);

  uint16_t calData[5];

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 0);

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.println("Touch corners as indicated");

  tft.println();

  tft.calibrateTouch(calData, TFT_WHITE, TFT_RED, 15);

  tft.setTextSize(2);
  tft.setCursor(0, 30);
  tft.print("Calibration complete!\nTouch screen with stylus\nto test different areas.\nDot must appear close to stylus\nPRESS encoder to save\nOr MOVE encoder to recalibrate\n");
  tft.printf("Cal Data: %d, %d, %d, %d, %d\n", calData[0], calData[1], calData[2], calData[3], calData[4]);
  
  // Display calibration data on the screen
 /*
 
 tft.setCursor(0, 120);
  tft.printf("Cal Data: %d, %d, %d, %d, %d\n", calData[0], calData[1], calData[2], calData[3], calData[4]);
  tft.println("");
  tft.println("");
  tft.println("Caldata should look similar to:");
  tft.println("324, 3335, 427, 2870, 5");
  tft.println("");
  tft.println("Repeat calibration if values look far");
  tft.println("off or buttons respond wrong");
*/

  while (1) {
        if ((digitalRead(ENCODER_BUTTON) == LOW))
          break;
        
        if (clw || cclw){
        clw = false;
        cclw = false;
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 20);
        tft.print("Recalibration\nTouch screen with stylus\nto test different areas\n\n");
        tft.calibrateTouch(calData, TFT_WHITE, TFT_RED, 15); 
        tft.printf("Cal Data: %d, %d, %d, %d, %d\n", calData[0], calData[1], calData[2], calData[3], calData[4]);
        }
        
        get_Touch();
        if (pressed)
          tft.fillCircle (tx,ty,3, TFT_YELLOW);
        pressed = false;
        delay(10);
         tft.fillCircle (tx,ty,3, TFT_BLACK);
  }

  //write to flash
  preferences.putInt("cal0", calData[0]);
  preferences.putInt("cal1", calData[1]);
  preferences.putInt("cal2", calData[2]);
  preferences.putInt("cal3", calData[3]);
  preferences.putInt("cal4", calData[4]);

  // Apply calibration data
  tft.setTouch(calData);

 tft.print("Calibration data saved.");

delay(1000);

}

//##########################################################################################################################//

void setNBFMOffset() {  // set best offset from crystal filter center frequency for flank demodulation

  int offset = 0, oldOffset = 0;
  encLockedtoSynth = false;

  tft.fillRect(5, 65, 333, 225, TFT_BLACK);

  tft.setCursor(5, 65);
  tft.print("Use encoder to set NBFM");


  tft.setCursor(5, 82);
  tft.print("shift for flank demodulation");

  tft.setCursor(5, 100);
  tft.print("Set for least distortion");

  tft.setCursor(5, 120);
  tft.print("Press encoder to save");

  offset = preferences.getInt("NBFMOffset", 0);
  tft.fillRect(5, 140, 320, 17, TFT_BLACK);
  tft.setCursor(5, 140);
  tft.printf("NBFM Offset: %d", offset);


  while (digitalRead(ENCODER_BUTTON) == HIGH) {

    if (oldOffset != offset) {
      tft.fillRect(5, 140, 320, 17, TFT_BLACK);
      tft.setCursor(5, 140);
      tft.printf("NBFM Offset: %d KHz", offset);
      si4735.setAM(520, 29900, SI4735TUNED_FREQ + offset, 1);
      oldOffset = offset;
    }

    delay(50);
    if (clw) {
      offset++;
    }

    if (cclw) {
      offset--;
    }

    clw = false;
    cclw = false;
  }

  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;

  preferences.putInt("NBFMOffset", offset);

  loadSi4735parameters();

  encLockedtoSynth = true;
  clearStatusBar();
}

//##########################################################################################################################//

#ifdef TV_TUNER_PRESENT

void correctTunerOffset() {  //corrects tuner frequency error.

  tunerOffsetPPM = preferences.getInt("tunerOffsetPPM", 0);
  encLockedtoSynth = false;

  tft.fillRect(5, 63, 333, 229, TFT_BLACK);
  calculateAndDisplaySignalStrength();

  tft.setCursor(5, 90);
  tft.print("Use encoder to compensate");

  tft.setCursor(5, 107);
  tft.print("for tuner freq error.");

  tft.setCursor(5, 130);
  tft.print("Press encoder to save.");
  tft.fillRect(5, 170, 320, 17, TFT_BLACK);
  tft.setCursor(5, 170);
  tft.printf("Correction: %dppm", tunerOffsetPPM);


  while (digitalRead(ENCODER_BUTTON) == HIGH) {

    if (clw || cclw) {

      calculateAndDisplaySignalStrength();
      tft.fillRect(5, 170, 320, 17, TFT_BLACK);
      tft.setCursor(5, 170);
      tft.printf("Correction: %dppm", tunerOffsetPPM);
      clw = false;
      cclw = false;
    }

    delay(50);


    if (clw) {
      tunerOffsetPPM++;
    }

    if (cclw) {
      tunerOffsetPPM--;
    }


    OLDPLLFREQ = PLLFREQ - 1;  // needed to generate the I2C bus packet
    programTVTuner();          // send a new telegram
  }

  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;

  preferences.putInt("tunerOffsetPPM", tunerOffsetPPM);
  OLDPLLFREQ = PLLFREQ - 1;
  programTVTuner();
  encLockedtoSynth = true;
  clearStatusBar();
}


#endif
//##########################################################################################################################//

void setRFGainCorrection() {  // fine tune RF gain so that S meter reads correctly

  RFGainCorrection = preferences.getInt("rfgc", 6);  // load last calibration factor

  encLockedtoSynth = false;
  clearStatusBar();
  while (digitalRead(ENCODER_BUTTON) == HIGH) {
    clearNotification();

    if (clw)
      RFGainCorrection++;
    if (cclw)
      RFGainCorrection--;
    clw = 0;
    cclw = 0;

    calculateAndDisplaySignalStrength();
    tft.fillRect(5, 100, 333, 16, TFT_BLACK);
    tft.setCursor(5, 100);
    tft.printf("RFGainCorrection:%d dB", RFGainCorrection);
    delay(50);
  }
  preferences.putInt("rfgc", RFGainCorrection);
  encLockedtoSynth = true;
}

//##########################################################################################################################//

void setFFTGain() {  // sets gain (amplitude) for the AF spectrum analyzers and the minioscilloscope

  FFTGain = preferences.getInt("FFTGain", 100);  // load last calibration factor

  encLockedtoSynth = false;
  clearStatusBar();

  while (digitalRead(ENCODER_BUTTON) == HIGH) {
    clearNotification();

    if (clw)
      FFTGain += 5;
    if (cclw)
      FFTGain -= 5;
    clw = 0;
    cclw = 0;

    FFTGain = constrain(FFTGain, 1, 1024);

    tft.fillRect(5, 100, 333, 16, TFT_BLACK);
    tft.setCursor(5, 100);
    tft.printf("FFT Gain :%d ", FFTGain);
    tft.fillRect(135, 294, 86, 27, TFT_NAVY);
    FFTSample(256, 0, true);
    delay(15);
  }

  preferences.putInt("FFTGain", FFTGain);
  encLockedtoSynth = true;
}


//##########################################################################################################################//
#ifdef NBFM_DEMODULATOR_PRESENT

void zeroDiscriminator(void) {

  encLockedtoSynth = false;
  tft.setTextSize(1);
  displayText(10, 64, 0, 0, "Tune to strong signal w. precise frequency.");
  displayText(10, 76, 0, 0, "Adjust encoder for lowest reading.");
   displayText(10, 90, 0, 0, "Press encoder to leave.");
  tft.setTextSize(2);
  tft.setCursor(10, 105);
  tft.printf("Voltage: %d", afcVoltage * 5);

  while (digitalRead(ENCODER_BUTTON) == HIGH) {


    getDiscriminatorVoltage();


    delay(50);

    if (clw) {
      discriminatorZero++;
    }
    if (cclw) {
      discriminatorZero--;
    }

    if (clw + cclw) {

      tft.fillRect(10, 105, 325, 20, TFT_BLACK);
      tft.setCursor(10, 105);
      tft.printf("Voltage: %d", afcVoltage * 5); // discriminator voltage gets divided by 5 to feed the meter, so this should match
    }


    clw = false;
    cclw = false;
  }

  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;
  encLockedtoSynth = true;

  preferences.putInt("dZero", discriminatorZero);
}

#endif
