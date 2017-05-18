#include <SoftwareSerial.h>

// https://github.com/zapta/linbus/tree/master/analyzer/arduino
#include "lin_frame.h"

// Pins we use for MCP2004
#define RX_PIN 10
#define TX_PIN 11
#define FAULT_PIN 14
#define CS_PIN 8

#define SYN_FIELD 0x55
#define SWM_ID 0x20

SoftwareSerial LINBusSerial(RX_PIN, TX_PIN);

//Lbus = LIN BUS from Car
//Vss = Ground
//Vbb = +12V

// MCP2004 LIN bus frame:
// ZERO_BYTE SYN_BYTE ID_BYTE DATA_BYTES.. CHECKSUM_BYTE

// Volvo V50 2007 SWM key codes

// BTN_NEXT       20 0 10 0 0 EF
// BTN_PREV       20 0 2 0 0 FD
// BTN_VOL_UP     20 0 0 1 0 FE
// BTN_VOL_DOWN   20 0 80 0 0 7F
// BTN_ENTER      20 0 1 0 0 FE
// BTN_BACK       20 0 8 0 0 F7
// BTN_UP         20 1 0 0 0 FE
// BTN_DOWN       20 2 0 0 0 FD
// BTN_LEFT       20 4 0 0 0 FB
// BTN_RIGHT      20 8 0 0 0 F7

// IGN_KEY_ON     50 E 0 F1


byte b, i, n;
LinFrame frame;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  // Open serial communications to host (PC) and wait for port to open:
  Serial.begin(9600, SERIAL_8E1);
  Serial.println("LIN Debugging begins");

  LINBusSerial.begin(9600);

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  pinMode(FAULT_PIN, OUTPUT);
  digitalWrite(FAULT_PIN, HIGH);

  frame = LinFrame();
}

void loop() {
  if (LINBusSerial.available()) {
    b = LINBusSerial.read();
    n = frame.num_bytes();

    if (b == SYN_FIELD && n > 2 && frame.get_byte(n - 1) == 0) {
      digitalWrite(LED_BUILTIN, HIGH);
      frame.pop_byte();
      handle_frame();
      frame.reset();
      digitalWrite(LED_BUILTIN, LOW);
    } else if (n == LinFrame::kMaxBytes) {
      frame.reset();
    } else {
      frame.append_byte(b);
    }
  }
}

void handle_frame() {
  if (frame.get_byte(0) != SWM_ID)
    return;

  // skip zero values 20 0 0 0 0 FF
  if (frame.get_byte(5) == 0xFF)
    return;

  if (!frame.isValid())
    return;


  dump_frame();
}

void dump_frame() {
  for (i = 0; i < frame.num_bytes(); i++) {
    Serial.print(frame.get_byte(i), HEX);
    Serial.print(" ");
  }
  Serial.println();
}


