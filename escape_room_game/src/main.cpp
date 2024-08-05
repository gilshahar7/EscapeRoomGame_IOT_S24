#include "../include/globals.h"
#include <Wheels.h>
#include <Fuel.h>
#include <Stars.h>

Wheels wheels;
Fuel fuel;
Stars stars;

/* CODE */
// STARRY NIGHT
// Blink the four star leds. Total time: 8 seconds.
void resetGlobal()
{
    wheels.reset();
    fuel.reset(true /*global*/);
    stars.reset();

    currentStage = READY;
}

// Wifi and MQTT functions
void callback(char *topic, byte *payload, unsigned int length)
{
    const String payloadString = String((char *)payload);
    Serial.println(payloadString);
    if (payloadString.indexOf(START_GAME) != -1)
    {
        timerCountDown.start(gameDuration);
        currentStage = WHEELS;
    }
    else if (payloadString.indexOf(WHEELS_HINT) != -1)
    {
        wheels.hint();
    }
    else if (payloadString.indexOf(WHEELS_SOLVE) != -1)
    {
        wheels.solve();
    }
    else if (payloadString.indexOf(FUEL_RESET) != -1)
    {
        fuel.reset(false /*global*/);
    }
    else if (payloadString.indexOf(FUEL_HINT) != -1)
    {
        fuel.hint();
    }
    else if (payloadString.indexOf(FUEL_SOLVE) != -1)
    {
        fuel.solve();
    }
    else if (payloadString.indexOf(STARS_HINT) != -1)
    {
        stars.hint();
        // TODO: Think about possible hint
    }
    else if (payloadString.indexOf(STARS_SOLVE) != -1)
    {
        stars.solve();
    }
    else if (payloadString.indexOf(GLOBAL_RESET) != -1)
    {
        resetGlobal();
    }
    else if (payloadString.indexOf(ADD_MIN) != -1)
    {
        gameDuration += 60;
    }
    else if (payloadString.indexOf(SUB_MIN) != -1)
    {
        if (gameDuration > 60)
            gameDuration -= 60;
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

/**
 * @brief Displays the remaining time of the game.
 * 
 * This function calculates the remaining time of the game and displays it on the timer display.
 * If the current stage is SOLVED, the function returns without doing anything.
 * The remaining time is calculated based on the current stage and the game duration.
 * The time is then converted into a string format and published to the MQTT client.
 * The time string is also modified to add dots between the digits for better readability.
 * Finally, the time string is displayed on the timer display with maximum brightness.
 */
void displayRemainingTime()
{
    if (currentStage == SOLVED) {
        return;
    }

    const unsigned long currentTime = millis();

    // Display remaining time
    const int MINUTE = 60;
    uint32_t remainingSeconds;

    if (currentStage == READY)
        remainingSeconds = gameDuration;
    else
        remainingSeconds = timerCountDown.remaining();

    uint32_t second = remainingSeconds % MINUTE;
    uint32_t minute = remainingSeconds / MINUTE;

    std::string strTime = std::to_string(minute / 10) + std::to_string(minute % 10) + std::to_string(second / 10) + std::to_string(second % 10);

    if (currentTime - lastTimerPublished > 500) {
        lastTimerPublished = currentTime;
        strTime.insert(2, ":");
        mqttClient.publish(ESP_TIMER_TOPIC, strTime.c_str());
    }

    char *timeString = const_cast<char*>(strTime.c_str());
    timeString[1] |= 0x80; // Add dots
    
    timerDisplay.displayOn();
    timerDisplay.displayString(timeString);
    timerDisplay.setBrightness(TM1650_MAX_BRIGHT);
}

/**
 * @brief Handles the reset and start functionality.
 * 
 * This function is responsible for handling the reset and start functionality in the escape room game.
 * It checks for key presses on the keypad and performs the corresponding actions based on the pressed key.
 * If the '*' key is pressed, it starts the countdown timer and sets the current stage to WHEELS.
 * If the '#' key is pressed, it resets the game globally and publishes a reset message to the MQTT client.
 */
void handleResetStart()
{
    char keys[] = "123 456 789 *0# N";
    uint8_t index = keypad.getKey();

    if (keys[prevKeyIndex] == 'N' && keys[index] != 'N')
    { // N = Not pressed
        char key = keys[index];
        if (key == '*')
        {
            timerCountDown.start(gameDuration);
            currentStage = WHEELS;
        }

        if (key == '#') {
            resetGlobal();
            mqttClient.publish(ESP_TOPIC, GLOBAL_RESET);
        }
    }
}

/* Main Code */
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);

    pinMode(ledsPin, OUTPUT); // transferring fuel leds + wheels hint led + starry night leds + keypad leds
    ws2812b.begin();

    wheels.setup();
    fuel.setup();
    stars.setup();

    // Initialize Keypad
    Wire.begin();
    Wire.setClock(400000);
    if (keypad.begin() == false)
    {
        Serial.println("\nERROR: cannot communicate to keypad.\n");
    }

    currentStage = READY;

    // connect to wifi
    setup_wifi();

    // set MQTT server and callback function
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(callback);

    // Connect to MQTT broker
    connect_to_mqtt();

    mqttClient.publish(ESP_TOPIC, GLOBAL_RESET);
    mqttClient.publish(ESP_TIMER_TOPIC, "15:00");

    // Timer display
    timerDisplay.init();
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
    case READY:
    {
        handleResetStart();
        break;
    }
    case WHEELS:
    {
        wheels.play();
        break;
    }
    case FUEL:
    {
        fuel.play();
        break;
    }
    case STARS:
    {
        stars.play();
        break;
    }
    case SOLVED:
        handleResetStart();
        break;
    }
}
