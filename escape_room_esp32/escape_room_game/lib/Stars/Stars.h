#ifndef STARS_H
#define STARS_H

#include "globals.h"
#include "utils.h"

class Stars
{
public:
    Stars() : _solveTime(0),
              _solved(false),
              _hintGiven(false),
              _correctPasscode(false),
              _blinkKeypadState(false),
              _lastBlinkStarsTime(0),
              _blinkStarsledNum(0),
              _blinkStars(false),
              _blinkPause(0)
    {
    }
    void setup()
    {
        pinMode(_relayPin, OUTPUT);
        digitalWrite(_relayPin, LOW);
    }

    void reset()
    {
        _correctPasscode = false;
        _blinkKeypadState = false;
        _hintGiven = false;
        _solved = false;
    }

    void hint()
    {
        _hintGiven = true;
    }

    void solve(bool isAdmin)
    {
        unsigned long currentTime = millis();
        if (!_solved)
        {
            _solveTime = millis();
            _solved = true;
            digitalWrite(_relayPin, HIGH);
        }
        else if (currentTime - _solveTime >= 1000)
        {
            _solveTime = currentTime;
            digitalWrite(_relayPin, LOW);
            currentStage = SOLVED;
            if (!isAdmin)
                mqttClient.publish(ESP_TOPIC, STARS_SOLVE);
            Serial.println("solved wheels");
        }
    }

    void blinkStars()
    {
        unsigned long currentTime = millis();
        const float colorLow = 0.1;
        if (currentTime - _lastBlinkStarsTime >= _starsBlinkInterval)
        {
            _lastBlinkStarsTime = currentTime;
            if (_blinkPause < 8)
            {
                if (_blinkStars)
                {
                    ws2812b.setPixelColor(_blinkingStars[_blinkStarsledNum], ws2812b.Color(245 * colorLow, 100 * colorLow, 10 * colorLow)); // it only takes effect if pixels.show() is called
                    _blinkStarsledNum = (_blinkStarsledNum + 1) % 4;
                }
                else
                {
                    ws2812b.setPixelColor(_blinkingStars[_blinkStarsledNum], ws2812b.Color(245, 100, 10)); // it only takes effect if pixels.show() is called
                }
                ws2812b.show();
                _blinkStars = !_blinkStars;
                _blinkPause++;
            }
            else
            {
                _blinkPause = 0;
            }
        }
    }

    void displayPasscodeLeds(int inputLen)
    {
        for (int i = 0; i < numKeypadLeds; ++i)
        {
            if (i < inputLen)
            {
                ws2812b.setPixelColor(keypadLeds[i], ws2812b.Color(25, 0, 0)); // it only takes effect if pixels.show() is called
            }
            else
            {
                ws2812b.setPixelColor(keypadLeds[i], ws2812b.Color(0, 0, 0)); // it only takes effect if pixels.show() is called
            }
            ws2812b.show();
        }
    }

    void playStarryNight()
    {
        char keys[] = "123 456 789 *0# N";
        uint8_t index = keypad.getKey();
        unsigned long currentTime = millis();

        if (!_blinkKeypadState && !_solved && currentTime - keypadLastDebounceTime > 50)
        {
            if (keys[prevKeyIndex] == 'N' && keys[index] != 'N')
            { // N = Not pressed
                char key = keys[index];
                if (key)
                {
                    inputString += key;
                    displayPasscodeLeds(inputString.length());
                }
                if (inputString.length() >= 4)
                {
                    _blinkKeypadState = true;
                    if (inputString == starSolution)
                    {
                        _correctPasscode = true;
                    }
                    else
                    {
                        inputString = "";
                    }
                }
            }
            keypadLastDebounceTime = currentTime;
            prevKeyIndex = index;
        }

        if (_blinkKeypadState)
        {
            _blinkKeypadState = utils::blinkKeypadLeds(_correctPasscode);
        }
        else if (_correctPasscode)
        {
            solve(false /*isAdmin*/);
        }
    }

private:
    unsigned long _solveTime;
    bool _solved;
    bool _hintGiven;

    bool _correctPasscode;
    bool _blinkKeypadState;

    const unsigned long _starsBlinkInterval = 1000;
    unsigned long _lastBlinkStarsTime;
    int _blinkStarsledNum;
    bool _blinkStars;
    int _blinkPause;

    String inputString;
    const String starSolution = "7031";
    const int _blinkingStars[4] = {16, 17, 18, 19};
    const byte _relayPin = 14;
};

#endif /* STARS_H */
