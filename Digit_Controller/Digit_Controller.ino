/*  Keypadtest.pde

    Demonstrate the simplest use of the  keypad library.

    The first step is to connect your keypad to the
    Arduino  using the pin numbers listed below in
    rowPins[] and colPins[]. If you want to use different
    pins then  you  can  change  the  numbers below to
    match your setup.

*/
#include <Keypad.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "SevSeg.h"

// Set the CE & CSN pins
#define CE_PIN   53
#define CSN_PIN 48

SevSeg sevseg; //Instantiate a seven segment object
const byte rxAddr[6] = "00001";

// Create a Radio
RF24 radio(CE_PIN, CSN_PIN);

const byte ROWS = 4; // Four rows
const byte COLS = 4; // Three columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'-', '0', '+', 'D'}
};
char rgbKeys[1][4] = {{'g', 'r', 'c', 'b'}};
byte rgbRowPins[ROWS] = { 26  };
byte rgbColPins[COLS] = { 25, 24, 23, 22 };

// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte colPins[ROWS] = { 9, 8, 7, 6 };
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte rowPins[COLS] = { 5, 4, 3, 2 };

// Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
Keypad rgbKpd = Keypad( makeKeymap(rgbKeys), rgbRowPins, rgbColPins, 1, 4 );

char digit_array[4];
int digit_count = 0;
String numberStr = "";
int number = 0;
void setup()
{
  pinMode(CE_PIN, OUTPUT);

  byte numDigits = 4;
  byte digitPins[] = {39, 38, 37, 36};
  byte segmentPins[] = {28, 29, 30, 31, 32, 33, 34, 35};
  bool resistorsOnSegments = false;
  bool leadingZeros = true;
  sevseg.begin(COMMON_ANODE, numDigits, digitPins, segmentPins, resistorsOnSegments, leadingZeros);

  sevseg.setBrightness(90);

  Serial.begin(9600);
  // Start the Radio!
  radio.begin();
  // Power setting. Due to likelihood of close proximity of the devices, set as RF24_PA_MIN (RF24_PA_MAX is default)
  radio.setPALevel(RF24_PA_MAX); // RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX

  // Slower data rate for better range
  radio.setDataRate( RF24_250KBPS ); // RF24_250KBPS, RF24_1MBPS, RF24_2MBPS

  // Number of retries and set tx/rx address
  radio.setRetries(15, 15);
  radio.openWritingPipe(rxAddr);

  // Stop listening, so we can send!
  radio.stopListening();
   Serial.println("Listening Stopped.");

  //clearDigit();
}

void clearDigit() {
  //number="";
  digit_count = 0;
  for (int i = 0; i < 4; i++) {
    digit_array[i] = '0';
  }
}

void loop()
{
  char key = kpd.getKey();
  char rgbKey = rgbKpd.getKey();
  if (key) // Check for a valid key.
  {
    if (isDigit(key) && digit_count < 3) {
      if (digit_count == 0) {
        numberStr = "";
      }
      numberStr += key;
      Serial.println(numberStr);
      digit_array[digit_count] = key;
      digit_count++;
    }
    else {
      switch (key)
      {
        case '-':
          if (number > 0) {
            number--;
          }
          numberStr = String(number);
          digit_count = numberStr.length();
          itoa(number, digit_array, 10);
          displayNumber();
          break;
        case '+':
          if (number < 999) {
            number++;
          }
          numberStr = String(number);
          digit_count = numberStr.length();
          itoa(number, digit_array, 10);
          displayNumber();
          //Serial.println(key);
          //radio.write(&key, sizeof(char));
          break;
        case 'A':
          numberStr = "";
          clearDigit();
          break;
        case 'D':
          if (digit_count > 0) {
            displayNumber();
          }
          Serial.println(digit_array);
          break;
        default:
          Serial.println(key);
      }
    }
  }

  if (rgbKey) {
    Serial.println(rgbKey);
    Serial.println("Attempting to send");
    bool ok = radio.write(&rgbKey, sizeof(char));
    if (ok)
      Serial.println("ok...sent");
    else
      Serial.println("failed.\n\r");
  }
  sevseg.setNumber(numberStr.toInt(), 2);
  sevseg.refreshDisplay();

}


void displayNumber() {
  Serial.println(digit_array);
  digit_array[digit_count] = 'D';
  Serial.println("Attempting to send");
  bool ok = radio.write(&digit_array, sizeof(digit_array));
  if (ok)
    Serial.println("ok...sent");
  else
    Serial.println("failed.\n\r");
  number = numberStr.toInt();
  if (number == 999) {
    number = 0;
    numberStr = String(number);
  }
  //digit_array[digit_count]
  //clearDigit();
}
