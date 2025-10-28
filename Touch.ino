
//draws and reads onscreen numeric keyboard

void freqScreen() {  // displays and reads frequency numeric keypad
  drawNumPad();
  drawKeypadButtons();
  tRel();
  readKeypadButtons();

  if (modType == WBFM && (FREQ < 88000000 || FREQ > 108000000)){
     modType = AM;
    loadSi4735parameters();
  }  
}

//##########################################################################################################################//
void drawKeypadButtons() {

  tft.setTextColor(TFT_GREEN);
  char labels[9] = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };

  int x_positions[] = { 25, 108, 191 };
  int y_positions[] = {70, 128, 186  };

  int index = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      tft.setCursor(x_positions[j], y_positions[i]);
      tft.print(labels[index++]);
    }
  }

  tft.setCursor(25, 244);
  tft.print("0");

  struct TextPos {
    int x, y;
    const char* text;
  };

  TextPos textButtons[] = {
    { 191, 250, "." },
    { 96, 255, "REBOOT" },
    { 182, 250, "   " },
    { 270, 70, "KHz" },
    { 270, 128, "MHz" },
    { 270, 194, "<<" },
    { 262, 255, "RETURN" }
  };

  tft.setTextSize(2);
  tft.setTextColor(textColor);
  for (int i = 0; i < sizeof(textButtons) / sizeof(textButtons[0]); ++i) {
    tft.setCursor(textButtons[i].x, textButtons[i].y);
    tft.print(textButtons[i].text);
  }
}

//##########################################################################################################################//

bool readKeypadButtons() {

  double f = 0;
  uint16_t xPos = 10;
  uint16_t yPos = 10;
  uint16_t index = 0;
  bool decimalPoint = false;
  int decimalPosition = 0;
  double val = 0;
  long freqSave = FREQ;


  tft.fillRect(10, 3, 325, 40, TFT_BLACK);

  showFreqHistory();


  while (index < 8) {  //6 digits frequency input
    tPress();
  
if (tx > 345 && loadHistory) {// touch was in big button area, load history
    loadFreqFromHistory();
    tRel();
    tx = 0; 
    return false;
   } 

    column = 1 + (tx / HorSpacing);  //  get row and column
    row = 1 + ((ty - 40) / vTouchSpacing);

    confirmTouch(row, column);  // draws an empty button for 200ms to confirm touch

    if (column < 4) {                        // must be a digit
      if (row == 1) val = column;            // 1-3
      if (row == 2) val = column + 3;        // 4-6
      if (row == 3) val = column + 6;        //7-9
      if (row == 4 && column == 1) val = 0;  // 0
    }
    if (row == 4 && column == 3) {  // Decimal point
      if (!decimalPoint) {
        decimalPoint = true;
      }
    }

    if (!decimalPoint) {
      f *= 10;   // multiply last value
      f += val;  // add button pressed
    }

    else {
      f += val * pow(10, -decimalPosition);  // add fraction
      decimalPosition++;
    }

    if (row == 1 && column == 4) {  // Enter Khz
      tft.fillRect(10, 3, 325, 40, TFT_BLACK);
      if (!decimalPoint)
        FREQ = (long)(f * 100);
      else
        FREQ = (long)(f * 1000);

      if (FREQ > MAX_FREQ) {  // limit max input
        FREQ = freqSave;
        keyPadErr();
      }
      tRel();
      selected_band = -1;  // remove selected band
      return true;
    }


    if (row == 2 && column == 4) {  //enter Mhz
      tft.fillRect(10, 3, 325, 40, TFT_BLACK);
      if (f > MAX_FREQ/1000)
        f/=1000; // Must be KHz by user error
      if (!decimalPoint)
        FREQ = (f * 100000);
      else
        FREQ = (f * 1000000);
      
      if (FREQ < MIN_FREQ) {  // limit max input
        FREQ = freqSave;
        keyPadErr();
      }
      
      tRel();
      selected_band = -1;  // remove selected band
      return true;
    }

    if (row == 3 && column == 4) {  // <<

      if (!decimalPoint) {
        f = (long)(f / 100);  // divide by 100 because it will get multiplied by 10 agai
        index--;
      } else {
        String fString = String(f, 3);
        fString = fString.substring(0, fString.indexOf('.'));  //eliminates digits after decimal point
        f = fString.toFloat();
        decimalPoint = false;
        decimalPosition = 0;
        index -= 4;
      }
      tft.setCursor(xPos + 30, yPos);
      tft.printf("%3.3f", f);
    }

    if (row == 4 && column == 2) {            // restart ESP if pressed
      preferences.putBool("fastBoot", true);  // set fastboot
      ESP.restart();
    }


    if (row == 4 && column == 4) {  // Return
      keyVal = (long)f / 10;        // keyVal is used as frequency input for other functions
      displayFREQ(0);
      tft.fillRect(10, 3, 325, 40, TFT_BLACK);  // overwrite frequency window
      FREQ = freqSave;
      FREQ_OLD = FREQ - 1;
      displayFREQ(FREQ);
      tRel();
      return false;
    }

    if (row > 4 || column > 4)
      return false;  // outside of keypad area


    tft.fillRect(10, 3, 325, 40, TFT_BLACK);
    etft.setTextColor(TFT_GREEN);
    etft.setTTFFont(Arial_32);  // display entered digits
    etft.setCursor(xPos + 30, yPos);
    if (!decimalPoint)
      etft.printf("%ld", (long)f);
    else
      etft.printf("%3.3f", f);
    index++;

    tRel();
    val = 0;
  }

  FREQ = freqSave;  // too many digits
  keyPadErr();
  return false;
}


