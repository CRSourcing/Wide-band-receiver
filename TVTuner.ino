/* Contains tuning logic for the TV tuner frontend. This logic is written for a group of Philips PAL I2C tuners build from 1995-2005 (Gen 3 and 4 MOPLL tuners).
Other Philips PAL tuners will probably work. 
Tuners from other manufacturers will probaly need another data format, but it may be possible to adapt accordingly. Philips NTSC tuners will need to change 
const long tunerIFLowEnd = 38000000l to const long tunerIFLowEnd = 45000000l and adapt the LC circuit on the IF output of the tuner accordingly.

It contains both the logic for the tuner PLL and the SI5351 which is used as local oscillator to mix down to 21.4MHz.


Below SHORTWAVE_MODE_UPPER_LIMIT the AD831 gets connected to the antenna input via the LNA and a lowpass filter and the tuner gets disabled.
The SI4732 is always used as a demodulator at 21.4MHz. The SI5351 is set to RF+IF (highside injection).

If FREQ exceeds SHORTWAVE_MODE_UPPER_LIMIT, the AD831 gets connected to the tuner output. 
The TV tuner gets activated and converts RF to an IF spectrum roughly 8MHz wide, centering at 36MHz.
The AD831 will then use a 1MHz slot in the IF range and mit ix down to 21.4MHz.
Since the tuner's LO is using highside injection, the spectum gets mirrored and 38MHz corresponds to the low end of the RF spectrum and 38MHz to the high end.
Once the SI5351 frequency reaches either end, the tuner will get reprogrammed. 


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
const long tunerStepSize = 50000l;  // Tuner step size (Hz), either 50000, 62500, or 31250
const long segmentSize = 1000000l;  // segment that will be covered before the tuner PLL gets reprogrammed, must be a multiple of tunerStepSize

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

#ifdef TV_TUNER_PRESENT

void programTVTuner() {

  static long errorComp = 0;
  digitalWrite(RELAY_SWITCH_OUTPUT, LOW);                                      // switch input to TV tuner
  PLLFREQ = (FREQ + tunerIFLowEnd + segmentSize) / segmentSize * segmentSize;  // round down and add 1 segment to get the PLL freq



  if (PLLFREQ != OLDPLLFREQ) {               // reprogram PLL only if the segment changes
    setDividerBytes(PLLFREQ);                // update the divider
    setBandSwitchByte();                     // set band byte
                                             // update frequency display
    dac1.outputVoltage((uint8_t)gainLimit);  // relevant if we come from shortwave and dac1 is set to a low voltage

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
    LO_RX = tunerIFLowEnd + segmentSize - (FREQ % segmentSize) + ((long)SI4735TUNED_FREQ * 1000);  // expression for LO above RF, to calculate SI5351 frequency
  else
    LO_RX = tunerIFLowEnd + segmentSize - (FREQ % segmentSize) - ((long)SI4735TUNED_FREQ * 1000);  // for debugging expression for LO below RF


  if (passbandShift)
    LO_RX -= passbandShift;

  if (!displayFineTuneOffset)  // when using fine tune as "clarifier"
    LO_RX -= fineTuneOffset;


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

void setTunerAGC(bool showValues) {  // use TV tuner AGC for strong signals.


  if (!TVTunerActive)
    return;


  static unsigned char currentAGCVal = gainLimit;  // start with max tuner gain
  static unsigned char agcResult;
  static unsigned char lastSignalStrength = 0;
  const unsigned char minGain = 70;

  agcResult = (255 - (signalStrength * 2) + 5);  // empirical formula
  agcResult = constrain(agcResult, minGain, gainLimit);

  if ((showValues) && displayDebugInfo && (agcResult != currentAGCVal || signalStrength != lastSignalStrength)) {
    tft.setTextColor(TFT_SKYBLUE);
    tft.setTextSize(1);
    tft.fillRect(143, 140, 18, 28, TFT_BLACK);
    tft.setCursor(143, 140);
    tft.printf("%d", signalStrength);
    lastSignalStrength = signalStrength;
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(143, 158);
    tft.printf("%d", currentAGCVal);
    tft.setTextSize(2);
    tft.setTextColor(textColor);
  }


  if (agcResult > currentAGCVal)
    dac1.outputVoltage((uint8_t)currentAGCVal++);  // stronger signal, gradual increase

  else if (agcResult < currentAGCVal)
    dac1.outputVoltage((uint8_t)currentAGCVal--);  // weaker signal, gradua decrease

  //DAC level vs tuner attenuation: 255 = 0dB, 159  = - 10dB, 145 = -20dB, 129 = -30dB, 105 = -40dB attenuation. Attenuation kicking in at around 180
}

//##########################################################################################################################//

void setInitialTunerGain() {  // biases the AGC so that the tuner does not run with full gain when no signal
                              // This reduces cross modulation and helps compensate for gain differences if a higher gain LNA is used.
  uint8_t oldgainLimit = 0;

  encLockedtoSynth = false;

  tft.setTextColor(textColor);
  tft.fillRect(10, 60, 328, 64, TFT_BLACK);
  tft.setCursor(10, 63);
  tft.print(F("Use encoder to set gain"));
  tft.setCursor(10, 83);
  tft.print(F("limit. Press enc. to leave."));

  while (digitalRead(ENCODER_BUTTON) == HIGH) {


    if (oldgainLimit != gainLimit) {
      tft.fillRect(10, 104, 323, 20, TFT_BLACK);
      tft.setCursor(10, 104);
      tft.printf("Gain limit: %d   DAC:%d", (gainLimit - 100) / 2, agcVal);
      oldgainLimit = gainLimit;
    }

    delay(20);
    if (clw)
      gainLimit += 5;
    if (cclw)
      gainLimit -= 5;


    gainLimit = constrain(gainLimit, 70, 200);  // DAC 200 means full gain, reduction kicks in at around 180


    agcVal = gainLimit;
    dac1.outputVoltage((uint8_t)agcVal);  // set the dac
    clw = false;
    cclw = false;
  }

  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;
  encLockedtoSynth = true;

  preferences.putUChar("agcS", gainLimit);  // write selected gain to flash
  Serial_printf("Tuner initial gain set %d\n", gainLimit);
  clearStatusBar();
  delay(500);
}

#endif


//##########################################################################################################################//
