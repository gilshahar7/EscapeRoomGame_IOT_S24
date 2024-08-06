#ifndef WHEELS_H
#define WHEELS_H

#include "globals.h"
#include <Compartment.h>

class Wheels
{
public:
    Wheels() : _hintGiven(false), compartment(_relayPin) {}
    void setup()
    {
        pinMode(_puzzlePin, INPUT_PULLUP);

        pinMode(_relayPin, OUTPUT);
        digitalWrite(_relayPin, LOW);

        // Set hint LED
        ws2812b.setPixelColor(_hintLedIndex, ws2812b.Color(0, 0, 0));
        ws2812b.show();
    }

    void reset()
    {
        _hintGiven = false;
        ws2812b.setPixelColor(_hintLedIndex, ws2812b.Color(0, 0, 0));
        ws2812b.show();
    }

    void play()
    {
        if (digitalRead(_puzzlePin) == HIGH)
        {
            solve();
        }
    }

    void hint()
    {
        ws2812b.setPixelColor(_hintLedIndex, ws2812b.Color(0, 200, 255));
        ws2812b.show();
        _hintGiven = true;
    }

    /**
     * Solves the puzzle by activating the wheels.
     * If the puzzle has not been solved yet, it sets the solve time, marks the puzzle as solved,
     * and activates the relay pin.
     * If the puzzle has already been solved and the time since the last solve is greater than or equal to 1000 milliseconds,
     * it updates the solve time, deactivates the relay pin, sets the current stage to FUEL,
     * and publishes a message to the MQTT topic.
     */
    void solve()
    {
        compartment.open();
        currentStage = FUEL;
        mqttClient->publish(ESP_TOPIC, WHEELS_SOLVE);
    }

private:
    bool _hintGiven;
    const byte _relayPin = 13;
    const byte _puzzlePin = 15;
    const int _hintLedIndex = 20;
public:
    Compartment compartment;
};

#endif /* WHEELS_H */