//##########################################################################################################################//


void keyPadErr() {

  tx = ty = pressed = 0;
  FREQ_OLD -= 1;  // no valid result
  tft.fillRect(10, 5, 325, 40, TFT_BLACK);
  etft.setCursor(10, 3);
  etft.print("Invalid entry");
  delay(500);
  tft.fillRect(10, 3, 325, 40, TFT_BLACK);
}



//##########################################################################################################################//
void drawNumPad() {
  uint16_t yb = 58;
  int h = 8;

    tft.fillRect(2, 50, 337, 240, TFT_BLACK);
 
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      drawButton(h + j * 83, (i + 1) * yb, TILE_WIDTH, TILE_HEIGHT, TFT_BTNBDR, TFT_BTNCTR);
    }
  }
}

//##########################################################################################################################//


void confirmTouch(int row, int column) {

  if (row > 4 || column > 4)
    return;
  uint16_t yb = 58;
  int h = 8;
  drawButton(h + (column - 1) * 83, (row) * yb, TILE_WIDTH, TILE_HEIGHT, TFT_BTNBDR, TFT_BTNCTR);  // draw empty button
  delay(200);
  drawKeypadButtons();
  //redraw buttons
}


//##########################################################################################################################//
// Touch functions

void tRel() {  // wait for touch release

  do {
    pressed = get_Touch();
    delay(50);
  } while (pressed);
}


//##########################################################################################################################//

void tDoublePress() {  // wait until pressed again
  do {
    get_Touch();
    delay(50);
  } while (pressed);
  delay(50);

  do {
    pressed = get_Touch();
    delay(50);
  } while (!pressed);
}


//##########################################################################################################################//

void tPress() {  // wait for press

  do {
    pressed = get_Touch();
    delay(50);
  } while (!pressed);
}

//##########################################################################################################################//

bool longPress() {

  uint32_t t1 = millis();
  const uint16_t longPressLimit = 800;
  while (true) {
    pressed = get_Touch();
    delay(10);
    uint32_t t2 = millis();
    if (t2 - t1 > longPressLimit) {
      return true;
    }  
    if (!pressed)
      return false;
  }
}

//##########################################################################################################################//

bool get_Touch() {  // implements a short beep when pressed, calls getTouch, or uses fast raw functions.

  static uint32_t t = 0;
  uint32_t tnew;
  static bool snd = true;

  tnew = millis();
  
#ifndef FAST_TOUCH_HANDLER
  pressed = tft.getTouch(&tx, &ty);
#endif

#ifdef FAST_TOUCH_HANDLER
 uint16_t x = 0, y = 0 ;
  uint16_t z = tft.getTouchRawZ();
  if ( z > 300) { 
    pressed = true;
    tft.getTouchRaw(&x, &y);  // Read raw x and y
    tft.convertRawXY(&x, &y); // Convert to screen coordinates
   tx = x;
   ty = y;
  
   if(y >= DISP_HEIGHT || x >= DISP_WIDTH) {// eliminate spurious error
      pressed = false;
      tx = 0;
      ty = 0;
      return false;
   }  
  }  
  
  else {
  pressed = false;
  }
  #endif

  if (tnew - t > 200 && !pressed)
    snd = true;

  if (pressed && pressSound && snd) {
    sineTone(440, 20);
    snd = false;
    t = millis();
  }
  
  if(pressed)
   enableAnimations = false; // stop any animation
  
  return pressed;
}

//##########################################################################################################################//

void checkTouchCoordinates() {  // touch coordinates for loop and mainscrees


  pressed = get_Touch();  // get_Touch replaces tft.getTouch() with a faster handler

  if (ttx || tty) {  // use coordinates from previous touch instead, if needed

    pressed = true;
    tx = ttx;
    ty = tty;
    ttx = 0;
    tty = 0;
  }


  if (showTouchCoordinates) {  // for debug
    tft.fillRect(320, 5, 159, 20, TFT_BLACK);
    tft.setTextColor(textColor);
    tft.setCursor(320, 5);
    tft.printf("x:%d y:%d", tx, ty);
  }

  if (pressed && ty > 293 && tx > 140 && tx < 235) {  // miniWindowMode selects btw off, low resolution, high resolution, mini osci. audio waterfall.
    tRel();
    miniWindowMode++;
  }




if (tx > 345 && loadHistory &&  redrawMainScreen == true) { // main loop interrupted, big buttons are hidden, waiting for user input
   loadFreqFromHistory();
  
}


  indicatorTouch();  //check whether indicators get touched directly
}
//##########################################################################################################################//

