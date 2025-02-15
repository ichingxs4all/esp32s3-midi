#include <BLEMIDI_Transport.h>

#include <hardware/BLEMIDI_ESP32_NimBLE.h> //Take version 1.4.3 !!


BLEMIDI_CREATE_INSTANCE("Michel-BLE-MIDI",MIDI)

bool isConnected = false;

// ESP32 Touch Test
// Just test touch pin - Touch0 is T0 which is on GPIO 4.
int val;
bool triggered = 0;
int tresHold = 20000;

void setup() {
  Serial.begin(115200);
  delay(1000);  // give me time to bring up serial monitor
  
  Serial.println("ESP32 Touch Test");
  MIDI.begin();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  BLEMIDI.setHandleConnected([]() {
    isConnected = true;
    digitalWrite(LED_BUILTIN, LOW);
  });

  BLEMIDI.setHandleDisconnected([]() {
    isConnected = false;
    digitalWrite(LED_BUILTIN, HIGH);
  });

  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    digitalWrite(LED_BUILTIN, LOW);
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    digitalWrite(LED_BUILTIN, HIGH);
  });
}

void loop() {
  
  MIDI.read();

  val = touchRead(T1);
  if(val > tresHold && triggered == 0) {
    MIDI.sendNoteOn (60, 100, 1);
    Serial.println("Note on");
    triggered = 1;
  }

  if(val < tresHold && triggered == 1) {
    MIDI.sendNoteOff (60, 0, 1);
    Serial.println("Note off");
    triggered = 0;
  }
  delay(10);
}
