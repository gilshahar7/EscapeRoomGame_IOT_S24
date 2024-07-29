#ifndef WHEELS_H
#define WHEELS_H

#include "globals.h"

class Wheels
{
public:
    Wheels() : _solveTime(0), _solved(false), _hintGiven(false) {}
    void setup() {
        pinMode(_puzzlePin, INPUT_PULLUP);

        pinMode(_relayPin, OUTPUT);
        digitalWrite(_relayPin, LOW);

        // Set hint LED
        ws2812b.setPixelColor(_hintLedIndex, ws2812b.Color(0, 0, 0));
        ws2812b.show();
    }

    void reset() {
        _hintGiven = false;
        _solved = false;
        ws2812b.setPixelColor(_hintLedIndex, ws2812b.Color(0, 0, 0));
        ws2812b.show();
    }

    void checkSolved() {
        if (digitalRead(_puzzlePin) == HIGH) {
            solve(false/*isAdmin*/);
        }
    }

    void hint() {
        ws2812b.setPixelColor(_hintLedIndex, ws2812b.Color(0, 200, 255));
        ws2812b.show();
        _hintGiven = true;
    }

    void solve(bool isAdmin) {
        unsigned long currentTime = millis();
        if (!_solved) {
            _solveTime = millis();
            _solved = true;
            digitalWrite(relayPin1, HIGH);
        }
        else if (currentTime - _solveTime >= 1000) {
            _solveTime = currentTime;
            digitalWrite(relayPin1, LOW);
            currentStage = WATER;
            if (!isAdmin)
                mqttClient.publish(ESP_TOPIC, WHEELS_SOLVE);
            Serial.println("solved wheels");
        }
    }

private:
    unsigned long _solveTime;
    bool _solved;
    bool _hintGiven;
    const byte _relayPin = 13;
    const byte _puzzlePin = 15;
    const int _hintLedIndex = 20;
};

#endif /* WHEELS_H */
