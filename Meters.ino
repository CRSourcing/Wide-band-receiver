/* 
From tft_espi library (examples)

 This example uses the hardware SPI only
 Needs Font 2 (also Font 4 if using large scale label)

 Updated by Bodmer for variable meter size
*/

//##########################################################################
//  Draw the analogue meters on the screen
// #########################################################################

void analogMeter(int ySh) {

  if (scanMode)  // space occupied
    return;

  if (ySh < 50)  // frame upper meter
    tft.pushImage(345, 53, 130, 76, (uint16_t *)meterFrame);
  else
    tft.pushImage(345, 132, 130, 76, (uint16_t *)meterFrame);

  tft.setTextColor(TFT_LIGHTBULB);  // Text colour

  tft.fillCircle(379, 108, 1, TFT_LIGHTGREY);  // needle stop
  tft.fillCircle(379, 187, 1, TFT_LIGHTGREY);




  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  for (int i = -50; i < 51; i += 5) {
    // Long scale tick length
    int tl = 15;

    // Coordinates of tick to draw
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (M_SIZE * 100 + tl) + M_SIZE * 120 + xSh;
    uint16_t y0 = sy * (M_SIZE * 100 + tl) + M_SIZE * 150 + ySh;
    uint16_t x1 = sx * M_SIZE * 100 + M_SIZE * 120 + xSh;
    uint16_t y1 = sy * M_SIZE * 100 + M_SIZE * 150 + ySh;

    // Short scale tick length
    if (i % 25 != 0) tl = 6;

    // Recalculate coords incase tick lenght changed
    x0 = sx * (M_SIZE * 100 + tl) + M_SIZE * 120 + xSh;
    y0 = sy * (M_SIZE * 100 + tl) + M_SIZE * 150 + ySh;
    x1 = sx * M_SIZE * 100 + M_SIZE * 120 + xSh;
    y1 = sy * M_SIZE * 100 + M_SIZE * 150 + ySh;


    tft.drawLine(x0, y0, x1, y1, TFT_WHITE);

    // Check if labels should be drawn, with position tweaks
    if (i % 25 == 0) {
      // Calculate label positions
      x0 = sx * (M_SIZE * 100 + tl + 10) + M_SIZE * 120 + xSh;
      y0 = sy * (M_SIZE * 100 + tl + 10) + M_SIZE * 150 + ySh;

      tft.setTextSize(1);

#ifndef NBFM_DEMODULATOR_PRESENT

      switch (i / 25) {
        case -2: tft.drawCentreString("0", x0 + 4, y0 - 4, 1); break;
        case -1: tft.drawCentreString("25", x0 + 2, y0, 1); break;
        case 0: tft.drawCentreString("50", x0, y0 + 5, 1); break;
        case 1: tft.drawCentreString("75", x0, y0, 1); break;
        case 2: tft.drawCentreString("100", x0 - 10, y0 - 6, 1); break;
      }
#endif


#ifdef NBFM_DEMODULATOR_PRESENT
      if (ySh < 50) {


        if (modType != WBFM)
          switch (i / 25) {
            case -2: tft.drawCentreString("-5", x0 + 11, y0 - 6, 1); break;
            case -1: tft.drawCentreString("-2.5", x0 + 2, y0, 1); break;
            case 0: tft.drawCentreString("0", x0, y0 + 5, 1); break;
            case 1: tft.drawCentreString("2.5", x0, y0, 1); break;
            case 2: tft.drawCentreString("5", x0 - 8, y0 - 6, 1); break;
          }

        else

          switch (i / 25) {
            case -2: tft.drawCentreString("0", x0 + 8, y0 - 4, 1); break;
            case -1: tft.drawCentreString("25", x0 + 2, y0, 1); break;
            case 0: tft.drawCentreString("50", x0, y0 + 5, 1); break;
            case 1: tft.drawCentreString("75", x0, y0, 1); break;
            case 2: tft.drawCentreString("100", x0 - 10, y0 - 6, 1); break;
          }

      }

      else

        switch (i / 25) {
          case -2: tft.drawCentreString("0", x0 + 8, y0 - 4, 1); break;
          case -1: tft.drawCentreString("25", x0 + 2, y0, 1); break;
          case 0: tft.drawCentreString("50", x0, y0 + 5, 1); break;
          case 1: tft.drawCentreString("75", x0, y0, 1); break;
          case 2: tft.drawCentreString("100", x0 - 10, y0 - 6, 1); break;
        }

#endif

      tft.setTextSize(2);
    }

    // Now draw the arc of the scale
    sx = cos((i + 5 - 90) * 0.0174532925);
    sy = sin((i + 5 - 90) * 0.0174532925);
    x0 = sx * M_SIZE * 100 + M_SIZE * 120 + xSh;
    y0 = sy * M_SIZE * 100 + M_SIZE * 150 + ySh;
    // Draw scale arc, don't draw the last part
    if (i < 50) tft.drawLine(x0, y0, x1, y1, TFT_LIGHTBULB);
  }

  //tft.drawString("%RH", M_SIZE*(3 + 230 - 40), M_SIZE*(119 - 20), 2); // Units at bottom right
  //tft.drawCentreString("%RH", M_SIZE*120, M_SIZE*75, 4); // Comment out to avoid font 4
  //tft.drawRect(1+ xSh, M_SIZE*3 + ySh, M_SIZE*236, M_SIZE*126, TFT_GREY); // Draw bezel line

  plotNeedle(0, 0);  // Put meter needle at 0
  plotNeedle2(0, 0);

  tft.setTextColor(textColor, TFT_BLACK);
}

