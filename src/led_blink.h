#ifndef LED_BLINK_KD_H
#define LED_BLINK_KD_H

#include <Arduino.h>

// BOARD
#define LED_PIN T2
#define LED_ON LOW
#define LED_OFF HIGH

#define BLINK_DELAY_IN_MS 500

enum indicator { WIFI_ERROR = 3,
                 MQTT_ERROR = 4 };

int ledState = LED_OFF;

void initLed(bool on) {
    pinMode(LED_PIN, OUTPUT);
    ledState = (on ? LED_ON : LED_OFF);
    digitalWrite(LED_PIN, ledState);
}

void setLed(bool on) {
    ledState = (on ? LED_ON : LED_OFF);
    digitalWrite(LED_PIN, ledState);
}
void ledOn() {
    setLed(true);
}

void ledOff() {
    setLed(false);
}

void ledBlinkTimes(int times) {
    if (times < 1) return;
    // if led is on, turn it off first
    if (ledState == LED_ON) ledOff();

    for (int i = 0; i < times; i++) {
        delay(BLINK_DELAY_IN_MS);
        ledOn();
        delay(BLINK_DELAY_IN_MS);
        ledOff();
    }
}

#endif  // LED_BLINK_KD_H