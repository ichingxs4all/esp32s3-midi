
#include <BLEMIDI_Transport.h>

#include <hardware/BLEMIDI_ESP32_NimBLE.h> //Take version 1.4.3 !!
//#include <hardware/BLEMIDI_ESP32.h>
//#include <hardware/BLEMIDI_nRF52.h>
//#include <hardware/BLEMIDI_ArduinoBLE.h>

BLEMIDI_CREATE_INSTANCE("Michel-BLE-MIDI",MIDI)

bool isConnected = false;

// ESP32 Touch Test
// Just test touch pin - Touch0 is T0 which is on GPIO 4.
int val;
bool triggered = 0;
int tresHold = 20000;

// the MIDI channel number to send messages
const int channel = 1;
const int velocity = 127; // Max velocity

int const numTouchPins = 9; // number of pins to use as touchpins, sending note values
int touch[numTouchPins];
int touchon[numTouchPins];
int touchpin[] = { 1, 2, 3, 4, 5 ,6, 7, 8,9 }; // which digital pins to use as touch pins
int touchpitch[] = { 61,63,66,68,70,73,75,78,80 }; // which midi notes to send from the touch pins
int long touchThreshold[] = { 20000, 21000, 24000, 20000, 23000, 23000, 23000, 23000, 23000 }; //which thresholds value for the touch pins

bool debug = false;

void setup() {

  if(debug) Serial.begin(115200);
  delay(1000);  // give me time to bring up serial monitor
  
  if(debug) Serial.println("ESP32 Touch Test");
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
  
 //  touchpads
  for (int i = 0; i < numTouchPins; i++) {
    touch[i] = touchRead(touchpin[i]);  
    if (touch[i] > touchThreshold[i] && touchon[i] == 0) {
      
      MIDI.sendNoteOn(touchpitch[i], velocity, channel);
      
      touchon[i] = 1;
    }
    if (touch[i] < touchThreshold[i] && touchon[i] == 1) {
      MIDI.sendNoteOff(touchpitch[i], velocity, channel);   
      touchon[i] = 0;
    }
  }

  
  MIDI.read(); 
  delay(10);
}