// #########################################################################


void plotNeedle(int value, int updateRate)  // upper needle, non blocking version so that the Smeter needle  moves slowly between updates
{


  if (!showMeters || scanMode)
    return;

  tft.setTextColor(TFT_LIGHTBULB, TFT_DARKVIOLET);

  if (value < 0) value = 0;  // Limit value to emulate needle end stops
  if (value > 100) value = 100;


  if (value != old_analog) {

    if (old_analog < value) old_analog += updateRate;
    else old_analog -= updateRate;


    if (!updateRate) {
      old_analog = value = 0;
    }

    float sdeg = map(old_analog, -10, 110, -150, -30);  // Map value to angle
    // Calculate tip of needle coords
    float sx = cos(sdeg * 0.0174532925);
    float sy = sin(sdeg * 0.0174532925);

    // Calculate x delta of needle start (does not start at pivot point)
    float tx = tan((sdeg + 90) * 0.0174532925);

    // Erase old needle image
    tft.drawLine(M_SIZE * (120 + 24 * ltx) - 1 + xSh, M_SIZE * (150 - 24) + ySh - 8, osx - 1 + xSh, osy + ySh, TFT_DARKVIOLET);
    tft.drawLine(M_SIZE * (120 + 24 * ltx) + xSh, M_SIZE * (150 - 24) + ySh - 8, osx + xSh, osy + ySh, TFT_DARKVIOLET);
    tft.drawLine(M_SIZE * (120 + 24 * ltx) + 1 + xSh, M_SIZE * (150 - 24) + ySh - 8, osx + 1 + xSh, osy + ySh, TFT_DARKVIOLET);

    // Re-plot text under needle



    // Store new needle end coords for next erase
    ltx = tx;
    osx = M_SIZE * (sx * 98 + 120);
    osy = M_SIZE * (sy * 98 + 150);

    // Draw the needle in the new postion, magenta makes needle a bit bolder
    // draws 3 lines to thicken needle

   tft.setTextColor(TFT_WHITE); 

#ifndef NBFM_DEMODULATOR_PRESENT
    tft.drawCentreString("RSSI", M_SIZE * 120 + xSh, M_SIZE * 75 + ySh, 1);
#else
    if (modType != WBFM) {

      tft.drawCentreString("Tune", M_SIZE * 120 + xSh, M_SIZE * 75 + ySh, 1);
    }

    else
      tft.drawCentreString("RSSI", M_SIZE * 120 + xSh, M_SIZE * 75 + ySh, 1);
#endif

    tft.drawLine(M_SIZE * (120 + 24 * ltx) - 1 + xSh, M_SIZE * (150 - 24) + ySh - 8, osx - 1 + xSh, osy + ySh, TFT_RED);
    tft.drawLine(M_SIZE * (120 + 24 * ltx) + xSh, M_SIZE * (150 - 24) + ySh - 8, osx + xSh, osy + ySh, TFT_MAGENTA);
    tft.drawLine(M_SIZE * (120 + 24 * ltx) + 1 + xSh, M_SIZE * (150 - 24) + ySh - 8, osx + 1 + xSh, osy + ySh, TFT_RED);
  }

  tft.setTextColor(textColor, TFT_BLACK);
}

