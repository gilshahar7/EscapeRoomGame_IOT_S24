#ifndef STARS_H
#define STARS_H

#include "globals.h"
#include "utils.h"

/**
 * @class Stars
 * @brief Represents a puzzle in an escape room game.
 * 
 * The Stars class is responsible for managing the puzzle related to stars in an escape room game.
 * It handles the setup, reset, hint, solve, blinkStars, displayPasscodeLeds, and play functions.
 * The class also maintains the state of the puzzle, including the solve time, whether it is solved, whether a hint has been given,
 * whether the correct passcode has been entered, the state of blinking the keypad and stars, and the input string for the passcode.
 * The Stars class uses the ws2812b library to control the color of the stars and keypad LEDs.
 * It also interacts with an MQTT client to publish messages when the puzzle is solved.
 */
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

    /**
     * Solves the puzzle and triggers the corresponding actions.
     * If the puzzle is not yet solved, it sets the solve time, marks it as solved, and activates the relay pin.
     * If the puzzle is already solved and the time since the last solve is greater than or equal to 1000 milliseconds,
     * it updates the solve time, deactivates the relay pin, sets the current stage to SOLVED, and publishes a message to the MQTT client.
     */
    void solve()
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
            mqttClient.publish(ESP_TOPIC, STARS_SOLVE);
        }
    }

    /**
     * @brief Function to blink the stars.
     * 
     * This function blinks the stars by changing their color. It uses the ws2812b library to set the color of the stars.
     * The stars blink at a specific interval defined by _starsBlinkInterval.
     * 
     * @note This function requires the ws2812b library to be included.
     */
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

    /**
     * Displays the passcode LEDs based on the input length.
     * 
     * @param inputLen The length of the input.
     */
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

    /**
     * @brief Plays the game by handling keypad input and checking the passcode.
     * 
     * This function is responsible for handling keypad input and checking the passcode entered by the user.
     * It takes the input from the keypad and updates the passcode accordingly.
     * If the passcode length reaches 4, it checks if the entered passcode matches the correct solution.
     * If the passcode is correct, it sets the `_correctPasscode` flag to true.
     * If the passcode is incorrect, it resets the input string.
     * The function also handles blinking the keypad LEDs based on the `_blinkKeypadState` flag.
     * If the passcode is correct or the game is already solved, it calls the `solve()` function.
     * 
     * @note This function assumes that the `keypad` object and `starSolution` string are properly initialized.
     */
    void play()
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
        else if (_correctPasscode || _solved)
        {
            solve();
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
