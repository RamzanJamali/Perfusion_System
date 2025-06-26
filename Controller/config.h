#pragma once

#include <Arduino.h>

// ——— Sensor Configuration ———
constexpr uint8_t DHT_PIN  = 2;         // Digital pin connected to the DHT22 data line

// ——— LED and Button ———
//This pin is used by AS5048A magnetic encoder. --> constexpr uint8_t LED_PIN    = 13;      // Onboard LED
constexpr uint8_t BUTTON_PIN =  3;      // Photo interrupter or pushbutton pin

// ——— Stepper / Valve Hardware ———
constexpr uint8_t STEP_PIN   =  8;
constexpr uint8_t DIR_PIN    =  9;
constexpr uint8_t ENABLE_PIN = 7;
constexpr uint8_t VALVE_PIN  =  5;

// ——— AS5048A Encoder ———
constexpr uint8_t CS_PIN = 10;

// Analog pin for pressure sensor
constexpr uint8_t PRESSURE_PIN = A0;    

// For Pressure Calculation
int LOW_PRESSURE = 5; // low pressure in cmH2O
int HIGH_PRESSURE = 10; // high pressure in cmH2O