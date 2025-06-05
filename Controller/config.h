#pragma once

#include <Arduino.h>

// ——— Sensor Configuration ———
constexpr uint8_t DHT_PIN  = 2;         // Digital pin connected to the DHT22 data line

// ——— LED and Button ———
constexpr uint8_t LED_PIN    = 13;      // Onboard LED
constexpr uint8_t BUTTON_PIN =  3;      // Photo interrupter or pushbutton pin

// ——— Stepper / Valve Hardware ———
constexpr uint8_t STEP_PIN   =  8;
constexpr uint8_t DIR_PIN    =  9;
constexpr uint8_t ENABLE_PIN = 10;
constexpr uint8_t VALVE_PIN  =  5;