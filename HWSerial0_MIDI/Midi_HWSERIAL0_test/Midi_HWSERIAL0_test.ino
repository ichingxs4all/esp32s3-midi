// Simple example for using the hardware serial port for MIDI

#include <MIDI.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial0, MIDI); // Hardware Serial port 0  ( TX pin GPIO43, RX pin GPIO44 on Xiao ESP32S3)

// Variable that holds the current position in the sequence.
uint32_t position = 0;

// Store example melody as an array of note values
byte note_sequence[] = {
    74, 78, 81, 86, 90, 93, 98, 102, 57, 61, 66, 69, 73, 78, 81, 85, 88, 92, 97, 100, 97, 92, 88, 85, 81, 78,
    74, 69, 66, 62, 57, 62, 66, 69, 74, 78, 81, 86, 90, 93, 97, 102, 97, 93, 90, 85, 81, 78, 73, 68, 64, 61,
    56, 61, 64, 68, 74, 78, 81, 86, 90, 93, 98, 102
};


void setup()
{
Serial.begin(115200); // Start the USB serial port to monitor incomming midi messages on the HARDWARE serial port

pinMode(LED_BUILTIN, OUTPUT);   //On Xiao ESP32S3 the builtin led is on GPIO21
digitalWrite(LED_BUILTIN,HIGH); //and active LOW

MIDI.begin(MIDI_CHANNEL_OMNI);  // Launch MIDI and listen to all channels 
  
// Attach the handleNoteOn function to the MIDI Library. It will
// be called whenever the Bluefruit receives MIDI Note On messages.
MIDI.setHandleNoteOn(handleNoteOn);

// Do the same for MIDI Note Off messages.
MIDI.setHandleNoteOff(handleNoteOff);
}

void loop()
{
  static uint32_t start_ms = 0;
  if (millis() - start_ms > 266) {
    start_ms += 266;

    // Setup variables for the current and previous
    // positions in the note sequence.
    int previous = position - 1;

    // If we currently are at position 0, set the
    // previous position to the last note in the sequence.
    if (previous < 0) {
      previous = sizeof(note_sequence) - 1;
    }

    // Send Note On for current position at full velocity (127) on channel 1.
    MIDI.sendNoteOn(note_sequence[position], 127, 1);

    // Send Note Off for previous note.
    MIDI.sendNoteOff(note_sequence[previous], 0, 1);

    // Increment position
    position++;

    // If we are at the end of the sequence, start over.
    if (position >= sizeof(note_sequence)) {
      position = 0;
    }
  }
   MIDI.read();
}


void handleNoteOn(byte channel, byte pitch, byte velocity) {
  // Log when a note is pressed.
  digitalWrite(LED_BUILTIN,LOW);

  Serial.print("Note on: channel = ");
  Serial.print(channel);

  Serial.print(" pitch = ");
  Serial.print(pitch);

  Serial.print(" velocity = ");
  Serial.println(velocity);
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  // Log when a note is released.
  digitalWrite(LED_BUILTIN,HIGH);

  Serial.print("Note off: channel = ");
  Serial.print(channel);

  Serial.print(" pitch = ");
  Serial.print(pitch);

  Serial.print(" velocity = ");
  Serial.println(velocity);
}
