/*******************************************************************
    Sketch for my Ikea Kids Kitchen Microwave mod.

    By Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/


#include <Adafruit_NeoPixel.h>
// Library for controlling the neopixel ring
// Search for "Neopixel" in the Arduino Library manager
// https://github.com/adafruit/Adafruit_NeoPixel

#include <TM1637Display.h>
// Library for the 7 segment display
// Search for "TM1637" in the Arduino Library manager
// https://github.com/avishorp/TM1637

#include <Encoder.h>
// Library for using the Rotary Encoder
// Search for "encoder" in the Arduino Library manager and select the one
// by Paul Stoffregen
// https://github.com/PaulStoffregen/Encoder

// --------
// Buzzer
// --------
#define BUZZER_PIN D7


// --------
// 7-Segment Display
// --------
#define CLK D6
#define DIO D5

// --------
// Neopixels
// --------
const int ledPin = D8;
#define NUMPIXELS      16

// --------
// Rotary Encoder
// --------

const int reButtonPin = D3;
const int reDTPin = D2;
const int reCLKPin = D1;



#define DEFAULT_BRIGHTNESS 128;
bool microwaveOn = false;
uint32_t colour = 0;
int brightness = DEFAULT_BRIGHTNESS;
int currentColourCode = 0;

int secondsOnTimer = 0;
int displayedSeconds = -1;

int startingLedIndex = 0;
const int numberOfLedsOn = 4;

unsigned long reButtonCoolDown = 1000;
unsigned long reButtonReadAllowedTime = 0;

unsigned long reTurnAllowedTime = 0;

unsigned long ledChangeDue = 0;

unsigned long countDownDue = 0;

long oldPosition  = -999;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, ledPin, NEO_GRB + NEO_KHZ800);

Encoder myEnc(reDTPin, reCLKPin);

TM1637Display display(CLK, DIO);

void setup() {

  Serial.begin(115200);

  display.setBrightness(0xff);

  colour = pixels.Color(255, 255, 0);

  pinMode(reButtonPin, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Get current Position
  oldPosition = myEnc.read();

  pixels.begin();
  turnOffAllPixels();
  //setAllPixels(getColour(0));
}

void setAllPixels(uint32_t targetColour) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, targetColour);
  }

  pixels.show();
}

void turnOffAllPixels() {
  setAllPixels(pixels.Color(0, 0, 0));
}

void handleRotaryEncoderTurn() {
  int amountSecondsChange = 5;

  long newPosition = myEnc.read();
  if (newPosition > oldPosition) {
    secondsOnTimer += amountSecondsChange;
    if (secondsOnTimer > 120)
    {
      secondsOnTimer = 120;
    }
  } else if (newPosition < oldPosition) {
    secondsOnTimer -= amountSecondsChange;
    if (secondsOnTimer < 0)
    {
      secondsOnTimer = 0;
    }
  }

  if (newPosition != oldPosition) {
    Serial.print("secondsOnTimer: ");
    Serial.println(secondsOnTimer);
  }

  oldPosition = newPosition;
}

void updateTimer(int sec) {
  int displayMinutes = sec / 60;
  int displaySeconds = sec % 60;

  uint8_t data[4];

  data[0] = display.encodeDigit(displayMinutes / 10);
  data[1] = display.encodeDigit(displayMinutes % 10);
  data[1] = data[1] | B10000000;
  data[2] = display.encodeDigit(displaySeconds / 10);
  data[3] = display.encodeDigit(displaySeconds % 10);

  display.setSegments(data);

  displayedSeconds = sec;

  Serial.println(sec);
}

void timerFinished() {
  turnOffAllPixels();
  microwaveDoneBuzzer();
  microwaveOn = false;
}

void microwaveDoneBuzzer() {
  for (int i = 0; i < 3; i++)
  {
    tone(BUZZER_PIN, 2000, 500);
    delay(700);
    noTone(BUZZER_PIN);
    delay(350);
  }
}

void updateLeds() {
  //turnOffAllPixels();
  for (int i = 0; i < numberOfLedsOn; i++) {
    int index = startingLedIndex + i;
    index = index % NUMPIXELS;
    pixels.setPixelColor(index, colour);
  }

  int trailingIndex = startingLedIndex - 1;
  if (trailingIndex < 0 )
  {
    trailingIndex = NUMPIXELS - 1;
  }
  pixels.setPixelColor(trailingIndex, pixels.Color(0, 0, 0));


  startingLedIndex++;
  if (startingLedIndex >= NUMPIXELS) {
    startingLedIndex = 0;
  }

  pixels.show();
}

void loop() {
  //uint32_t tempColour = 0;

  if (reTurnAllowedTime < millis()) {
    handleRotaryEncoderTurn();
    reTurnAllowedTime = millis() + 150;
  }

  if (reButtonReadAllowedTime < millis()) {
    if (digitalRead(reButtonPin) == LOW) {
      reButtonReadAllowedTime = millis() + reButtonCoolDown;
      microwaveOn = !microwaveOn;

      if (microwaveOn) {
        countDownDue = millis() + 1000;
        ledChangeDue = 0;
        //setAllPixels(colour);
      } else {
        turnOffAllPixels();
      }

    }
  }

  if (displayedSeconds != secondsOnTimer) {
    updateTimer(secondsOnTimer);
  }

  if (microwaveOn) {
    if (secondsOnTimer > 0) {
      if (millis() > countDownDue) {
        secondsOnTimer--;
        countDownDue = millis() + 1000;
      }
    }

    if (secondsOnTimer == 0) {
      timerFinished();
    } else if (millis() > ledChangeDue) {
      updateLeds();
      ledChangeDue = millis() + 250;
    }
  }

}
