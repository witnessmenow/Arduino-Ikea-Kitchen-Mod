/*******************************************************************
    Sketch for my Ikea Kids Kitchen oven mod.

    By Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/


#include <Adafruit_NeoPixel.h>
// Library for controlling the neopixel ring
// Search for "Neopixel" in the Arduino Library manager
// https://github.com/adafruit/Adafruit_NeoPixel

#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
// Library for using the Rotary Encoder
// Search for "encoder" in the Arduino Library manager and select the one
// by Paul Stoffregen
// https://github.com/PaulStoffregen/Encoder

#define COLOUR_WHITE 0
#define COLOUR_RED 1
#define COLOUR_BLUE 2
#define COLOUR_GREEN 3

// --------
// Buttons
// --------
// One leg of the buttons connected to Ground
// The other connected to the following
const int blueButton = D4;
const int greenButton = D7;
const int whiteButton = D5;
const int redButton = D6;

#define NUM_BUTTONS 4


// --------
// Neopixels
// --------
const int ledPin = D8;
#define NUMPIXELS      32

// --------
// Rotary Encoder
// --------

const int reButtonPin = D3;
const int reDTPin = D2;
const int reCLKPin = D1;



#define DEFAULT_BRIGHTNESS 128;
bool lightsAreOn = false;
uint32_t colour = 0;
int brightness = DEFAULT_BRIGHTNESS;
int currentColourCode = 0;

unsigned long reButtonCoolDown = 1000;
unsigned long reButtonReadAllowedTime = 0;
unsigned long ovenTimeout = 0;
long oldPosition  = -999;

int buttons[NUM_BUTTONS] = {blueButton, greenButton, whiteButton, redButton};
int buttonsCodes[NUM_BUTTONS] = {COLOUR_BLUE, COLOUR_GREEN, COLOUR_WHITE, COLOUR_RED};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, ledPin, NEO_GRB + NEO_KHZ800);

Encoder myEnc(reDTPin, reCLKPin);

void setup() {

  Serial.begin(115200);

  colour = getColour(COLOUR_RED);

  // initialize the pushbutton pin as an input:
  pinMode(blueButton, INPUT_PULLUP);
  pinMode(greenButton, INPUT_PULLUP);
  pinMode(whiteButton, INPUT_PULLUP);
  pinMode(redButton, INPUT_PULLUP);

  pinMode(reButtonPin, INPUT_PULLUP);

  // Get current Position
  oldPosition = myEnc.read();

  pixels.begin();
  turnOffAllPixels();
  setAllPixels(getColour(0));
}

void setAllPixels(uint32_t targetColour) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, targetColour);
  }

  pixels.show();
}

void turnOffAllPixels(){
  setAllPixels(pixels.Color(0, 0, 0));
}

uint32_t getColour(int colourCode) {
  uint32_t tempColour = 0;
  switch (colourCode) {
    case COLOUR_WHITE:
      tempColour = pixels.Color(brightness, brightness, brightness);
      break;
    case COLOUR_RED:
      tempColour = pixels.Color(brightness, 0, 0);
      break;
    case COLOUR_BLUE:
      tempColour = pixels.Color(0, 0, brightness);
      break;
    case COLOUR_GREEN:
      tempColour = pixels.Color(0, brightness, 0);
      break;
  }

  return tempColour;
}

bool getBrightnessChange() {
  bool brightnessChanged = false;
  int amountBirghtnessChange = 5;

  long newPosition = myEnc.read();
  if (newPosition > oldPosition) {
    brightnessChanged = true;
    brightness += amountBirghtnessChange;
    if (brightness > 255)
    {
      brightness = 255;
    }
  } else if (newPosition < oldPosition) {
    brightnessChanged = true;
    brightness -= amountBirghtnessChange;
    if (brightness < 10)
    {
      brightness = 10;
    }
  }

  if(newPosition != oldPosition){
    Serial.print("Brigthness: ");
    Serial.println(brightness);
  }

  oldPosition = newPosition;

  return brightnessChanged;
}

void loop() {
  //uint32_t tempColour = 0;
  bool newColour = false;
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (digitalRead(buttons[i]) == LOW) {
      currentColourCode = buttonsCodes[i];
      //colour = getColour(buttonsCodes[i]);
      newColour = true;
      Serial.print("Button Pressed: ");
      Serial.println(i);
    }
  }

  bool newBrightness = getBrightnessChange();

  if (reButtonReadAllowedTime < millis()) {
    if (digitalRead(reButtonPin) == LOW) {
      reButtonReadAllowedTime = millis() + reButtonCoolDown;
      lightsAreOn = !lightsAreOn;

      if (lightsAreOn) {
        setAllPixels(colour);
      } else {
        turnOffAllPixels();
      }

    }
  }

  if (newColour || newBrightness) {
    ovenTimeout = millis() + 300000;
    colour = getColour(currentColourCode);
    lightsAreOn = true;
    setAllPixels(colour);
  }

  if (millis() == ovenTimeout && lightsAreOn) {
    lightsAreOn = false;
    turnOffAllPixels();
  }
}
