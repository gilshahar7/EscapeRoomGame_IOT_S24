#ifndef GLOBALS_H
#define GLOBALS_H

#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <PubSubClient.h>
#include <I2CKeyPad.h>
#include <CountDown.h>
#include <HT16K33.h>

enum stage
{
	READY,
    WHEELS,
    FUEL,
    STARS,
    SOLVED
};
stage currentStage;

/* CONSTANTS */
// ESP MQTT TOPIC
const char *ESP_TOPIC = "esp";
const char* ESP_TIMER_TOPIC = "esp_timer";

// MQTT MESSAGES
const char *START_GAME = "start_game";
const char *WHEELS_HINT = "wheels_hint";
const char *WHEELS_SOLVE = "wheels_solved";
const char *FUEL_RESET = "fuel_reset";
const char *FUEL_HINT = "fuel_hint";
const char *FUEL_SOLVE = "fuel_solved";
const char *STARS_HINT = "star_hint";
const char *STARS_SOLVE = "star_solved";
const char *GLOBAL_RESET = "global_reset";
const char *ADD_MIN = "add_min";
const char *SUB_MIN = "sub_min";
const char *COMPARTMENT_OPEN1 = "comp_1_open";
const char *COMPARTMENT_OPEN2 = "comp_2_open";
const char *COMPARTMENT_OPEN3 = "comp_3_open";

// Wi-Fi
WiFiClient espClient;

// MQTT
int connectionTries = 3;
bool connected = false;
IPAddress mqtt_ip;
const char *mqtt_hostname = "DESKTOP-E9DDPAE.local";
const int mqtt_port = 1883;
PubSubClient* mqttClient = nullptr;

// KEYPAD
const int numKeypadLeds = 4;
const int keypadLeds[] = {21, 22, 23, 24};
I2CKeyPad keypad(0x20);
uint8_t prevKeyIndex = 16;
unsigned long keypadLastDebounceTime = 0;

// LEDS
const byte ledsPin = 33;
const int numFuelLeds = 16;
const int numStarLeds = 4;
Adafruit_NeoPixel ws2812b(numFuelLeds + numStarLeds + 1 + numKeypadLeds, ledsPin, NEO_GRB + NEO_KHZ800);

// TIMER
unsigned long lastTimerPublished = 0;
unsigned long gameDuration = 15 * 60;
CountDown timerCountDown(CountDown::SECONDS);
HT16K33 timerDisplay(0x70);

#endif /* GLOBALS_H */
