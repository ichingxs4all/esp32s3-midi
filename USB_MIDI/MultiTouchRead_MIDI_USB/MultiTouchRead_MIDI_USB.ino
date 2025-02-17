// Example for reading the touchpins of a Xiao ESP32S3 module and sending it out as MIDI notes through USB
// Adapted for use with the 'MIDI Madness Maker' from Michelle Vossen & Veerle Pennock https://v0ss3n.github.io/midimadness/
// All code below is inspired and constructed from examples of the libraries that are used.
//
// Make shure you include ci.json file in your new sketch ..like in this one. Otherwise the touch function does not work.
// And don't forget to set in Arduino Tools Menu both the 'Upload Mode' and the 'USB Mode:' to "USB-OTG(TinyUSB)" otherwise you pull your hair off why it is not working
//
// In this example the builtin led on the Xiao ESP32S3 will blink 4 times when it starts..this signals the callibrating fase at start.
// While calibrating do not touch the touch pins otherwise the calibrating is not usefull anymore
//
// Michel Gutlich mragutlich@gmail.com. https://github.com/ichingxs4all/esp32s3-midi
//

//#include "driver/touch_sensor.h"

#include <Arduino.h>           //Standard Arduino include library
#include <Adafruit_TinyUSB.h>  //Set in Arduino Tools Menu the 'USB Mode' to "USB-OTG(TinyUSB)"
#include <MIDI.h>              //The FortySevenEffects MIDI library

// The MegunoLink library for the exponential filter http://www.MegunoLink.com/documentation/arduino-libraries/exponential-filter/
#include "MegunoLink.h"
#include "Filter.h"            

// USB MIDI object/Users/mragutli/Documents/GitHub/midimadness/Arduino (code)/MIDI_madness_tone_bleeps_with-variable-capacitance/pitches.h
Adafruit_USBD_MIDI usb_midi;

// Create a new instance of the Arduino MIDI Library,
// and attach usb_midi as the transport.
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

// the MIDI channel number to send messages
uint8_t const channel = 1;      // The Midi channel we are sending to

uint8_t const numTouchPins = 8; // Number of pins to use as touchpins

uint32_t touch[numTouchPins];   // In here we store the actual touch pin reading

int touchLastCC[numTouchPins];  // In here we store the latest touch Control Change reading to compare with

uint8_t touchon[numTouchPins];  // Trigger flags are stored to remember which note is already playing

uint32_t touchBaseLine[numTouchPins];  // In here we store the calibrated baseline levels of the touch pins

uint8_t touchpin[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };             // Which digital pins to use as touch pins.
uint8_t touchpitch[] = { 61, 63, 66, 68, 70, 73, 75, 78, 79 };  // Which midi notes to send from the touch pins.
uint8_t touchCC[] = { 10, 11, 12, 13, 14, 15, 16, 17, 18 };     // Which Control Change number is connected to the touch pin.

uint16_t touchTreshold[] = { 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000 };      // Which tresholds value for the touch pins.
uint32_t touchMax[] = { 500000, 500000, 500000, 500000, 500000, 500000, 500000, 500000, 500000 };  // The maximum touch pin reading to be able to calculate the velocity or control range.

// Create array of new exponential filters with a weight of 10 and initial value of 0. A high weight is less filtering (for example 90 ), a low weight is more filtering ( for example 5).
ExponentialFilter<long> TouchFilter1(10, 0);
ExponentialFilter<long> TouchFilter2(10, 0);
ExponentialFilter<long> TouchFilter3(10, 0);
ExponentialFilter<long> TouchFilter4(10, 0);
ExponentialFilter<long> TouchFilter5(10, 0);
ExponentialFilter<long> TouchFilter6(10, 0);
ExponentialFilter<long> TouchFilter7(10, 0);
ExponentialFilter<long> TouchFilter8(10, 0);
ExponentialFilter<long> TouchFilter9(10, 0);


bool debug = false;           // Set this to TRUE to enable some debugging info on the serial console

bool enableNote = true;       //Set this to FALSE if you want to disable Note messages
bool enableCC = true;         // Set this to FALSE if you want to disable Control Change Messages
bool enableVelocity = true;  // Set this to FALSE if you need a FIXED velocity on the played notes

uint8_t velocity = 100;          // Max velocity for the fixed value

bool toggle = false;

