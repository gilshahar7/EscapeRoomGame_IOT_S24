#ifndef WATER_H
#define WATER_H

#include "globals.h"

#define BLINK_COUNT 10

enum hintState
{
    OFF,
    FIRST_TRANSFER,
    SECOND_TRANSFER,
    HINT_GIVEN
};

class Water
{
public:
    Water() : _lastTransferTime(0),
              _transferState(false),
              _fromJug(-1),
              _toJug(-1),
              _blinkCount(BLINK_COUNT),
              _lastBlinkTime(0),
              _targetJug(-1),
              _blinkState(true),
              _light(true),
              _lastConnectTime(0),
              _connectState(false),
              _solveTime(0),
              _solved(false),
              _hintState(OFF)
    {
    }

    void setup()
    {
        pinMode(_relayPin, OUTPUT);
        digitalWrite(_relayPin, LOW);
        pinMode(_resetButtonPin, INPUT_PULLUP);
        pinMode(_transferButtonPin, INPUT_PULLUP);
        pinMode(_transferPossibleLED, OUTPUT);
        digitalWrite(_transferPossibleLED, LOW);
        updateDisplay();
    }

    void reset(bool global)
    {
        if (global)
            _hintState = OFF;
        _solved = false;
        _transferState = false;

        _currentValues[0] = _hintState == HINT_GIVEN ? 3 : 8;
        _currentValues[1] = _hintState == HINT_GIVEN ? 2 : 0;
        _currentValues[2] = _hintState == HINT_GIVEN ? 3 : 0;
        updateDisplay();
    }

    void hint()
    {
        if (_hintState == OFF)
        {
            reset(false /*global*/);
            _hintState = FIRST_TRANSFER;
        }
        else if (_hintState == FIRST_TRANSFER)
        {
            if (transfer(0, 1)) // 8, 0, 0 -> 3, 5, 0
                _hintState = SECOND_TRANSFER;
        }
        else if (_hintState == SECOND_TRANSFER)
        {
            if (transfer(1, 2)) // 3, 5, 0 -> 3, 2, 3
                _hintState = HINT_GIVEN;
        }
    }

    void solve()
    {
        unsigned long currentTime = millis();
        if (!_solved)
        {
            _solveTime = millis();
            _solved = true;
            _blinkState = false;
            digitalWrite(_relayPin, HIGH);
        }
        else if (currentTime - _solveTime >= 1000)
        {
            _solveTime = currentTime;
            _blinkState = true;
            digitalWrite(_relayPin, LOW);
            currentStage = STARS;
            mqttClient.publish(ESP_TOPIC, WATER_SOLVE);
        }
    }

    void updateDisplay()
    {
        int ledIndex = 0;
        for (int jug = 0; jug < _numJugs; jug++)
        {
            for (int i = 0; i < _capacities[jug]; i++)
            {
                if (i < _currentValues[jug])
                {
                    ws2812b.setPixelColor(_ledMapping[ledIndex], ws2812b.Color(0, 0, 25)); // it only takes effect if pixels.show() is called
                }
                else
                {
                    ws2812b.setPixelColor(_ledMapping[ledIndex], ws2812b.Color(0, 0, 0)); // it only takes effect if pixels.show() is called
                }
                ledIndex++;
            }
        }
        ws2812b.show();
    }

    bool isConnected(byte OutputPin, byte InputPin)
    {
        // To test whether the pins are connected, set the first as output and the second as input
        pinMode(OutputPin, OUTPUT);
        pinMode(InputPin, INPUT_PULLUP);

        // Set the output pin LOW
        digitalWrite(OutputPin, LOW);

        // If connected, the LOW signal should now be detected on the input pin
        // (Remember, we're using LOW not HIGH, because an INPUT_PULLUP will read HIGH by default)
        bool result = !digitalRead(InputPin);

        // Set the output pin back to its default state
        pinMode(OutputPin, INPUT_PULLUP);

        return result;
    }