// #########################################################################



void plotNeedle2(int value2, int updateRate)  // lower needle
{


  if (!showMeters || scanMode)
    return;

  tft.setTextColor(TFT_LIGHTBULB, TFT_DARKVIOLET);

  if (value2 < 0) value2 = 0;  // Limit value to emulate needle end stops
  if (value2 > 100) value2 = 100;

  // Move the needle until new value reached
  if (value2 != old_analog2) {

    if (old_analog2 < value2)
      old_analog2 += updateRate;
    else
      old_analog2 -= updateRate;


    if (!updateRate) {  // reset needles to zero
      old_analog2 = value2 = 0;
    }


    float sdeg = map(old_analog2, -10, 110, -150, -30);  // Map value to angle
    // Calculate tip of needle coords
    float sx = cos(sdeg * 0.0174532925);
    float sy = sin(sdeg * 0.0174532925);

    // Calculate x delta of needle start (does not start at pivot point)
    float tx2 = tan((sdeg + 90) * 0.0174532925);

    // Erase old needle image
    tft.drawLine(M_SIZE * (120 + 24 * ltx2) - 1 + xSh, M_SIZE * (150 - 24) + ySh2 - 8, osx2 - 1 + xSh, osy2 + ySh2, TFT_DARKVIOLET);
    tft.drawLine(M_SIZE * (120 + 24 * ltx2) + xSh, M_SIZE * (150 - 24) + ySh2 - 8, osx2 + xSh, osy2 + ySh2, TFT_DARKVIOLET);
    tft.drawLine(M_SIZE * (120 + 24 * ltx2) + 1 + xSh, M_SIZE * (150 - 24) + ySh2 - 8, osx2 + 1 + xSh, osy2 + ySh2, TFT_DARKVIOLET);

    // Re-plot text under needle
    tft.drawCentreString("VU", M_SIZE * 120 + xSh, M_SIZE * 75 + ySh2, 1);

    // Store new needle end coords for next erase
    ltx2 = tx2;
    osx2 = M_SIZE * (sx * 98 + 120);
    osy2 = M_SIZE * (sy * 98 + 150);

    // Draw the needle in the new postion, magenta makes needle a bit bolder
    // draws 3 lines to thicken needle
    tft.drawLine(M_SIZE * (120 + 24 * ltx2) - 1 + xSh, M_SIZE * (150 - 24) + ySh2 - 8, osx2 - 1 + xSh, osy2 + ySh2, TFT_RED);
    tft.drawLine(M_SIZE * (120 + 24 * ltx2) + xSh, M_SIZE * (150 - 24) + ySh2 - 8, osx2 + xSh, osy2 + ySh2, TFT_MAGENTA);
    tft.drawLine(M_SIZE * (120 + 24 * ltx2) + 1 + xSh, M_SIZE * (150 - 24) + ySh2 - 8, osx2 + 1 + xSh, osy2 + ySh2, TFT_RED);
  }

  tft.setTextColor(textColor, TFT_BLACK);
}
