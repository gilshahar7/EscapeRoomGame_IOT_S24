#include "../include/globals.h"
#include <Wheels.h>
#include <Water.h>
#include <Stars.h>

Wheels wheels;
Water water;
Stars stars;

/* CODE */
// STARRY NIGHT
// Blink the four star leds. Total time: 8 seconds.
void resetGlobal()
{
    wheels.reset();
    water.reset(true /*global*/);
    stars.reset();

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
        stars.hint();
        // TODO: Think about possible hint
    }
    else if (payloadString.indexOf(STARS_SOLVE) != -1)
    {
        stars.solve(true /*isAdmin*/);
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
    while (!mqttClient.connected() && connectionTries-- > 0)
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (mqttClient.connect("ESP32Client"))
        {
            Serial.println("connected");
            // Subscribe to the admin topic
            mqttClient.subscribe("admin");
            utils::blinkKeypadLedsBlocking(true);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            utils::blinkKeypadLedsBlocking(false);
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
    stars.setup();

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

    // Timer display
    timerCountDown.start(15 * 60); // 15 minutes

    timerDisplay.clear();
}

void loop()
{
    mqttClient.loop();

    // display remaining time
    displayRemainingTime();

    // Blink stars constantly
    stars.blinkStars();

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
        stars.playStarryNight();
        break;
    }
    case SOLVED:
        break;
    }
}
