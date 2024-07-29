#ifndef UTILS_H
#define UTILS_H

#include "globals.h"

#define BLINKS 5

namespace utils {

    unsigned long lastBlinkTime = 0;
    const unsigned long blinkInterval = 100;
    int blinkCount = BLINKS;
    bool light = true;

    bool blinkKeypadLeds(bool correct)
    {
        unsigned long currentTime = millis();
        if (currentTime - lastBlinkTime >= blinkInterval && blinkCount > 0) {
            lastBlinkTime = currentTime;
            if (light) {
                for (int i = 0; i < numKeypadLeds; i++) {
                    ws2812b.setPixelColor(keypadLeds[i], ws2812b.Color(correct ? 0 : 25, correct ? 25 : 0, 0)); // it only takes effect if pixels.show() is called
                }
                ws2812b.show();
            } else {
                for (int i = 0; i < numKeypadLeds; i++) {
                    ws2812b.setPixelColor(keypadLeds[i], ws2812b.Color(0, 0, 0)); // it only takes effect if pixels.show() is called
                }
                ws2812b.show();
                blinkCount--;
            }
            light = !light;
        }
        
        if (blinkCount <= 0) {
            blinkCount = BLINKS;
            for (int i = 0; i < numKeypadLeds; i++)
            {
                ws2812b.setPixelColor(keypadLeds[i], ws2812b.Color(0, 0, 0)); // it only takes effect if pixels.show() is called
            }
            ws2812b.show();
            return false;
        }

        return true;
    }

    void blinkKeypadLedsBlocking(bool correct) {
        for(int j = 0; j < 5; j++) {
            for(int i = 0; i < numKeypadLeds; i++) {
                ws2812b.setPixelColor(keypadLeds[i], ws2812b.Color(correct ? 0 : 25, correct ? 25 : 0, 0));  // it only takes effect if pixels.show() is called
            }
            ws2812b.show();
            delay(100);

            for(int i = 0; i < numKeypadLeds; i++) {
                ws2812b.setPixelColor(keypadLeds[i], ws2812b.Color(0, 0, 0));  // it only takes effect if pixels.show() is called
            }
            ws2812b.show();
            delay(100);
        }
        for(int i = 0; i < numKeypadLeds; i++) {
            ws2812b.setPixelColor(keypadLeds[i], ws2812b.Color(0, 0, 0));  // it only takes effect if pixels.show() is called
        }
        ws2812b.show();
    }
}

#endif /* UTILS_H */
