//#########################################################################################################################//

void setScanRange() {

  if (!getFreqLimits()) {  // keypad return pressed
    redrawMainScreen = true;
    return;
  }
  tft.fillRect(5, 3, 470, 52, TFT_BLACK);
  tft.setCursor(12, 7);
  tft.print("1 and RETURN = stop mode");
  tft.setCursor(12, 26);
  tft.print("2 and RETURN = cont. mode");
  tPress();
  readKeypadButtons();
  tft.fillRect(5, 3,470, 52, TFT_BLACK);
  redrawMainScreen = true;
  mainScreen();
  tft.setCursor(10, 123);
  printScanRange();
  ty = 100;  // changes ty so that scanUpDown() assumes a SEEK UP press and starts to scan from low to high freeq
}

//##########################################################################################################################//

void printScanRange() {

  if (showScanRange == false)
    return;

  tft.fillRect(3, 119, 334, 58, TFT_BLACK);  // overwrite highest button row
  tft.setCursor(10, 123);
  if (keyVal == 1)
    tft.print("Stay when signal drops");
  if (keyVal == 2)
    tft.print("Move on when signal drops");
  tft.setCursor(10, 150);
  tft.printf("Range:%ldKHz - %ldKHz", lim2 / 1000, lim1 / 1000);
}
//##########################################################################################################################//

void scanUpDown(bool stop) {  // changes Freq when SCAN buttons are pressed

  uint32_t stpsize;
  if ((STEP > DEFAULT_AM_STEP) && (modType == AM))
    stpsize = DEFAULT_AM_STEP;
  else
    stpsize = STEP;

  if (ty > 40 && ty < 120 && tx > 340 && FREQ < (MAX_FREQ - STEP) && !stop) {  // SEEK UP button
    FREQ += stpsize;
  }

  if (ty > 215 && ty < 295 && tx > 340 && FREQ > (MIN_FREQ + STEP) && !stop) {  // SEEK DOWN button
    FREQ -= stpsize;
  }

  if (pressed && tx > 340 && ty > 135 && ty < 205)  // SET RANGE button
    setScanRange();
}


//##########################################################################################################################//

void freq_UP_DOWN() {  // called from setMode(), changes freq when frequency display pressed, starts slow and gets faster if no signal found


  long stpsize;
  static int dly = 150;
  static long before;
  long cycleTime;
  long now = millis();  // measure loop time
  cycleTime = now - before;
  before = now;


  if ((STEP > DEFAULT_AM_STEP) && (modType == AM) && use1MHzSteps == false)
    stpsize = DEFAULT_AM_STEP;
  else
    stpsize = STEP;

  getRSSIAndSNR(); 



  if (! showMeters) {

  if (ty > 45 && ty < 115 && FREQ < (MAX_FREQ - STEP))   // UP pressed
      FREQ += stpsize; 

  if (ty > 130 && ty < 200 && FREQ > (MIN_FREQ + STEP)) {  // DOWN pressed
      FREQ -= stpsize;
    }
  }


else if (showMeters && ! useNixieDial ) {

  if (ty < 45 && tx < 115 && FREQ < (MAX_FREQ - STEP)) {  // Freq display left = DOWN, use when meters are displayed
    FREQ -= stpsize;
  }

  if (ty < 45 && tx > 115 && tx < 330 && FREQ > (MIN_FREQ + STEP)) {  // Freq display right = UP
    FREQ += stpsize;
  }
}


if (useNixieDial) {


  if (ty < 45  && tx > 240 && tx < 350 && FREQ < (MAX_FREQ - STEP)) {  // Freq display left = DOWN, x coordinates are different when using nixie display
    FREQ -= stpsize;
  }

  if (ty < 45 && tx >= 350 && FREQ > (MIN_FREQ + STEP)) {  // Freq display right = UP
    FREQ += stpsize;
  }


 if (tx > 3 && tx < 220 && ty > 3 && ty < 65){ // fine tune through dial
    disableFFT = true;  
   dialFineTune();
  }



}





  delay(dly + 2 * signalStrength);  // slow down when we have a signal

  dly -= 10;

  if (dly <= 5)
    dly = 5;

  if (cycleTime > 300)  // touch was released
    dly = 150;          // start slow again
}



//##########################################################################################################################//


bool getFreqLimits() {
  bool result;

  drawNumPad();
  drawKeypadButtons();
  tft.fillRect (4, 4, 470, 40, TFT_BLACK);
  tft.setCursor (10, 20);
  tft.print("Enter low limit:");
  tPress();  // wait until pressed
  result = readKeypadButtons();
  if (!result)
    return false;

  lim1 = FREQ;
  tft.fillRect (4, 4, 470, 40, TFT_BLACK);
  tft.setCursor (10, 20);
  tft.print("Enter high limit:");
  tPress();
  result = readKeypadButtons();
  if (!result)
    return false;
  lim2 = FREQ;

  if (lim2 > lim1) {  // invert order
    long temp = lim2;
    lim2 = lim1;
    lim1 = temp;
  }

  return true;
}

//##########################################################################################################################//
