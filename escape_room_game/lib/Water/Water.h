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

    /**
     * Solves the water puzzle.
     * 
     * This function is responsible for solving the water puzzle. It checks if the puzzle has already been solved,
     * and if not, it sets the solve time, marks the puzzle as solved, turns off the relay, and updates the current stage.
     * If the puzzle has already been solved, it checks if enough time has passed since the last solve, and if so,
     * it updates the solve time, turns on the relay, updates the blink state, updates the current stage, and publishes
     * a message to the MQTT client.
     */
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

    /**
     * @brief Updates the display of the water jugs.
     * 
     * This function updates the display of the water jugs by setting the color of each LED
     * based on the current water level in each jug. The LEDs are controlled using the ws2812b library.
     * The LEDs are lit up with a blue color for the water level and turned off for the empty space.
     * The changes take effect only when the `ws2812b.show()` function is called.
     */
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

    /**
     * The `play` function is responsible for controlling the gameplay logic of the water puzzle in the escape room game.
     * It checks the state of the hint, reset button, transfer button, and the connection between jugs.
     * If the hint state is in the first or second transfer, it calls the `hint` function.
     * If the reset button is pressed, it calls the `reset` function with the `global` parameter set to false.
     * If the transfer button is pressed or the transfer state is true, it transfers water from one jug to another using the `transfer` function.
     * It checks if the transfer is possible based on the current values and capacities of the jugs.
     * If the puzzle is solved, it either blinks the jug or calls the `solve` function to open the door.
     */
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

    /**
     * Transfers water from one container to another.
     * 
     * @param from The index of the container to transfer water from.
     * @param to The index of the container to transfer water to.
     * @return True if the transfer is complete, false otherwise.
     */
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

    /**
     * @brief Blinks the jug LEDs based on the target jug level.
     * 
     * This function calculates the offset of the LEDs based on the target jug level and blinks the LEDs accordingly.
     * It uses the millis() function to determine the current time and checks if enough time has passed to blink the LEDs.
     * The LEDs are set to a specific color depending on the light state.
     * If the blink count reaches zero, the LEDs are set to a different color and the blink state is set to false.
     * 
     * @note This function requires the ws2812b library to control the LEDs.
     */
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
