#include <Arduino.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Adafruit_NeoPixel.h>

// ---------- PINs ----------
#define LED_PIN   5   // D1
#define IR_PIN    4   // D2
#define NUM_LEDS  20

// ---------- STEP ----------
#define BRIGHTNESS_STEP  30
#define COLOR_STEP       30

// ---------- KEYS ----------
#define IR_POWER   0xFFA25D
#define IR_UP      0xFF02FD
#define IR_DOWN    0xFF9867
#define IR_MODE    0xFF629D

#define IR_RED_U   0xFF30CF
#define IR_GREEN_U 0xFF18E7
#define IR_BLUE_U  0xFF7A85

#define IR_RED_D   0xFF10EF
#define IR_GREEN_D 0xFF38C7
#define IR_BLUE_D  0xFF5AA5

// ---------- IR ----------
IRrecv irrecv(IR_PIN);
decode_results results;

// ---------- LED ----------
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ---------- STATUS ----------
bool powerState = true;
uint8_t brightness = 120;

uint8_t red = 80;
uint8_t green = 80;
uint8_t blue = 80;

uint8_t mode = 0;
unsigned long lastTick = 0;

// ---------- EFFECTS ----------
void staticColor() {
  strip.setBrightness(brightness);
  strip.fill(strip.Color(red, green, blue));
  strip.show();
}

void rainbow() {
  static uint16_t hue = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.ColorHSV(hue + i * 1500));
  }
  strip.setBrightness(brightness);
  strip.show();
  hue += 250;
}

void chase() {
  static int pos = 0;
  strip.clear();
  strip.setPixelColor(pos, strip.Color(red, green, blue));
  strip.setBrightness(brightness);
  strip.show();
  pos = (pos + 1) % NUM_LEDS;
}

void blink() {
  static bool on = false;
  strip.clear();
  if (on) strip.fill(strip.Color(red, green, blue));
  strip.setBrightness(brightness);
  strip.show();
  on = !on;
}

void police() {
  static bool side = false;
  strip.clear();
  for (int i = 0; i < NUM_LEDS / 2; i++) {
    strip.setPixelColor(i, side ? strip.Color(255, 0, 0) : 0);
    strip.setPixelColor(i + NUM_LEDS / 2, side ? 0 : strip.Color(0, 0, 255));
  }
  strip.setBrightness(brightness);
  strip.show();
  side = !side;
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  irrecv.enableIRIn();

  strip.begin();
  strip.show();
  strip.setBrightness(brightness);

  Serial.println("IR RGB Controller Ready");
}

// ---------- LOOP ----------
void loop() {
  if (irrecv.decode(&results)) {
    uint32_t code = results.value;

    // ---- Power ----
    if (code == IR_POWER) {
      powerState = !powerState;
      if (!powerState) {
        strip.clear();
        strip.show();
      }
    }

    // ---- Brightness ----
    else if (code == IR_UP)
      brightness = (brightness + BRIGHTNESS_STEP <= 255) ? brightness + BRIGHTNESS_STEP : 255;

    else if (code == IR_DOWN)
      brightness = (brightness >= BRIGHTNESS_STEP) ? brightness - BRIGHTNESS_STEP : 0;

    // ---- Mode ----
    else if (code == IR_MODE)
      mode = (mode + 1) % 5;

    // ---- Color UP ----
    else if (code == IR_RED_U)
      red = (red + COLOR_STEP <= 255) ? red + COLOR_STEP : 255;

    else if (code == IR_GREEN_U)
      green = (green + COLOR_STEP <= 255) ? green + COLOR_STEP : 255;

    else if (code == IR_BLUE_U)
      blue = (blue + COLOR_STEP <= 255) ? blue + COLOR_STEP : 255;

    // ---- Color DOWN ----
    else if (code == IR_RED_D)
      red = (red >= COLOR_STEP) ? red - COLOR_STEP : 0;

    else if (code == IR_GREEN_D)
      green = (green >= COLOR_STEP) ? green - COLOR_STEP : 0;

    else if (code == IR_BLUE_D)
      blue = (blue >= COLOR_STEP) ? blue - COLOR_STEP : 0;

    irrecv.resume();
  }

  if (!powerState) return;

  if (millis() - lastTick > 70) {
    lastTick = millis();

    switch (mode) {
      case 0: staticColor(); break;
      case 1: rainbow();     break;
      case 2: chase();       break;
      case 3: blink();       break;
      case 4: police();      break;
    }
  }
}
