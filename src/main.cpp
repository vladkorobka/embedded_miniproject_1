#include <Arduino.h>

#define BTN_LEFT_PIN 38
#define BTN_RIGHT_PIN 39
#define LDR_PIN 4

const unsigned long DEBOUNCE_MS = 30;

const int RAW_LIGHT = 250;
const int RAW_DARK = 5000;
const uint8_t LEVELS[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180 };
const int NUM_LEVELS = sizeof(LEVELS) / sizeof(LEVELS[0]);

struct Color {
  const char *name;
  uint8_t r, g, b;
};

const Color COLORS[] = {
    {"red", 255, 0, 0},
    {"green", 0, 255, 0},
    {"blue", 0, 0, 255},
    {"yellow", 255, 160, 0},
};
const int NUM_COLORS = sizeof(COLORS) / sizeof(COLORS[0]);

bool debounceStep(uint8_t pin, bool &raw, bool &stable, unsigned long &lastChange) {
  bool reading = (digitalRead(pin) == LOW);

  if (reading != raw) {
    lastChange = millis();
    raw = reading;
  }

  if (millis() - lastChange > DEBOUNCE_MS) {
    stable = raw;
  }
  return stable;
}

bool leftRaw = false, leftStable = false, leftPrev = false;
unsigned long leftLastChange = 0;

bool rightRaw = false, rightStable = false, rightPrev = false;
unsigned long rightLastChange = 0;

int colorIndex = 0;
int brightness = LEVELS[0];
unsigned long lastLogMs = 0;
int lastRaw = 0;

void updateLed() {
  const Color &c = COLORS[colorIndex];
  neopixelWrite(RGB_BUILTIN, c.r * brightness / 255, c.g * brightness / 255, c.b * brightness / 255);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("Miniproject 1: Color lamp: buttons + LDR brightness");

  pinMode(BTN_LEFT_PIN, INPUT_PULLUP);
  pinMode(BTN_RIGHT_PIN, INPUT_PULLUP);

  // updateLed();
}

void loop() {
  // натиснув праву кнопку
  bool rightNow = debounceStep(BTN_RIGHT_PIN, rightRaw, rightStable, rightLastChange);
  if (rightNow && !rightPrev) {
    colorIndex = (colorIndex + 1) % NUM_COLORS;
    updateLed();
  }
  rightPrev = rightNow;

  // натиснув ліву кнопку
  bool leftNow = debounceStep(BTN_LEFT_PIN, leftRaw, leftStable, leftLastChange);
  if (leftNow && !leftPrev) {
    colorIndex = (colorIndex + NUM_COLORS - 1) % NUM_COLORS;
    updateLed();
  }
  leftPrev = leftNow;

  // читаємо значення фоторезистора
  lastRaw = analogRead(LDR_PIN);

  int level = map(lastRaw, RAW_LIGHT, RAW_DARK, 0, NUM_LEVELS);
  level = constrain(level, 0, NUM_LEVELS - 1);

  if (LEVELS[level] != brightness) {
    brightness = LEVELS[level];
    updateLed();
  }

  if (millis() - lastLogMs >= 500) {
    lastLogMs = millis();
    Serial.printf("color=%s ADC raw=%d brightness=%d\n", COLORS[colorIndex].name, lastRaw, brightness);
  }
}