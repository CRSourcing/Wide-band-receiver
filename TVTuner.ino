/* Contains tuning logic for the TV tuner frontend. This logic is written for a group of Philips PAL I2C tuners build from 1995-2005 (Gen 3 and 4 MOPLL tuners).
Other Philips PAL tuners will probably work. 
Tuners from other manufacturers will probaly need another data format, but it may be possible to adapt accordingly. NTSC tuners will need to change 
const long segmentStartFreq = 36000000 to const long segmentStartFreq = 45000000 and adapt the LC circuit on the IF output of the tuner accordingly.

It contains both the logic for the tuner PLL and the SI5351 which is used as local oscillator to mix down to 21.4MHz.


Below SHORTWAVE_MODE_UPPER_LIMIT the AD831 gets connected to the antenna input via the LNA and a lowpass filter and the tuner gets disabled.
The SI4732 is always used as a demodulator at 21.4MHz. The SI5351 is set to RF+IF (highside injection).

If FREQ exceeds SHORTWAVE_MODE_UPPER_LIMIT, the AD831 gets connected to the tuner output. 
The TV tuner gets activated and converts RF to an IF spectrum roughly 7MHz wide, centering at 36MHz.
The AD831 will then use a 1MHz slot between 36 and 37MHz and mit ix down to 21.4MHz.
Since the LO is using highside injection, the spectum gets mirrored and 37MHz corresponds to the low end of the RF spectrum and 36MHz to the high end.

Once the LO frequency reaches either end, the tuner will get reprogrammed. 
SI5351 gets programmed to oscillate betwee 57.4 and 58.4MHz when the tuner is used. 

*/
#ifdef TV_TUNER_PRESENT

uint8_t addressByte = 0b1100001;       // write mode, tuner AS pin (3) open, exclude LSB
uint8_t dividerByte1 = 0b00001010;     // 0 N14 N13 N12 N11 N10 N9 N8 // set divider to 2580 (random frequency selection)
uint8_t dividerByte2 = 0b00010100;     // N7 N6 N5 N4 N3 N2 N1 N0
uint8_t controlDataByte = 0b10000000;  //charge pump 20uA, 0RSA RSB0 = PLL step size 50KHz, do not use WSB bit (not working)
uint8_t bandSwitchByte = 0b00011001;   // 0 0 0 AGC2 AGC1 SP3 SP2 SP1 = lowBand, lowAGC (not changeable)

/*
 Last 3 bits: P2 P1 P0 Selected band
0 0 1 low-band
0 1 0 mid-band
1 0 0 high-band
Samsung tuners use inverse 3 bits for band selection, Alps and Sony use 2 bits

*/

uint8_t I2CDataPacket[4] = { dividerByte1, dividerByte2, controlDataByte, bandSwitchByte };
const long tunerStepSize = 50000;        // Tuner step size (Hz), either 50000, 62500, or 31250
const long segmentStartFreq = 36000000;  // tunes from segmentStartFreq + segmentSize down to segmentStartFreq
const long segmentSize = 1000000;        // segment that will be covered before the tuner PLL gets reprogrammed, must be a multiple of tunerStepSize

#endif


//##########################################################################################################################//


#ifdef TV_TUNER_PRESENT
// Function to calculate divider bytes
void setDividerBytes(int PLLFreq) {

  // Calc divider value
  int divider = PLLFreq / tunerStepSize;

  // Program divider bytes
  dividerByte1 = (divider >> 8) & 0x7F;  // Higher 7 bits (N14 to N8)
  dividerByte2 = divider & 0xFF;         // Lower 8 bits (N7 to N0)
}

#endif

//##########################################################################################################################//

