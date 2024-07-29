#include "../include/globals.h"
#include "../include/Wheels.h"
#include <Water.h>

Wheels wheels;
Water water;

/* CODE */
// Transferring Water

bool blinkStarsisOn = false;
int blinkStarsledNum = 0;

// STARRY NIGHT
// Blink the four star leds. Total time: 8 seconds.
bool blinkStars(void *)
{
    const float colorLow = 0.1;
    if (blinkStarsisOn)
    {
        ws2812b.setPixelColor(blinkingStars[blinkStarsledNum], ws2812b.Color(245 * colorLow, 100 * colorLow, 10 * colorLow)); // it only takes effect if pixels.show() is called
        blinkStarsledNum = (blinkStarsledNum + 1) % 4;
    }
    else
    {
        ws2812b.setPixelColor(blinkingStars[blinkStarsledNum], ws2812b.Color(245, 100, 10)); // it only takes effect if pixels.show() is called
    }
    ws2812b.show();
    blinkStarsisOn = !blinkStarsisOn;

    return true; // repeat
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

void blinkPassword(bool correct)
{
    for (int j = 0; j < 5; j++)
    {
        for (int i = 0; i < numKeypadLeds; i++)
        {
            ws2812b.setPixelColor(keypadLeds[i], ws2812b.Color(correct ? 0 : 25, correct ? 25 : 0, 0)); // it only takes effect if pixels.show() is called
        }
        ws2812b.show();
        delay(100);

        for (int i = 0; i < numKeypadLeds; i++)
        {
            ws2812b.setPixelColor(keypadLeds[i], ws2812b.Color(0, 0, 0)); // it only takes effect if pixels.show() is called
        }
        ws2812b.show();
        delay(100);
    }
    for (int i = 0; i < numKeypadLeds; i++)
    {
        ws2812b.setPixelColor(keypadLeds[i], ws2812b.Color(0, 0, 0)); // it only takes effect if pixels.show() is called
    }
    ws2812b.show();
}

void solveStars(bool isAdmin)
{
    digitalWrite(relayPin3, HIGH);
    delay(1000);
    digitalWrite(relayPin3, LOW);
    currentStage = SOLVED;
    if (!isAdmin)
        mqttClient.publish(ESP_TOPIC, STARS_SOLVE);
    Serial.print("solved stars");
}

void playStarryNight()
{
    char keys[] = "123 456 789 *0# N";
    uint8_t index = keypad.getKey();
    unsigned long currentTime = millis();

    if (currentTime - keypadLastDebounceTime > 50)
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
                if (inputString == starSolution)
                {
                    blinkPassword(true /*correct*/);
                    solveStars(false /*isAdmin*/);
                }
                else
                {
                    inputString = "";
                    blinkPassword(false /*correct*/);
                }
            }
        }
        keypadLastDebounceTime = currentTime;
        prevKeyIndex = index;
    }
}

void resetGlobal()
{
    wheels.reset();
    water.reset(true /*global*/);

    isStarHintGiven = false;

    currentStage = WHEELS;
}

// Wifi and MQTT functions
void callback(char *topic, byte *payload, unsigned int length)
{
    const String payloadString = String((char *)payload);
    Serial.println(payloadString);
    if (payloadString.indexOf(WHEELS_HINT) != -1)
    {
        wheels.hint();
    }
    else if (payloadString.indexOf(WHEELS_SOLVE) != -1)
    {
        wheels.solve(true /*isAdmin*/);
    }
    else if (payloadString.indexOf(WATER_RESET) != -1)
    {
        water.reset(false /*global*/);
    }
    else if (payloadString.indexOf(WATER_HINT) != -1)
    {
        water.hint();
    }
    else if (payloadString.indexOf(WATER_SOLVE) != -1)
    {
        water.solve(true /*isAdmin*/);
    }
    else if (payloadString.indexOf(STARS_HINT) != -1)
    {
        isStarHintGiven = true;
        // TODO: Think about possible hint
    }
    else if (payloadString.indexOf(STARS_SOLVE) != -1)
    {
        solveStars(true /*isAdmin*/);
    }
    else if (payloadString.indexOf(GLOBAL_RESET) != -1)
    {
        resetGlobal();
    }
}

void setup_wifi()
{
    Serial.println();
    Serial.print("Setting up AP: ");
    Serial.println(ssid);

    // Set the ESP32 wifi to connect to hotspot
    WiFi.begin(ssid, password);
    // Set the ESP32 as an access point
    // WiFi.softAP(ssid, password);

    // Print the IP address of the access point
    // Serial.print("AP IP address: ");
    // Serial.println(WiFi.softAPIP());
}

void connect_to_mqtt()
{
    int tries = 3;
    while (!mqttClient.connected() && tries-- > 0)
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (mqttClient.connect("ESP32Client"))
        {
            Serial.println("connected");
            // Subscribe to the admin topic
            mqttClient.subscribe("admin");
            blinkPassword(true);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            blinkPassword(false);
        }
    }
}

void displayRemainingTime()
{
    timerDisplay.loop();
    const int MINUTE = 60;

    uint32_t remainingSeconds = timerCountDown.remaining();
    uint32_t second = remainingSeconds % MINUTE;
    uint32_t minute = remainingSeconds / MINUTE;

    const uint32_t timerDigits[] = {minute / 10, minute % 10, second / 10, second % 10};

    timerDisplay.clear();
    for (int i = 0; i < 4; ++i)
    {
        timerDisplay.setNumber(i + 1, timerDigits[i]);
    }
    timerDisplay.setDot(2);
    timerDisplay.show();
}

/* Main Code */
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);

    pinMode(ledsPin, OUTPUT); // transferring water leds + wheels hint led + starry night leds + keypad leds
    ws2812b.begin();

    wheels.setup();
    water.setup();

    // Starry night
    pinMode(relayPin3, OUTPUT);
    digitalWrite(relayPin3, LOW);

    blinkTimer.every(1000, blinkStars); // Blink all stars every 9 seconds

    // Timer display
    timerCountDown.start(15 * 60); // 15 minutes

    timerDisplay.clear();

    // Initialize Keypad
    Wire.begin();
    Wire.setClock(400000);
    if (keypad.begin() == false)
    {
        Serial.println("\nERROR: cannot communicate to keypad.\n");
    }

    currentStage = WHEELS;

    // connect to wifi
    setup_wifi();

    // set MQTT server and callback function
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(callback);

    // Connect to MQTT broker
    connect_to_mqtt();
}

void loop()
{
    mqttClient.loop();

    // display remaining time
    displayRemainingTime();

    // Blink stars constantly
    blinkTimer.tick();

    switch (currentStage)
    {
    case WHEELS:
    {
        wheels.checkSolved();
        break;
    }
    case WATER:
    {
        water.playTransferWater();
        break;
    }
    case STARS:
    {
        playStarryNight();
        break;
    }
    case SOLVED:
        break;
    }

    // delay(10); // this speeds up the simulation
}
