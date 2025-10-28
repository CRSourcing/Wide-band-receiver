#define RING_BUFFER_SIZE 512  // Log Serial to ringBuffer
#pragma once


class RingBuffer {
private:
  char buffer[RING_BUFFER_SIZE];
  size_t head = 0;  // Write position
  size_t tail = 0;  // Read position
  bool full = false;

public:
  void write(char c) {
    buffer[head] = c;
    head = (head + 1) % RING_BUFFER_SIZE;
    if (full) 
      tail = (tail + 1) % RING_BUFFER_SIZE;  // Overwrite oldest data if full
    full = (head == tail);
  }

  String read() {
    String result;
    while (tail != head || full) {
      result += buffer[tail];
      tail = (tail + 1) % RING_BUFFER_SIZE;
      full = false;
    }
    return result;
  }

  void clear() {
    head = tail;
    full = false;
  }

  bool isEmpty() const { return (!full && (head == tail)); }
};

extern RingBuffer logBuffer; 


// Override Serial.print()
size_t Serial_print(const char* str) {
  size_t len = strlen(str);
  for (size_t i = 0; i < len; i++) {
    logBuffer.write(str[i]);
  }
  return Serial.print(str);
}

// Override Serial.println()
size_t Serial_println(const char* str) {
  size_t len = strlen(str);
  for (size_t i = 0; i < len; i++) {
    logBuffer.write(str[i]);
  }
  logBuffer.write('\n');  // Add newline to buffer
  return Serial.println(str);
}

// Override Serial.printf()
int Serial_printf(const char* format, ...) {
  char buf[256]; 
  va_list args;
  va_start(args, format);
  int len = vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);

  if (len > 0) {
    for (int i = 0; i < len; i++) {
      logBuffer.write(buf[i]);
    }
    Serial.printf("%s", buf);  //  Serial
  }
  return len;
}