void tune() {  // selects tuning method depending on frequency and programs si5351

#ifdef TV_TUNER_PRESENT

  if (FREQ > HIGH_BAND_UPPER_LIMIT)  // upper limit
    FREQ = HIGH_BAND_UPPER_LIMIT;
#endif

#ifndef TV_TUNER_PRESENT
  if (FREQ > HIGHEST_ALLOWED_FREQUENCY)   // upper limit without tuner
    FREQ = HIGHEST_ALLOWED_FREQUENCY;
#endif



    // The below sequences programs the SI5351 directly if the RF is within DIRECTMODE range (tuner not in use)
    if (FREQ <= SHORTWAVE_MODE_UPPER_LIMIT && FREQ >= LOWEST_ALLOWED_FREQUENCY) {
      TVTunerActive = false;


      if (!singleConversionMode ) {
        singleConversionMode  = true;  // single conversion with LO above the IF, update needed to select BFO's 
        loadSi4735parameters();
      }


      OLDPLLFREQ = -1;
      digitalWrite(IF_INPUT_RELAY, HIGH);  // connect bandpass with AD831
      
      
      if(! lowSideInjection)
         LO_RX = abs((SI4735TUNED_FREQ * 1000) + FREQ); // normal mode with LO above RF
      else 
         LO_RX = abs((SI4735TUNED_FREQ * 1000) - FREQ); // debugging mode with LO above RF
      

      si5351.set_freq(LO_RX * 100ULL, SI5351_CLK2);
      return;
    }

#ifdef TV_TUNER_PRESENT
    // If the freq is higher, we need to build the I2C data packet and program the TV tuner
    if (FREQ > SHORTWAVE_MODE_UPPER_LIMIT) {
      TVTunerActive = true;

      if (singleConversionMode ) {
        singleConversionMode  = false;  // double conversion, 2 * sideband inversion  cancel themselves out
        loadSi4735parameters();
      }

      programTVTuner();
    }

#endif
  }

  //##########################################################################################################################//

