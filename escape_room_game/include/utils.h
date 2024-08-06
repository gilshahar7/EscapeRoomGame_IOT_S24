#ifndef UTILS_H
#define UTILS_H

#include "globals.h"

#define BLINKS 5

namespace utils
{

    unsigned long lastBlinkTime = 0;
    const unsigned long blinkInterval = 100;
    int blinkCount = BLINKS;
    bool light = true;

    void setKeyPadLEDColors(int r, int g, int b)
    {
        for (int i = 0; i < numKeypadLeds; i++)
        {
            ws2812b.setPixelColor(keypadLeds[i], ws2812b.Color(r, g, b)); // it only takes effect if pixels.show() is called
        }
        ws2812b.show();
    }

    bool blinkKeypadLeds(bool correct)
    {
        unsigned long currentTime = millis();
        if (currentTime - lastBlinkTime >= blinkInterval && blinkCount > 0)
        {
            lastBlinkTime = currentTime;
            if (light)
            {
                setKeyPadLEDColors(correct ? 0 : 25, correct ? 25 : 0, 0);
            }
            else
            {
                setKeyPadLEDColors(0, 0, 0);
                blinkCount--;
            }
            light = !light;
        }

        if (blinkCount <= 0)
        {
            blinkCount = BLINKS;
            setKeyPadLEDColors(0, 0, 0);
            return false;
        }

        return true;
    }

    void blinkKeypadLedsBlocking(bool correct)
    {
        for (int j = 0; j < 5; j++)
        {
            setKeyPadLEDColors(correct ? 0 : 25, correct ? 25 : 0, 0);
            delay(100);

            setKeyPadLEDColors(0, 0, 0);
            delay(100);
        }
        setKeyPadLEDColors(0, 0, 0);
    }
}

#endif /* UTILS_H */
