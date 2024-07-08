#include "USB.h"
#include "USBHIDKeyboard.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <map>

#define PIN_PIXS 21
#define PIX_NUM 20

Adafruit_NeoPixel pixels(PIX_NUM, PIN_PIXS, NEO_GRB + NEO_KHZ800);
long check10ms = 0;
const u16_t btn_pins[] = {1,  2,  3,  4,  5,  6,  7,  8, 9,
                          10, 11, 12, 13, 14, 16, 17, 18};
const char keyboard_vs[] = "xljgdaoutyipsfhkz";
const u16_t btn_count = sizeof(btn_pins) / sizeof(btn_pins[0]);
u32_t btn_value = 0;
USBHIDKeyboard Keyboard;

inline void initLEDs() {
  pinMode(LED_BUILTIN, OUTPUT);
  pixels.begin();
  pixels.setBrightness(3);
  pixels.clear();
  pixels.show();
}

inline void showStartUp() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  u32_t tests[] = {0xff007f, 0x00ffcd, 0xeed202};
  u16_t length = sizeof(tests) / sizeof(tests[0]);
  for (u16_t j = 0; j < length; j++) {
    for (u16_t i = 0; i < 9; i++) {
      pixels.setPixelColor(8 - i, tests[j]);
      pixels.setPixelColor(8 + i, tests[j]);
      pixels.show();
      delay(50);
    }
    pixels.setPixelColor(18, tests[j]);
    pixels.setPixelColor(19, tests[j]);
    pixels.show();
    delay(500);
    pixels.clear();
    pixels.show();
  }
  pixels.setPixelColor(17, tests[1]);
  pixels.show();
}

void inline setupButtons() {
  u16_t idx = 0;
  for (auto pin : btn_pins) {
    pinMode(pin, INPUT_PULLUP);
  }
  Keyboard.begin();
  USB.begin();
}

void inline onKeyStatusChange(u16_t idx, bool press) {
  pixels.setPixelColor(btn_count - 1 - idx, press ? 0xFF0378 : 0x0);
  if (press) {
    Keyboard.press(keyboard_vs[idx]);
  } else {
    Keyboard.release(keyboard_vs[idx]);
  }
}

void inline onBtnChange(u32_t v_old, u32_t v_new) {
  for (u16_t i = 0; i < btn_count; i++) {
    if ((v_new ^ v_old) & 0x1 == 0x1) {
      onKeyStatusChange(i, v_new & 0x1);
    }
    v_new >>= 1;
    v_old >>= 1;
  }
  pixels.show();
}

void inline checkBtnValue() {
  u32_t bv = 0;
  for (auto it : btn_pins) {
    bv <<= 1;
    auto v = digitalRead(it);
    bv += v ? 0 : 1;
  }
  if (bv != btn_value) {
    onBtnChange(btn_value, bv);
    btn_value = bv;
  }
}

void setup() {
  initLEDs();
  showStartUp();
  setupButtons();
}

void loop() {
  auto ms = millis();
  if (ms - check10ms >= 10) {
    check10ms = ms;
    checkBtnValue();
  }
}