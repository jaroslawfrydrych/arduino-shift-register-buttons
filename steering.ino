#include "buttontype.enum.h"
#include "relay.h"
#include "button.h"

/*
   SN74HC165N_shift_reg

   Program to shift in the bit values from a SN74HC165N 8-bit
   parallel-in/serial-out shift register.

   This sketch demonstrates reading in 16 digital states from a
   pair of daisy-chained SN74HC165N shift registers while using
   only 4 digital pins on the Arduino.

   You can daisy-chain these chips by connecting the serial-out
   (Q7 pin) on one shift register to the serial-in (Ds pin) of
   the other.

   Of course you can daisy chain as many as you like while still
   using only 4 Arduino pins (though you would have to process
   them 4 at a time into separate unsigned long variables).

*/

/* How many shift register chips are daisy-chained.
*/
#define NUMBER_OF_SHIFT_CHIPS   1

/* Width of data (how many ext lines).
*/
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8

/* Width of pulse to trigger the shift register to read and latch.
*/
#define PULSE_WIDTH_USEC   5

/* Optional delay between shift register reads.
*/
#define POLL_DELAY_MSEC   1

/* You will need to change the "int" to "long" If the
   NUMBER_OF_SHIFT_CHIPS is higher than 2.
*/
#define BYTES_VAL_T unsigned int

/*
   Define used buttons count.
*/
#define BUTTONS_COUNT 4

/*
   Define delay for on / off buttons
*/
#define ON_OFF_BUTTON_DELAY 200

/*
   Define delay for temporatry buttons.
*/
#define TEMPORARY_BUTTON_DELAY 100

unsigned long currentMillis = millis();
bool relayState[BUTTONS_COUNT]   = {false, false, false, false};
int relayPin[BUTTONS_COUNT]      = {A0, A1, A2, A3};
/*
   Type 0 means it will be temporary - active when pressed. 1 Means it will be toogled.
*/
int relayType[BUTTONS_COUNT]     = {0, 1, 0, 0};
unsigned long relayLastAction[BUTTONS_COUNT] = {currentMillis, currentMillis, currentMillis, currentMillis};


int clockPin        = 8; // Connect Pin 12 to CLK (the clock that times the shifting) - Pin 2 (CP) of 74HC165
int ploadPin        = 9;  // Connect Pin 8 to SH/!LD (shift or active low load) - Pin 1 (!PL) of 74HC165
int clockEnablePin  = 10;  // Connect Pin 9 to !CE (clock enable, active low) - Pin 15 (!CE) of 74HC165
int dataPin         = 11; // Connect Pin 11 to SER_OUT (Serial data out) - Pin 9 (Q7) of 74HC165

BYTES_VAL_T pinValues;
BYTES_VAL_T oldPinValues;


Relay relays[] = {Relay(A0), Relay(A1), Relay(A2), Relay(A3)};

Button buttons[] = {
  Button(0, MOMENTARY, relays[0]),
  Button(1, NON_MOMENTARY, relays[1]),
  Button(2, MOMENTARY, relays[2]),
  Button(3, MOMENTARY, relays[3])
};


void setup()
{
  Serial.begin(9600);

  setupShiftRegister();

  pinValues = read_shift_regs();
  pinValueChange();
  oldPinValues = pinValues;
}

void loop()
{
  pinValues = read_shift_regs();
  
  if (pinValues != oldPinValues)
  {
    pinValueChange();
    oldPinValues = pinValues;
  }
}

void setupShiftRegister() {
  pinMode(ploadPin, OUTPUT);
  pinMode(clockEnablePin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);

  digitalWrite(clockPin, LOW);
  digitalWrite(ploadPin, HIGH);

}

void setupRelays() {
  for (int i = 0; i < BUTTONS_COUNT; i++) {
    pinMode(relayPin[i], OUTPUT);
    digitalWrite(relayPin[i], HIGH);
  }
}

/* This function is essentially a "shift-in" routine reading the
   serial Data from the shift register chips and representing
   the state of those pins in an unsigned integer (or long).
*/
BYTES_VAL_T read_shift_regs()
{
  long bitVal;
  BYTES_VAL_T bytesVal = 0;

  /* Trigger a parallel Load to latch the state of the data lines,
  */
  digitalWrite(clockEnablePin, HIGH);
  digitalWrite(ploadPin, LOW);
  delayMicroseconds(PULSE_WIDTH_USEC);
  digitalWrite(ploadPin, HIGH);
  digitalWrite(clockEnablePin, LOW);

  /* Loop to read each bit value from the serial out line
     of the SN74HC165N.
  */
  for (int i = 0; i < DATA_WIDTH; i++)
  {
    bitVal = digitalRead(dataPin);

    /* Set the corresponding bit in bytesVal.
    */
    bytesVal |= (bitVal << ((DATA_WIDTH - 1) - i));

    /* Pulse the Clock (rising edge shifts the next bit).
    */
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(clockPin, LOW);
  }

  return (bytesVal);
}

void buttonKeydown(int index) {
  buttons[index].keydown(onButtonKeydown, index);
}

void buttonKeyup(int index) {
  buttons[index].keyup(onButtonKeyup, index);
}

void onButtonKeydown(int index) {
  Button *button = &buttons[index];
  Relay *relay = &relays[index];
  
  if (!button->isNonMomentaryType() || (button->isNonMomentaryType() && !relay->isRelayOn())) {
    relay->turnOn();
  } else {
    relay->turnOff();
  }
}

void onButtonKeyup(int index) {
  if (buttons[index].isNonMomentaryType()) {
    return;
  }

  relays[index].turnOff();
}

bool isButtonPressed(int index) {
  return relayState[index];
}

void setButtonOn(int index) {
  relayState[index] = true;
  digitalWrite(relayPin[index], LOW);
  relayLastAction[index] = millis();
}

void setButtonOff(int index) {
  relayState[index] = false;
  digitalWrite(relayPin[index], HIGH);
  relayLastAction[index] = millis();
}

void pinValueChange() {
  for (int i = 0; i < BUTTONS_COUNT; i++) {

    if ((pinValues >> i) & 1) {
      buttonKeydown(i);
    } else {
      buttonKeyup(i);
    }
  }
}
