The external power of the MIDI Madness Maker can easily be adjusted to accept voltages above 5V by adding a voltage regulator of the 78L05 type.
Please be aware that the pinout of a 78L05 is NOT the same as its bigger brother 7805.

The patch can be applied by removing the diode and clearing the mounting holes with a desoldering pump.
Also clear the mounting hole of the clamp hole of the potentiometer.
Double check the polarity of the regulator and put the pin in the middle of the regulator through the mounting hole of the potentiometer ( see pictures).
Solder this middle pin also to outside pin of the potentiometer ( closest to regulator).
Then solder the other pins of the regulator. Double check with the pictures if the orientation is still correct.

First try out without the Xiao ESP32S3 mounted. Test with a Multimeter if the voltage between the upper right pin (+5V ) and the pin below ( GND) is reading 5V. If not...check the switch..( the led should be lighting up ).
If you have triple checked everything you can mount the Xiao with the power switched off and then turn it back on.

