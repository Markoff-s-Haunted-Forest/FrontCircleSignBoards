
// Arduino 7 segment display example software
// http://www.hacktronics.com/Tutorials/arduino-and-7-segment-led.html
// License: http://www.opensource.org/licenses/mit-license.php (Go crazy)

// Define the LED digit patters, from 0 - 9
// Note that these patterns are for common cathode displays
// For common anode displays, change the 1's to 0's and 0's to 1's
// 1 = LED on, 0 = LED off, in this order:
//                                    Arduino pin: 2,3,4,5,6,7,8
/*byte seven_seg_digits[10][7] = { { 1,1,1,0,1,1,1 },  // = 0
                                                           { 0,0,1,0,0,0,1 },  // = 1
                                                           { 1,0,1,1,1,1,0 },  // = 2
                                                           { 1,0,1,1,0,1,1 },  // = 3
                                                           { 0,1,1,1,0,0,1 },  // = 4
                                                           { 1,1,0,1,0,1,1 },  // = 5
                                                           { 1,1,0,1,1,1,1 },  // = 6
                                                           { 1,0,1,0,0,0,1 },  // = 7
                                                           { 1,1,1,1,1,1,1 },  // = 8
                                                           { 1,1,1,1,0,1,1 }   // = 9
                                                           };
*/

//***********************************************************************TUOMAS ADDED CODE*******************************************************************

//reversed polarity (5v relays are active LOW, 1 = LED off, 0 = LED on)
//when I get my 12v relays, this will switch back to active HIGH

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// define the pins
#define CE_PIN   53
#define CSN_PIN 48

// Create a Radio
RF24 radio(CE_PIN, CSN_PIN);

// The tx/rx address
const byte rxAddr[6] = "00001";


byte seven_seg_digits[10][7] = { { 1, 1, 1, 0, 1, 1, 1 }, // = 0
  { 0, 0, 1, 0, 0, 0, 1 }, // = 1
  { 1, 0, 1, 1, 1, 1, 0 }, // = 2
  { 1, 0, 1, 1, 0, 1, 1 }, // = 3
  { 0, 1, 1, 1, 0, 0, 1 }, // = 4
  { 1, 1, 0, 1, 0, 1, 1 }, // = 5
  { 1, 1, 0, 1, 1, 1, 1 }, // = 6
  { 1, 0, 1, 0, 0, 0, 1 }, // = 7
  { 1, 1, 1, 1, 1, 1, 1 }, // = 8
  { 1, 1, 1, 1, 0, 1, 1 } // = 9
};
//};

//---------------------------------------------RGB Switch----------------------------------------------------------
/*
  RGB Common switching (pins 9, 10, 11)
  These pins will control the color of the LEDs
  These pins will be default : (Red ON, Green OFF, Blue OFF)

  These will be buttons on the controller panel 4-button strip (1 = Red, 2 = Green, 3 = Blue, 4 = All Off)
  1 = Change Red (pin 9) State (low to high or high to low)
  2 = Change Green (pin 10) State (low to high or high to low)
  3 = Change Blue (pin 11) State (low to high or high to low)
  4 = Change all (Red, Green, Blue) state to OFF (either active high or active low) ********************************
  The buttons will be on / off, to allow for color mixing
*/

//int red = 9;
//pinMode(red, OUTPUT);
//digitalWrite(red, LOW); //Set to LED ON, ****active LOW, will Switch with 12v relays*****
//
//int green = 10;
//pinMode(green, OUTPUT);
//digitalWrite(green, HIGH); //Set to LED OFF, ****active LOW, will Switch with 12v relays****
//
//int blue = 11;
//pinMode(blue, OUTPUT);
//digitalWrite(blue, HIGH); //Set to LED OFF, ****active LOW, will Switch with 12v relays****

//---------------------------------------------------------------------------------------------------------------------

//------------------------------------------------NumPad Funtions------------------------------------------------------
/*
   1,2,3,4,5,6,7,8,9,0 = Respective digits
   A = Clear / Delete (in case of typos on number pad)
   B = NOTHING
   C = NOTHING
   D = 'Enter' for number entry
 * * = '-' (subtract 1 from output number)
   # = '+' (add 1 from output number)
*/

