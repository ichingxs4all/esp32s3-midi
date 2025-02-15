#include <MIDI.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial0, MIDI); // Hardware Serial port 0  ( TX pin GPIO43, RX pin GPIO44 on Xiao ESP32S3)

// the MIDI channel number to send messages
const int channel = 1;
const int velocity = 127; // Max velocity

int const numTouchPins = 9; // number of pins to use as touchpins, sending note values
int touch[numTouchPins];
int touchon[numTouchPins];
int touchpin[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 }; // which digital pins to use as touch pins
int touchpitch[] = { 61,63,66,68,70,73,75,78,80 }; // which midi notes to send from the touch pins
int long touchThreshold[] = { 20000, 21000, 24000, 20000, 23000, 23000, 23000, 23000, 23000 }; //which thresholds value for the touch pins

bool debug = false;

void setup() {
  
pinMode(LED_BUILTIN, OUTPUT);
digitalWrite(LED_BUILTIN, HIGH);

// Initialize MIDI, and listen to all MIDI channels
// This will also call usb_midi's begin()
MIDI.begin(MIDI_CHANNEL_OMNI);

// Attach the handleNoteOn function to the MIDI Library. It will
// be called whenever the Bluefruit receives MIDI Note On messages.
MIDI.setHandleNoteOn(handleNoteOn);

// Do the same for MIDI Note Off messages.
MIDI.setHandleNoteOff(handleNoteOff);

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
  
  MIDI.read(); //Check for incoming MIDI messages
 
}

void handleNoteOn(byte channel, byte pitch, byte velocity) {
  // Log when a note is pressed.
  digitalWrite(LED_BUILTIN,LOW);
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  // Log when a note is released.
  digitalWrite(LED_BUILTIN,HIGH);
}