void setup() {
  //If you connect pin D2 / GPIO3 ( T2 on the MIDI Madness Maker) to 3V3 on the board while starting you can enable debug mode and some debug info is send to the serial console
  if(digitalRead(D2) == HIGH ) debug = true;  

  //usb_midi.setStringDescriptor("TinyUSB MIDI");

  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  if (debug) {
    delay(1000);           //Wait for the serial port to come up
    Serial.begin(115200);  //Setup serial port for debugging
  }

  pinMode(LED_BUILTIN, OUTPUT);     //Define the builtin led port as output
  digitalWrite(LED_BUILTIN, HIGH);  //Turn it off...yes...the led is wired such a way that is ON when set LOW and viceversa

  // Initialize MIDI, and listen to all MIDI channels
  // This will also call usb_midi's begin()
  MIDI.begin(MIDI_CHANNEL_OMNI);

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  delay(1000);  //Wait a second for the USB stack to come up

  calibrateTouchPins();  //Do the calibartion of the touch pins ....do not touch the pins while calibrating !
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH); //Turn of led
  
  //  Reading all the defined touchpins
  for (int i = 0; i < numTouchPins; i++) {

    touch[i] = touchRead(touchpin[i]);


    if(debug) debugTouchPinReading(i);


    if (enableCC) { // If enbled send Control Change Messages
    
    filterTouch( i, touch[i]); // Filter the touch value

    if(touchLastCC[i] > 0 ){ //If the mapped CC value is greater then 0 send it
    MIDI.sendControlChange(touchCC[i], touchLastCC[i], channel); //Send the message
    toggle = !toggle;  //A little toggle to let the led flash while sending messages
    digitalWrite(LED_BUILTIN, toggle);
    }
    }

    if (enableVelocity) {
      velocity = map(touch[i], (touchBaseLine[i] + touchTreshold[i]), touchMax[i], 0, 127);  //Map the touched pin reading above treshold to the velocity range of 0 - 127
    }


    if(enableNote){ //Send notes if enabled

    if (touch[i] > (touchBaseLine[i] + touchTreshold[i]) && touchon[i] == 0) {  //If the last read value is above treshold play the according  midi note if not already triggered
      MIDI.sendNoteOn(touchpitch[i], velocity, channel);                        //
      digitalWrite(LED_BUILTIN, LOW);
      touchon[i] = 1;
    }
    if (touch[i] < (touchBaseLine[i] + touchTreshold[i]) && touchon[i] == 1) {  //If the last read value is below treshold stop playing the according  midi note if not already triggered
      MIDI.sendNoteOff(touchpitch[i], 0, channel);
      digitalWrite(LED_BUILTIN, HIGH);
      touchon[i] = 0;
    }
    }
  }

  if (debug) Serial.println();  //Just a newline after all readings

  // i think if you remove these last two lines everything breaks and things are sad and people cry
  MIDI.read();  // read and discard any incoming MIDI messages
  delay(10);
}

void calibrateTouchPins() {
  if (debug) Serial.println("Starting calibration");

  digitalWrite(LED_BUILTIN, toggle);

  for (int i = 0; i < numTouchPins; i++) {
    if (debug) {
      Serial.print("Calibrate pin : ");
      Serial.println(i);
    }
    for (int j = 0; j < 100; j++) {
      touch[i] = touchRead(touchpin[i]);
      touchBaseLine[i] = touchBaseLine[i] + touch[i];
      delay(5);
    }

    toggle = !toggle;  //A little toggle to let the led flash while calibrating
    digitalWrite(LED_BUILTIN, toggle);

    touchBaseLine[i] = touchBaseLine[i] / 100;  //Calculate the average of 100 samples

    if (debug) {
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(touchBaseLine[i]);
      Serial.print(" , ");
    }
  }

  digitalWrite(LED_BUILTIN, HIGH);  //Turn led off to signal calibrating is ready

  if (debug) {
    Serial.println();
    Serial.println("Calibrated");
    delay(1000);
  }
}


void filterTouch(int touchPin, int long touchValue){
  
  switch (touchPin) {

    case 0:
    TouchFilter1.Filter(touchValue);
    touchLastCC[touchPin]=  map(TouchFilter1.Current(), (touchBaseLine[touchPin] + touchTreshold[touchPin]), touchMax[touchPin], 0, 127); //Map the value into midi range
    break;

    case 1:
    TouchFilter2.Filter(touchValue);
    touchLastCC[touchPin] = map(TouchFilter2.Current(), (touchBaseLine[touchPin] + touchTreshold[touchPin]), touchMax[touchPin], 0, 127); //Map the value into midi range
    break;

    case 2:
    TouchFilter3.Filter(touchValue);
    touchLastCC[touchPin]= map(TouchFilter3.Current(), (touchBaseLine[touchPin] + touchTreshold[touchPin]), touchMax[touchPin], 0, 127); //Map the value into midi range
    break;

    case 3:
    TouchFilter4.Filter(touchValue);
    touchLastCC[touchPin]= map(TouchFilter4.Current(), (touchBaseLine[touchPin] + touchTreshold[touchPin]), touchMax[touchPin], 0, 127); //Map the value into midi range
    break;

    case 4:
    TouchFilter5.Filter(touchValue);
    touchLastCC[touchPin]= map(TouchFilter5.Current(), (touchBaseLine[touchPin] + touchTreshold[touchPin]), touchMax[touchPin], 0, 127); //Map the value into midi range
    break;

    case 5:
    TouchFilter6.Filter(touchValue);
    touchLastCC[touchPin]= map(TouchFilter6.Current(), (touchBaseLine[touchPin] + touchTreshold[touchPin]), touchMax[touchPin], 0, 127); //Map the value into midi range
    break;

    case 6:
    TouchFilter7.Filter(touchValue);
    touchLastCC[touchPin]= map(TouchFilter7.Current(), (touchBaseLine[touchPin] + touchTreshold[touchPin]), touchMax[touchPin], 0, 127); //Map the value into midi range
    break;

    case 7:
    TouchFilter8.Filter(touchValue);
    touchLastCC[touchPin]= map(TouchFilter8.Current(), (touchBaseLine[touchPin] + touchTreshold[touchPin]), touchMax[touchPin], 0, 127); //Map the value into midi range
    break;

  }
 }

 void debugTouchPinReading(int i){

   if (debug) {  //Write all touch pin readings to the erial port
      Serial.print(i);
      Serial.print(": ");
      Serial.print(touch[i]);
      Serial.print(" , ");
    }
 }