    void play()
    {
        if (_hintState == FIRST_TRANSFER || _hintState == SECOND_TRANSFER)
        {
            hint();
            return;
        }

        int resetButtonState = digitalRead(_resetButtonPin);
        if (resetButtonState == LOW)
        {
            reset(false /*global*/);
        }
        int transferButtonState = digitalRead(_transferButtonPin);

        if (!_transferState)
        {
            _fromJug = -1;
            _toJug = -1;
            for (int i = 0; i < _numJugs; i++)
            {
                for (int j = 0; j < _numJugs; j++)
                {
                    if (i == j)
                        continue;
                    if (isConnected(_fillingPins[i], _fillingPins[j]))
                    {
                        _fromJug = j;
                        _toJug = i;
                        break;
                    }
                }
            }
        }

        if ((_fromJug != -1 && _toJug != -1) || _transferState)
        {
            if (_currentValues[_fromJug] > 0 && _currentValues[_toJug] < _capacities[_toJug])
            {
                digitalWrite(_transferPossibleLED, HIGH);
                if (transferButtonState == LOW || _transferState)
                {
                    _transferState = true;
                    transfer(_fromJug, _toJug);
                }
            }
            else
            {
                _transferState = false;
                _fromJug = _toJug = -1;
                digitalWrite(_transferPossibleLED, LOW);
            }
        }
        else
        {
            digitalWrite(_transferPossibleLED, LOW);
        }

        // check if the puzzle is solved and open the door.
        if ((isTransferSolved() && !_transferState) || _solved)
        {
            if (_blinkState)
            {
                blinkJug();
            }
            else
            {
                solve();
            }
        }
    }

    bool isTransferSolved()
    {
        for (int i = 0; i < _numJugs; i++)
        {
            if (_currentValues[i] == _target)
            {
                // Blink the LEDs rapidly in green for 3 seconds
                _targetJug = i;
                return true;
            }
        }
        return false;
    }

    // returns true if the transfer is completed
    bool transfer(int from, int to)
    {
        // calculate how much water can be transferred
        int amountToTransfer = min(_currentValues[from], _capacities[to] - _currentValues[to]);

        // perform the transfer gradually
        unsigned long currentTime = millis();
        if (amountToTransfer > 0 && currentTime - _lastTransferTime >= _transferInterval)
        {
            _lastTransferTime = currentTime;
            _currentValues[from]--;
            _currentValues[to]++;
            updateDisplay();
        }
        if (amountToTransfer <= 0)
            return true;
        return false;
    }

    void blinkJug()
    {
        int ledsOffset = 0;
        for (int i = 0; i < _targetJug; i++)
        {
            ledsOffset += _capacities[i];
        }
        unsigned long currentTime = millis();
        if (currentTime - _lastBlinkTime >= _blinkInterval && _blinkCount > 0)
        {
            _lastBlinkTime = currentTime;
            if (_light)
            {
                for (int i = 0; i < _target; i++)
                {
                    ws2812b.setPixelColor(_ledMapping[i + ledsOffset], ws2812b.Color(0, 25, 0)); // it only takes effect if pixels.show() is called
                }
                ws2812b.show();
            }
            else
            {
                for (int i = 0; i < _target; i++)
                {
                    ws2812b.setPixelColor(_ledMapping[i + ledsOffset], ws2812b.Color(0, 0, 0)); // it only takes effect if pixels.show() is called
                }
                ws2812b.show();
                _blinkCount--;
            }
            _light = !_light;
        }

        if (_blinkCount <= 0)
        {
            _blinkState = false;
            _blinkCount = BLINK_COUNT;
            for (int i = 0; i < _target; i++)
            {
                ws2812b.setPixelColor(_ledMapping[i + ledsOffset], ws2812b.Color(0, 0, 25)); // it only takes effect if pixels.show() is called
            }
            ws2812b.show();
        }
    }

private:
    // transfer state variables
    const unsigned long _transferInterval = 500;
    unsigned long _lastTransferTime;
    bool _transferState;
    int _fromJug;
    int _toJug;

    // blinking state variables
    const unsigned long _blinkInterval = 100;
    int _blinkCount;
    unsigned long _lastBlinkTime;
    int _targetJug;
    bool _blinkState;
    bool _light;

    // connection variables
    const unsigned long _connectInterval = 100;
    unsigned long _lastConnectTime;
    bool _connectState;

    // puzzle state variables
    unsigned long _solveTime;
    bool _solved;
    hintState _hintState;

    // puzzle variables
    const int _numJugs = 3;
    const int _capacities[3] = {8, 5, 3};
    const int _target = 4;
    int _currentValues[3] = {8, 0, 0};

    // Pins and LED indexes
    const byte _relayPin = 12;
    const byte _transferButtonPin = 35;
    const byte _transferPossibleLED = 32;
    const byte _resetButtonPin = 34;
    const byte _fillingPins[3] = {25, 26, 27};
    const int _ledMapping[16] = {0, 1, 2, 3, 4, 5, 6, 7, 12, 11, 10, 9, 8, 13, 14, 15};
};

#endif /* WATER_H */