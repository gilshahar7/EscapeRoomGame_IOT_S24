#ifndef COMPARTMENT_H
#define COMPARTMENT_H

#include "globals.h"

class Compartment
{
public:
    Compartment(const byte relayPin): _relayPin(relayPin), _lastOpenedTime(0) {}

    void handle()
    {
        if (_lastOpenedTime != 0) {
            uint64_t timeDiff = esp_timer_get_time() - _lastOpenedTime;
            if (timeDiff < 0.5 * 1e6) {
                digitalWrite(_relayPin, HIGH);
            }
            else if (timeDiff > 0.5 * 1e6 && timeDiff < 0.7 * 1e6) {
                digitalWrite(_relayPin, LOW);
            }
            else if (timeDiff > 0.7 * 1e6 && timeDiff < 1.2 * 1e6) {
                digitalWrite(_relayPin, HIGH);
            }
            else if (timeDiff > 1.2 * 1e6) {
                digitalWrite(_relayPin, LOW);
                _lastOpenedTime = 0;
            }
        }
    }

    void open()
    {
        _lastOpenedTime = esp_timer_get_time();
    }

private:
    const uint64_t _intervalOn = 0.5 * 1e6;
    const uint64_t _intervalOff = 0.2 * 1e6;
    uint64_t _lastOpenedTime;
    byte _relayPin;
};

#endif /* COMPARTMENT_H */
