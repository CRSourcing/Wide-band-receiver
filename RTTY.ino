#ifdef AUDIO_SQUAREWAVE_PRESENT

// Basic RTTY decoder, 45.45 Baud only
//##########################################################################################################################//

#define MARK 2125
#define SPACE 2295
int bitTiming = 22000;
bool mark = false;
bool space = false;
int range = 85;  // aceptable frequency deviation


//##########################################################################################################################//

void RTTYINIT() {

  attachInterrupt(digitalPinToInterrupt(PULSE_PIN), handlePulse, RISING);  // Attach interrupt for frequency measurement
  frq = 0;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tuneRTTYFreq();
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 30);
  RTTYDecode();
  detachInterrupt(digitalPinToInterrupt(PULSE_PIN));
}

//##########################################################################################################################//


void tuneRTTYFreq() {  // helper function to fine tune the the receiving frequency


  int lineX = 0;

  int prevX = 0;    // Prev X position
  int prevY = 320;  // Prev Y position
  uint16_t color;

  tft.setCursor(0, 65);
  tft.setTextColor(TFT_GREEN);
  tft.print("MARK = 2125 Hz");
  tft.setTextColor(TFT_RED);
  tft.print(" SPACE = 2295 Hz");

  tft.setTextSize(1);

  tft.setCursor(330, 140);
  tft.print("Space");
  tft.drawFastHLine(0, 140, 320, TFT_RED);

  tft.setTextColor(TFT_GREEN);
  tft.setCursor(330, 240);
  tft.print("Mark");
  tft.drawFastHLine(0, 240, 320, TFT_GREEN);


  tft.setTextSize(2);
  tft.setCursor(10, 280);

  tft.print("Match Mark and Space w. lines");
  tft.setCursor(10, 300);
  tft.print("Press encoder to continue");
  displayFREQ(FREQ);

  while (true) {


    getState(1000);


    int currentY = 1350 - frq / 1.7 + 140;  // calculate y pos based on freq
    currentY = constrain(currentY, 120, 260);


    if (frq < 1550)
      currentY = 265;

    if (frq > 2450)
      currentY = 175;

    // color based on freq
    color = TFT_GREY;

    // Mark (1) at 2125 Hz
    if (mark)
      color = TFT_GREEN;

    // Space (0) at 2295 Hz
    if (space)
      color = TFT_RED;


    if (lineX)
      tft.drawLine(prevX, prevY, lineX, currentY, color);

    // Update previous pos
    prevX = lineX;
    prevY = currentY;
    // Increment X position
    lineX += 2;



    if (lineX == 320) {
      lineX = 0;
      prevX = 0;    // Reset
      prevY = 320;  // Reset
      tft.fillRect(0, 115, 325, 160, TFT_BLACK);
      tft.drawFastHLine(0, 240, 320, TFT_GREEN);
      tft.drawFastHLine(0, 140, 320, TFT_RED);
    }

    if (digitalRead(ENCODER_BUTTON) == LOW) {
      while (digitalRead(ENCODER_BUTTON) == LOW)
        ;
      return;
    }


    if (clw) {  // encoder clockwise



      FREQ += 25;  // Fine tune 25Hz with encoder
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


//##########################################################################################################################//

void getState(long Duration) {  // calculates freq by measuring time btw. rising edges.
                                //45.45 Baud needs at least 706 microseconds to capture 2 rising edges,better use 1000us to remove glitches
                                // oversamples, the longer the duration, the better the result.
  uint16_t sampler = 0;
  uint16_t ctr = 0;
  frq = 0;
  static bool set = false;
  newPulse = false;
  mark = space = false;
  long startT = micros();
  long freq = 0;
  while (micros() < startT + Duration) {

    while (!newPulse)  // get  lastPulseTime
      ;

    lastPulseTime = micros();
    newPulse = false;

    while (!newPulse)  // wait for new rising pulse
      ;
    volatile long currentTime = micros();  // Get timestamp
    newPulse = false;
    pulseInterval = currentTime - lastPulseTime;  // Calculate time elapsed
    lastPulseTime = currentTime;

    if (pulseInterval > 0) {
      freq = 1000000 / pulseInterval;  // calc. frequency in Hz (f = 1 / T)
    }

    if (freq > 2000 && freq < 2500) {  // oversample
      sampler += freq;
      ctr++;
    }
  }

  if (ctr)
    frq = sampler / ctr;


  if (frq >= (SPACE - range) && frq <= (SPACE + range)) {
    space = true;
    mark = false;
    if (set) {
      tft.fillCircle(350, 10, 5, TFT_BLACK);
      set = false;
    }
  }

  else if (frq >= (MARK - range) && frq <= (MARK + range)) {
    mark = true;
    space = false;

    if (set) {
      tft.fillCircle(350, 10, 5, TFT_BLACK);
      set = false;
    }
  }


  if (mark == false && space == false) {


    if (!set) {
      tft.fillCircle(350, 10, 5, TFT_YELLOW);
      set = true;
    }
  }
}
//##########################################################################################################################//



void RTTYDecode() {

  tft.setTextColor(TFT_GREEN);
  initBaudotLookup();
  waitForStopBitEnd();

  while (true) {

    bitTiming = encoderModifyDuration(bitTiming);

    if (digitalRead(ENCODER_BUTTON) == LOW)
      return;
    waitForStartBitEnd();
    decodeRTTYCharacter();
    waitForStopBitEnd();
  }
}


//##########################################################################################################################/

// Define Baudot Code Structure
typedef struct {
  uint8_t baudot;  // 5-bit Baudot code
  char letters;    // ASCII character in Letters shift
  char figures;    // ASCII character in Figures shift
} BaudotCode;

// Baudot Code Lookup Table based on frequency of letters in English
const BaudotCode baudot_table[32] = {
  { 0b00001, 'E', '3' }, 
  { 0b00011, 'A', '-' }, 
  { 0b00110, 'I', '8' }, 
  { 0b10000, 'T', '5' }, 
  { 0b00101, 'S', '\'' }, 
  { 0b01010, 'R', '4' }, 
  { 0b01100, 'N', ',' }, 
  { 0b00111, 'U', '7' }, 
  { 0b01001, 'D', '$' }, 
  { 0b10100, 'H', '#' }, 
  { 0b11000, 'O', '9' }, 
  { 0b01101, 'F', '!' }, 
  { 0b10010, 'L', ')' }, 
  { 0b10101, 'Y', '6' }, 
  { 0b11010, 'G', '&' }, 
  { 0b11100, 'M', '.' }, 
  { 0b10011, 'W', '2' }, 
  { 0b01011, 'J', '\a' }, 
  { 0b10110, 'P', '0' }, 
  { 0b11101, 'X', '/' }, 
  { 0b11001, 'B', '?' }, 
  { 0b01110, 'C', ':' }, 
  { 0b10111, 'Q', '1' }, 
  { 0b01111, 'K', '(' }, 
  { 0b10001, 'Z', '+' }, 
  { 0b11110, 'V', '=' }, 
  { 0b11011, ' ', ' ' },  // FIGS
  { 0b11111, ' ', ' ' },
  { 0b00010, '\n', '\n' },
  { 0b01000, '\r', '\r' },
  { 0b00100, ' ', ' ' },
  { 0b00000, '\0', '\0' }  // NULL
};

char ltrs_lookup[32] = { 0 };  // Letters mode
char figs_lookup[32] = { 0 };  // Figures mode


// default: Letters mode
bool isFigures = false;

// Initialize lookup tables once at startup
void initBaudotLookup() {
  for (int i = 0; i < 32; i++) {
    uint8_t code = baudot_table[i].baudot & 0x1F;
    ltrs_lookup[code] = baudot_table[i].letters;
    figs_lookup[code] = baudot_table[i].figures;
  }
}


//##########################################################################################################################/
char baudotToASCII(uint8_t baudot) {
  baudot &= 0x1F;

  if (baudot == 0b11011) {  // FIGS
    isFigures = true;
    return '\0';
  } else if (baudot == 0b11111) {  // LTRS
    isFigures = false;
    return '\0';
  }

  return isFigures ? figs_lookup[baudot] : ltrs_lookup[baudot];
}

//##########################################################################################################################/



void decodeRTTYCharacter() {
  uint8_t baudotChar = 0;
  long st;
  int xPos, yPos;
  for (int i = 0; i < 5; i++) {
    st = micros();

    delay(3);         // ignore transition spikes
    getState(17000);  // sample center

    if (mark) {
      baudotChar |= (1 << i);  // Set bit (LSB first!!!!)
    }

    while ((micros() - st) < bitTiming)  // fill bitTiming
      delayMicroseconds(10);
  }

  //##########################################################################################################################/


  // Convert Baudeot to ASCII
  char decodedChar = baudotToASCII(baudotChar);

  if (isprint(decodedChar)) {
    Serial_printf("%c", decodedChar);
    tft.print(decodedChar);
  }

  xPos = tft.getCursorX();
  yPos = tft.getCursorY();

  if (xPos > 460)
    tft.print("\n\n");

  if (yPos > 300) {  // reset screen
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 30);
  }
}

//##########################################################################################################################/

int encoderModifyDuration(int value) {  // Helper function to change bitTiming
  if (clw || cclw) {
    int xPos = tft.getCursorX();
    int yPos = tft.getCursorY();

    value += (clw ? 100 : -100);  // Increment or decrement based on clw or cclw
    clw = false;
    cclw = false;  // Reset both flags

    tft.fillRect(400, 10, 70, 16, TFT_BLACK);
    tft.setCursor(400, 10);
    tft.printf("%d", value);
    tft.setCursor(xPos, yPos);
  }
  return value;
}
//##########################################################################################################################/

/*

Idle State (Mark): Continuous 2125 Hz signal.

Start Bit (Space): Transition to 2295 Hz signal.

Data Bits: Sequence of 5 bits, each either mark (2125 Hz) or space (2295 Hz), depending on the character.

Stop Bits (Mark): Return to 2125 Hz signal, typically for 1.5 bits duration.

*/

void waitForStopBitEnd() {
  long timeout = micros() + bitTiming * 1.5;

  do {
    getState(5000);
  }

  while (mark && (micros() < timeout));
}
//##########################################################################################################################/


void waitForStartBitEnd() {

  getState(2000);
  if (space) {
    long timeout = micros() + bitTiming - 2000;
    while (space && micros() < timeout) {
      getState(2000);
    }
  }
}


//##########################################################################################################################/

#endif