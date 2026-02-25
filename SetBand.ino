// Select Band button, selects a predefined band.

void setBand(bool isWaterfall) {  // Use Select Band button to set start freq to a specific band, also used for range view
  uint16_t yb = 58;
  int h = 8, row, column;
  int page = 0;

 const int x_positions[] = { 15, 98, 181, 264 };
 const  int y_positions[] = { 70, 128, 186, 244 };

  tRel();
  while (true) {

    tft.fillRect(2, 53, 337, 238, TFT_BLACK);  // overwrite previous content
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        drawButton(h + j * 83, (i + 1) * yb, TILE_WIDTH, TILE_HEIGHT, TFT_BTNBDR, TFT_BTNCTR);
      }
    }

    tft.setTextSize(2);


    int label_index = page * 16;
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        etft.setCursor(x_positions[j] + 8, y_positions[i]);
        const BandInfo& band = bandList[label_index];

        if (strcmp(band.bandName, "EXIT") == 0 || strcmp(band.bandName, "NEXT") == 0) {
          etft.setCursor(x_positions[j] + 5, y_positions[i]);
          etft.setTextColor(TFT_WHITE);  // color for Exit and Next
        } else if (band.isAmateurRadioBand) {
          etft.setTextColor(TFT_GREEN);  // color for amateur radio
        } else {
          etft.setTextColor(TFT_ORANGE);  // color for broadcast
        }

        etft.setTTFFont(Arial_14);
        etft.print(band.bandName);

        if (band.startFreqKHz) {
          etft.setTTFFont(Arial_8);
          etft.setCursor(x_positions[j] + 6, y_positions[i] + 24);
          etft.printf("%lu KHz", band.startFreqKHz);
        }
        label_index++;
      }
    }


    tft.fillRect(0, 294, 479, 25, TFT_BLACK);
    tft.setCursor(0, 303);
    tft.setTextColor(TFT_GREEN);
    tft.printf("Band looping: %s\n", loopBands ? "Enabled" : "Disabled");
    tPress();  // wait until pressed
    tft.fillRect(0, 300, 400, 19, TFT_BLACK);
    column = 1 + (tx / HorSpacing);  // get row and column
    row = 1 + ((ty - 40) / VerSpacing);

    selected_band = (row - 1) * 4 + (column - 1) + page * 16;
    BandInfo& sel = bandList[selected_band];

    if (strcmp(sel.bandName, "EXIT") == 0) {
      selected_band = -1;
      return;
    }

    if (strcmp(sel.bandName, "NEXT") == 0)
      page = (page + 1);  // load pages

    else {
      if (!isWaterfall) {
        FREQ = 1000ULL * sel.startFreqKHz;



#ifdef TINYSA_PRESENT
        showSelectedBand();
#endif


        break;
      }

      else {
        lim1 = 1000ULL * sel.stopFreqKHz;
        lim2 = 1000ULL * sel.startFreqKHz;
        waterFall(false);  // call waterFall range view, do not use keypad
        return;
      }
    }
  }
}

//##########################################################################################################################//

void autoloopBands() {

  if (selected_band == -1)  // no band selected
    return;

  if (loopBands == true) {  // when true and a band is loaded, freq will loop
    long freqKHz = FREQ / 1000;
    BandInfo& sel = bandList[selected_band];
    if (freqKHz < sel.startFreqKHz)
      freqKHz = sel.stopFreqKHz;
    if (freqKHz > sel.stopFreqKHz)
      freqKHz = sel.startFreqKHz;
    FREQ = freqKHz * 1000;
  }

  else {

    long freqKHz = FREQ / 1000;
    BandInfo& sel = bandList[selected_band];
    if (freqKHz < sel.startFreqKHz)
      selected_band = -1;
    else if (freqKHz > sel.stopFreqKHz)
      selected_band = -1;
  }



  if (selected_band == -1) {  // we have left the selected band, so change span back to default

    char buffer[50];
    sprintf(buffer, "sweep span %ld", span);
    delay(100);
    Serial_println(buffer);  // change span to default
    delay(100);
    Serial_println("rbw 12");  // set rbw back to default
  }
}