//---------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------4 digit SSD------------------------------------------------------------
/*
   to serve as the visual component for the input controller
   will show numbers as they're typed, and allow for 'clear' and 'submit'
   will continue to show whatever number is currently on the boards
*/
//---------------------------------------------------------------------------------------------------------------------

//------------------------------------------------RF transcievers for wireless control---------------------------------
/*
   there will be 1 transciever on each board (3 separate boards)
   The controller panel will be connected to Board 1 via TX/RX with a cable connection
   The Transciever on Board 1 will repeat the signal (from the control panel) and transmit to the other 2 Boards
*/
//---------------------------------------------------------------------------------------------------------------------


// ****************************************************END OF TUOMAS CODE************************************************************************

int rgbPin[3] = {9, 10, 11};
int defaultColor = 0;
int digits[3] = {0, 0, 0};
int startPin[3] = {2, 22, 40};
int number = 0;
int tmpDigits[3] = {0, 0, 0};
String inString = "";
void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 3; i++) {
    int pin = startPin[i];
    for (int j = 0; j < 7; j++) {
      pinMode(pin, OUTPUT);
      pin++;
    }
  }

  for (int p = 0; p < 3; p++) {
    pinMode(rgbPin[p], OUTPUT);
  }

  displayDigits();
  changeColor(0);
  setupRF();
}

void setupRF() {
  pinMode(53, OUTPUT);
  // Start the radio, again set to min & slow as I'm guessing while testing theire really close to each other
  radio.begin();
  // Power setting. Due to likelihood of close proximity of the devices, set as RF24_PA_MIN (RF24_PA_MAX is default)
  radio.setPALevel(RF24_PA_MAX); // RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX

  // Slower data rate for better range
  radio.setDataRate( RF24_250KBPS ); // RF24_250KBPS, RF24_1MBPS, RF24_2MBPS

  // Set the reading pipe and start listening
  radio.openReadingPipe(0, rxAddr);
  radio.startListening();
  Serial.println("Listening");
}

void changeColor(int color) {

  for (int p = 0; p < 3; p++) {
    if (color != 3) {
      if (p == color) {
        digitalWrite(rgbPin[p], !digitalRead(rgbPin[p]));
      }
    } else {
      digitalWrite(rgbPin[p], LOW); // need to change HIGH to LOW when using 12v relay
    }
  }

}

void sevenSegWrite(byte digit, int d) {
  byte pin = startPin[d];
  for (byte segCount = 0; segCount < 7; ++segCount) {
    digitalWrite(pin, seven_seg_digits[digit][segCount]);
    ++pin;
  }
}
int digitCount = 0;
void loop() {
  if ( radio.available()) {
    while (radio.available()) {
      //JD DEBUG
      Serial.println("Radio Available");
      char text[4] = {0};
      radio.read(&text, sizeof(text));
      int textSize = sizeof(text);
      Serial.println(text);
      int inChar = text[0];
      // Serial.println(inChar);
      if (isDigit(inChar)) {
        Serial.println(inChar);
        while (digitCount < textSize) {
          inChar = text[digitCount];
          if ((char)inChar == 'D') {
            break;
          }
          inString += (char)inChar;
          digitCount++;
        }
      }
      switch (inChar) {
        case 'r':
          changeColor(0);
          break;
        case 'g':
          changeColor(1);
          break;
        case 'b':
          changeColor(2);
          break;
        case 'c':
          changeColor(3);
          break;
        case 'D':
        case '\n':

          if (digitCount > 0 && digitCount < 4) {

            digitCount = 0;
            number = inString.toInt();
            inString = "";
            if (number == 999) {
              number = 0 ;

              clearDigit();
              return;
            }
            displayDigits();
          }
          break;
      }
    }
  }
  else {
    Serial.println("Radio Unavailable");
  }

}

void displayDigits() {
  int tmpNum = number;
  for (int i = 2; i > -1; i--) {
    sevenSegWrite(tmpNum % 10, i);
    tmpNum = tmpNum / 10;
  }
}


void clearDigit() {
  for (int i = 0; i < 3; i++) {
    int pin = startPin[i];
    for (int j = 0; j < 7; j++) {
      digitalWrite(pin, 0);
      pin++;
    }
  }
}