#ifdef TV_TUNER_PRESENT

  void programTVTuner() {

    static long errorComp = 0;
    digitalWrite(IF_INPUT_RELAY, LOW);                                              // switch input to TV tuner
    PLLFREQ = (FREQ + segmentStartFreq + segmentSize) / segmentSize * segmentSize;  // round down and add 1 segment to get the PLL freq



    if (PLLFREQ != OLDPLLFREQ) {  // reprogram PLL only if the segment changes
      setDividerBytes(PLLFREQ);   // update the divider
      setBandSwitchByte();        // set band byte
      dac1.outputVoltage((uint8_t) initialGain); // relevant if we come from shortwave and dac1 is set to a low voltage    

/*
If fwanted > fcurrent, use telegram as :
Start – ADB – ACK - DB1 – ACK – DB2 – ACK - CB – ACK - BB – ACK – Stop

If fw < fc, use telegram as :
Start – ADB – ACK - CB – ACK – BB – ACK - DB1 – ACK - DB2 – ACK – Stop

*/

      if (PLLFREQ > OLDPLLFREQ) {
        I2CDataPacket[0] = dividerByte1;
        I2CDataPacket[1] = dividerByte2;
        I2CDataPacket[2] = controlDataByte;
        I2CDataPacket[3] = bandSwitchByte;
      }

      if (PLLFREQ < OLDPLLFREQ) {
        I2CDataPacket[0] = controlDataByte;
        I2CDataPacket[1] = bandSwitchByte;
        I2CDataPacket[2] = dividerByte1;
        I2CDataPacket[3] = dividerByte2;
      }


      Wire.endTransmission();  // End any communication with the SI chip(s), otherwise NACK errors
      delay(5);
      Wire.setClock(400000);                // Adjust for tuner
      Wire.beginTransmission(addressByte);  // Start communication
      // Send all bytes in the data packet
      for (int i = 0; i < sizeof(I2CDataPacket); i++) {
        Wire.write(I2CDataPacket[i]);  // Write each byte
      }
      Wire.endTransmission();  // End da transmission
      Wire.setClock(I2C_BUSSPEED);

      errorComp = tunerOffsetPPM * (long)(PLLFREQ / 1000000);  // calculate  ppm compensation
      OLDPLLFREQ = PLLFREQ;
    }

    // now program the SI5351


    if (!lowSideInjection) 
       LO_RX = segmentStartFreq + segmentSize - (FREQ % segmentSize) + ((long)SI4735TUNED_FREQ * 1000);  // expression for LO above RF, to calculate SI5351 frequency
    else 
       LO_RX = segmentStartFreq + segmentSize - (FREQ % segmentSize) - ((long)SI4735TUNED_FREQ * 1000);  // for debugging expression for LO below RF

    si5351.set_freq((LO_RX + errorComp) * 100ULL, SI5351_CLK2);  // add the tuner  offset correction

    //Serial_printf("PLL: %lu KHz,  SI5351: %lu KHz, FREQ: %lu KHz, errorComp: %ld\n", PLLFREQ / 1000, LO_RX / 1000, FREQ / 1000, errorComp);
  }
  //##########################################################################################################################//

  void setBandSwitchByte() {


    /*
 Last 3 bits: P2 P1 P0 Selected band
0 0 1 low-band
0 1 0 mid-band
1 0 0 high-band
*/

  
    // Set the correct band based on frequency
    if (FREQ >= LOW_BAND_LOWER_LIMIT && FREQ <= LOW_BAND_UPPER_LIMIT) {
      //bandSwitchByte = 0b00011001;
      bandSwitchByte = 0b00000001;
    }

    else if (FREQ > LOW_BAND_UPPER_LIMIT && FREQ <= MID_BAND_UPPER_LIMIT) {
      //bandSwitchByte = 0b00011010;
      bandSwitchByte = 0b00000010;
 
    }

    else if (FREQ > MID_BAND_UPPER_LIMIT && FREQ <= HIGH_BAND_UPPER_LIMIT) {
      //bandSwitchByte = 0b00011100;
      bandSwitchByte = 0b00000100;
 
    }
  }


  //##########################################################################################################################//

  void setTunerAGC() {  // use TV tuner AGC for strong signals.


    if (!TVTunerActive)
      return;
    static unsigned char oldRSSI = initialGain;
    static unsigned char newRSSI = 0;
    unsigned char adj =  signalStrength * 2;
    const unsigned char minGain = 70; 

    newRSSI = (255 - adj + 5);   // test formula

  if (newRSSI > initialGain) // max allowed gain
      newRSSI = initialGain;

  if (newRSSI < minGain) // min. allowed gain
      newRSSI = minGain;

     agcVal = newRSSI;                                       


    //Serial_printf("agcVal%d oldRSSSI%d newRSSI%d\n", agcVal, oldRSSI, newRSSI);
    //tft.fillRect(250,300,230,18, TFT_BLACK);
    //tft.setCursor (250, 300);
    //tft.printf("DAC:%d newRSSI%d",agcVal, newRSSI);


    tft.setTextColor(TFT_SKYBLUE);
    tft.setTextSize(1);
    tft.fillRect(143, 140, 16, 24, TFT_BLACK);
    tft.setCursor(143, 140);
    tft.printf("%d", signalStrength);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(143, 155);
    tft.printf("%d",oldRSSI);
    tft.setTextSize(2);
     tft.setTextColor(textColor);

    if (newRSSI > oldRSSI) {
    dac1.outputVoltage((uint8_t)oldRSSI);  // stronger signal, gradual increase during each call
    oldRSSI++;
    }

    else if (newRSSI < oldRSSI) {
    dac1.outputVoltage((uint8_t)oldRSSI);  // weaker signal
    oldRSSI--;
    }


    //DAC level vs tuner attenuation: 255 = 0dB, 159  = - 10dB, 145 = -20dB, 129 = -30dB, 105 = -40dB attenuation. Attenuation kicking in at around 180
  }

  //##########################################################################################################################//

  void setInitialTunerGain() {  // biases the AGC so that the tuner does not run with full gain when no signal
                                // This reduces cross modulation and helps compensate for gain differences if a higher gain LNA is used.
    uint8_t oldinitialGain = 0;

    encLockedtoSynth = false;

    tft.setTextColor(textColor);
    tft.fillRect(10, 60, 328, 64, TFT_BLACK);
    tft.setCursor(10, 63);
    tft.print("Use encoder to set gain");
    tft.setCursor(10, 83);
    tft.print("limit. Press enc. to leave.");

    while (digitalRead(ENCODER_BUTTON) == HIGH) {


      if (oldinitialGain != initialGain) {
        tft.fillRect(10, 104, 323, 20, TFT_BLACK);
        tft.setCursor(10, 104);
        tft.printf("Gain limit: %d   DAC:%d", (initialGain - 100) / 2, agcVal);
        oldinitialGain = initialGain;
      }

      delay(20);
      if (clw)
        initialGain += 5;
      if (cclw)
        initialGain -= 5;

      if (initialGain > 200) // DAC 200 means full gain, reduction kicks in at around 180
        initialGain = 200;

      if (initialGain < 70)
        initialGain = 70;

      agcVal = initialGain;
      dac1.outputVoltage((uint8_t)agcVal);  // set the dac
      clw = false;
      cclw = false;
    }

    while (digitalRead(ENCODER_BUTTON) == LOW)
      ;
    encLockedtoSynth = true;

    preferences.putUChar("agcS", initialGain);  // write selected gain to flash
    Serial_printf ("Tuner initial gain set %d\n", initialGain);
    clearStatusBar();

delay(200);

  }

#endif


  //##########################################################################################################################//
