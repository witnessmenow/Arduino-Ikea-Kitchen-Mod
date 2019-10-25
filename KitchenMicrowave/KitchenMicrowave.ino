/*******************************************************************
    Sketch for my Ikea Kids Kitchen Microwave mod.

    By Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
// Library for controlling the neopixel ring
// Search for "Neopixel" in the Arduino Library manager
// https://github.com/adafruit/Adafruit_NeoPixel

#include <TM1637Display.h>
// Library for the 7 segment display
// Search for "TM1637" in the Arduino Library manager
// https://github.com/avishorp/TM1637

#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
// Library for using the Rotary Encoder
// Search for "encoder" in the Arduino Library manager and select the one
// by Paul Stoffregen
// https://github.com/PaulStoffregen/Encoder

#include <ezTime.h>

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
unsigned long reLastUsedTimer = 0;

unsigned long reTurnAllowedTime = 0;

unsigned long ledChangeDue = 0;

unsigned long countDownDue = 0;

long oldPosition  = -999;

char ssid[] = "wifi";         // your network SSID (name)
char password[] = "yourwifipassword"; // your network password

boolean dotsOn;

unsigned long oneSecondLoopDue = 0;

Timezone myTZ;

const uint8_t LETTER_A = SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G;
const uint8_t LETTER_B = SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
const uint8_t LETTER_C = SEG_A | SEG_D | SEG_E | SEG_F;
const uint8_t LETTER_D = SEG_B | SEG_C | SEG_D | SEG_E | SEG_G;
const uint8_t LETTER_E = SEG_A | SEG_D | SEG_E | SEG_F | SEG_G;
const uint8_t LETTER_F = SEG_A | SEG_E | SEG_F | SEG_G;

const uint8_t LETTER_O = SEG_C | SEG_D | SEG_E | SEG_G;

const uint8_t SEG_BOOT[] = {
  LETTER_B,                                        // b
  LETTER_O,                                        // o
  LETTER_O,                                        // o
  SEG_D | SEG_E | SEG_F | SEG_G                    // t - kinda
};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, ledPin, NEO_GRB + NEO_KHZ800);

Encoder myEnc(reDTPin, reCLKPin);

TM1637Display display(CLK, DIO);

void setup() {

  Serial.begin(115200);

  display.setBrightness(0xff);
  display.setSegments(SEG_BOOT);

    // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Uncomment the line below to see what it does behind the scenes
  // setDebug(INFO);

  waitForSync();

  Serial.println();
  Serial.println("UTC:             " + UTC.dateTime());

  // Provide official timezone names
  // https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
  //myTZ.setLocation(F("Pacific/Auckland"));
  //Serial.print(F("New Zealand:     "));
  //Serial.println(myTZ.dateTime());

  // Wait a little bit to not trigger DDoS protection on server
  // See https://github.com/ropg/ezTime#timezonedropnl
  //delay(5000);

  // Or country codes for countries that do not span multiple timezones
  myTZ.setLocation(F("America/Toronto"));
  Serial.print(F("Time in NY:         "));
  Serial.println(myTZ.dateTime());

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
    reLastUsedTimer = millis() + 5000;
    secondsOnTimer += amountSecondsChange;
    if (secondsOnTimer > 120)
    {
      secondsOnTimer = 120;
    }
  } else if (newPosition < oldPosition) {
    reLastUsedTimer = millis() + 5000;
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

void displayTime(bool dotsVisible) {

  String timeHour = myTZ.dateTime("h");
  //24-hour time
  //String timeHour = myTZ.dateTime("h");
  String timeMinutes = myTZ.dateTime("i");

  uint8_t data[4];


  data[0] = display.encodeDigit(timeHour.substring(0,1).toInt());
  data[1] = display.encodeDigit(timeHour.substring(1).toInt());


  if (dotsVisible) {
    // Turn on double dots
    data[1] = data[1] | B10000000;
  }

  data[2] = display.encodeDigit(timeMinutes.substring(0,1).toInt());
  data[3] = display.encodeDigit(timeMinutes.substring(1).toInt());

  display.setSegments(data);
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
  events();
  //uint32_t tempColour = 0;

  if (reTurnAllowedTime < millis()) {
    handleRotaryEncoderTurn();
    reTurnAllowedTime = millis() + 150;
  }

  //Back to clock mode if we haven't used the rotary encoder for 5 seconds
  if (millis() > reLastUsedTimer && !microwaveOn && secondsOnTimer > 0) {
    secondsOnTimer = 0;
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
  else if (!microwaveOn && secondsOnTimer <= 0)
  {
    unsigned long now = millis();
    if (now > oneSecondLoopDue) {
      displayTime(dotsOn);
      dotsOn = !dotsOn;
      oneSecondLoopDue = now + 1000;
    }
  }
}
