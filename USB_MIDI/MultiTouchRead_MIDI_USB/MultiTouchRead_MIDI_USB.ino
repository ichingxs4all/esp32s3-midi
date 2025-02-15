#include <Arduino.h>
#include <Adafruit_TinyUSB.h> //Set in Arduino Tools Menu the 'USB Mode' to "USB-OTG(TinyUSB)"
#include <MIDI.h>

// USB MIDI object
Adafruit_USBD_MIDI usb_midi;

// Create a new instance of the Arduino MIDI Library,
// and attach usb_midi as the transport.
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

// the MIDI channel number to send messages
const int channel = 1;
const int velocity = 127; // Max velocity

int const numTouchPins = 9; // number of pins to use as touchpins, sending note values
int touch[numTouchPins];
int touchon[numTouchPins];
int touchpin[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 }; // which digital pins to use as touch pins
int touchpitch[] = { 61,63,66,68,70,73,75,78,79 }; // which midi notes to send from the touch pins
int long touchThreshold[] = { 20000, 21000, 24000, 20000, 23000, 23000, 23000, 23000, 23000 }; //which thresholds value for the touch pins

bool debug = false;

void setup() {

  usb_midi.setStringDescriptor("TinyUSB MIDI");

   // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }
  
  if(debug) Serial.begin(115200); //Setup serial port

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  

  // Initialize MIDI, and listen to all MIDI channels
  // This will also call usb_midi's begin()
  MIDI.begin(MIDI_CHANNEL_OMNI);

   // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }
}

void loop() { 
 //  touchpads
  for (int i = 0; i < numTouchPins; i++) {
    touch[i] = touchRead(touchpin[i]);  

    if(debug){ //Write touchpin reading to serial port
    Serial.print(i);
    Serial.print(": ");
    Serial.print(touch[i]);
    Serial.print(" , ");
    }

    if (touch[i] > touchThreshold[i] && touchon[i] == 0) {
      
      
      MIDI.sendNoteOn(touchpitch[i], velocity, channel);
      
      touchon[i] = 1;
    }
    if (touch[i] < touchThreshold[i] && touchon[i] == 1) {
      MIDI.sendNoteOff(touchpitch[i], velocity, channel);   
      touchon[i] = 0;
    }
  }
  if(debug) Serial.println();

  // i think if you remove these last two lines everything breaks and things are sad and people cry
  MIDI.read(); // read and discard any incoming MIDI messages
  delay(10);
}

