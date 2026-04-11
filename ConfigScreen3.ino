void ConfigScreen3() {

  if (!altStyle)  // clear  background
    tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  else
    drawButton(2, 61, 337, 228, TFT_NAVY, TFT_DARKGREY);  //Background
  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);


  redrawMainScreen = true;
  drawCf3Btns();
  readCf3Btns();
}

//##########################################################################################################################//

void drawCf3Btns() {


  struct Button {
    const int x;
    const int y;
    const char* label;
  };

  const Button buttons[] PROGMEM = {
    { 20, 132, "Debug" },
    { 20, 153, "Info" },
    { 100, 130, "Waterf." },
    { 100, 151, "Colors" },
    { 185, 132, "Have" },
    { 185, 151, "Fun" },
    { 265, 132, "" },
    { 265, 151, "" },
    { 20, 188, "" },
    { 20, 210, "" },
    { 100, 188, "" },
    { 100, 210, "" },
    { 185, 190, "" },
    { 188, 210, "" },
    { 262, 190, "" },
    { 262, 210, "" },
    { 100, 245, "" },
    { 100, 265, "" },
    { 185, 245, "" },
    { 185, 265, "" },
    { 270, 245, "" },
    { 270, 265, "" }

  };

  etft.setTextColor(TFT_GREEN);

  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].label);
  }

  drawButton(8, 236, 75, 49, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(20, 255);
  etft.print(F("BACK"));
  ;
  etft.setTextColor(textColor);
  tDoublePress();
}
//##########################################################################################################################//

void readCf3Btns() {



  if (!pressed) return;
  int buttonID = getButtonID();

  if (!buttonID)
    return;  // outside of area

  switch (buttonID) {
    case 21:
      displayDebugInfo = !preferences.getBool("dbgI", 0);
      preferences.putBool("dbgI", displayDebugInfo);
      tft.setCursor(10, 75);
      tft.printf("DisplayDebugInfo %s", displayDebugInfo ? "Yes" : "No");
      if (!displayDebugInfo)
        tft.fillRect(0, 296, 130, 24, TFT_BLACK);
      delay(1000);
      break;
    case 22:
      smoothColorGradient = !smoothColorGradient;
      preferences.putBool("smoothWF", smoothColorGradient);
      tft.setCursor(10, 75);
      tft.printf("Smooth waterf. colors: %s", smoothColorGradient ? "Yes" : "No");
      delay(1000);
      break;
    case 23:
      funEnabled = !funEnabled;
      tft.setCursor(10, 75);
      if (funEnabled)
        tft.print(F("Have fun!"));
      else
        tft.print(F("Fun disabled"));
      preferences.putBool("fun", funEnabled);
      delay(500);
      break;
    case 24:


      break;
    case 31:
      break;
    case 32:
      break;
    case 33:
      break;
    case 34:
      break;
    case 41:
      tRel();
      return;
    case 42:
      break;
    case 43:
      break;
    case 44:
      break;
    default:
      resetMainScreen();
      return;
  }

  redrawMainScreen = true;
  tRel();
}
