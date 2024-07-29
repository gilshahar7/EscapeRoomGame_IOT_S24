#ifndef GLOBALS_H
#define GLOBALS_H

#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <I2CKeyPad.h>
#include <arduino-timer.h>
#include <DIYables_4Digit7Segment_74HC595.h> // timer
#include <CountDown.h>

enum stage {WHEELS, WATER, STARS, SOLVED};
stage currentStage;

/* CONSTANTS */
// ESP MQTT TOPIC
const char* ESP_TOPIC = "esp";

// MQTT MESSAGES
const char* WHEELS_HINT = "wheels_hint";
const char* WHEELS_SOLVE = "wheels_solved";
const char* WATER_RESET = "water_reset";
const char* WATER_HINT = "water_hint";
const char* WATER_SOLVE = "water_solved";
const char* STARS_HINT = "star_hint";
const char* STARS_SOLVE = "star_solved";
const char* GLOBAL_RESET = "global_reset";

// Wi-Fi
const char* ssid = "AndroidAP";
const char* password = "yuval3101";
WiFiClient espClient;

// MQTT
int connectionTries = 3;
bool connected = false;
const char* mqtt_server = "192.168.39.237";
const int mqtt_port = 1883;
PubSubClient mqttClient(mqtt_server, mqtt_port, espClient);

// TRANSFERRING WATER
const byte ledsPin = 33;
const int numWaterLeds = 16;

// STARRY NIGHT
auto blinkTimer = timer_create_default();

const int numStarLeds = 4;
const int blinkingStars[4] = {16,17,18,19};

// KEYPAD
const int numKeypadLeds = 4;
const int keypadLeds[] = {21, 22, 23, 24};
I2CKeyPad keypad(0x20);
uint8_t prevKeyIndex = 16;
unsigned long keypadLastDebounceTime = 0;

// LEDS
Adafruit_NeoPixel ws2812b(numWaterLeds + numStarLeds + 1 + numKeypadLeds, ledsPin, NEO_GRB + NEO_KHZ800);

// TIMER
CountDown timerCountDown(CountDown::SECONDS);

const byte timerSCLK = 5;
const byte timerRCLK = 18;
const byte timerDIO = 19;
DIYables_4Digit7Segment_74HC595 timerDisplay(timerSCLK, timerRCLK, timerDIO);


#endif /* GLOBALS_H */
