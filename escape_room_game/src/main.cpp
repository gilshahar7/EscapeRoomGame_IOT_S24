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

    mqttClient->publish(ESP_TOPIC, GLOBAL_RESET);

    currentStage = READY;
}

void handleCompartments()
{
    wheels.compartment.handle();
    fuel.compartment.handle();
    stars.compartment.handle();
}

static std::pair<uint32_t, uint32_t> calcTimePassed()
{
    const int MINUTE = 60;
    uint32_t passedSeconds = gameDuration - timerCountDown.remaining();

    uint32_t second = passedSeconds % MINUTE;
    uint32_t minute = passedSeconds / MINUTE;

    return std::make_pair(minute, second);
}

static std::pair<uint32_t, uint32_t> calcRemainingTime()
{
    const int MINUTE = 60;
    uint32_t remainingSeconds;

    if (currentStage == READY)
        remainingSeconds = gameDuration;
    else
        remainingSeconds = timerCountDown.remaining();

    uint32_t second = remainingSeconds % MINUTE;
    uint32_t minute = remainingSeconds / MINUTE;

    return std::make_pair(minute, second);
}

inline static std::string formatTime(uint32_t minute, uint32_t second)
{
    return std::to_string(minute / 10) + std::to_string(minute % 10) + ":" + std::to_string(second / 10) + std::to_string(second % 10);
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
    }
    else if (payloadString.indexOf(STARS_SOLVE) != -1)
    {
        stars.solve();
        auto [minute, second] = calcTimePassed();
        mqttClient->publish(ESP_COMPLETION_TOPIC, formatTime(minute, second).c_str());   
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
    else if (payloadString.indexOf(COMPARTMENT_OPEN1) != -1)
    {
        wheels.compartment.open();
    }
    else if (payloadString.indexOf(COMPARTMENT_OPEN2) != -1)
    {
        fuel.compartment.open();
    }
    else if (payloadString.indexOf(COMPARTMENT_OPEN3) != -1)
    {
        stars.compartment.open();
    }
}

void setup_wifi()
{
    WiFiManager wifiManager;
    if (!wifiManager.autoConnect("escape_room_game_AP")) {
        Serial.println("Failed to connect and hit timeout");
    }
    Serial.println("Connected to WiFi!");

    if (!MDNS.begin("esp32")) {
        Serial.println("Error setting up MDNS responder!");
    }
}

void connect_to_mqtt()
{
    MDNS.addService("mqtt", "tcp", mqtt_port);

    int mqttBrokerAddress = MDNS.queryService("mqtt", "tcp");

    if (mqttBrokerAddress == 0) {
        Serial.println("MQTT service not found.");
        utils::blinkKeypadLedsBlocking(false);
    } else {
        mqtt_ip = MDNS.IP(mqttBrokerAddress-1);
        utils::setKeyPadLEDColors(0, 0, 255);
    }
    Serial.println(mqtt_ip);

    mqttClient = new PubSubClient(mqtt_ip, mqtt_port, espClient);
    // set MQTT server and callback function
    // mqttClient->setServer(mqtt_ip, mqtt_port);
    mqttClient->setCallback(callback);

    while (!mqttClient->connected() && connectionTries-- > 0 && mqttBrokerAddress != 0)
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (mqttClient->connect("ESP32Client"))
        {
            Serial.println("connected");
            // Subscribe to the admin topic
            mqttClient->subscribe("admin");
            utils::blinkKeypadLedsBlocking(true);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqttClient->state());
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
    auto [minute, second] = calcRemainingTime();

    std::string strTime = formatTime(minute, second);

    if (currentTime - lastTimerPublished > 500) {
        lastTimerPublished = currentTime;
        mqttClient->publish(ESP_TIMER_TOPIC, strTime.c_str());
    }
    
    timerDisplay.displayTime(minute, second);
}

/**
 * @brief Handles the reset and start functionality.
 * 
 * This function is responsible for handling the reset and start functionality in the escape room game.
 * It checks for key presses on the keypad and performs the corresponding actions based on the pressed key.
 * If the '*' key is pressed, it starts the countdown timer and sets the current stage to WHEELS.
 * If the '#' key is pressed, it resets the game globally and publishes a reset message to the MQTT client.
 */
void handleKeypadInput()
{
    char keys[] = "123 456 789 *0# N";
    uint8_t index = keypad.getKey();

    if (keys[prevKeyIndex] == 'N' && keys[index] != 'N')
    { // N = Not pressed
        char key = keys[index];
        if (key == '*')
        {
            resetGlobal();
            timerCountDown.start(gameDuration);
            currentStage = WHEELS;
        }

        if (key == '#') {
            resetGlobal();
        }

        if (key == '1') {
            fuel.compartment.open();
        }

        if (key == '2') {
            stars.compartment.open();
        }

        if (key == '3') {
            wheels.compartment.open();
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
    
    if (!MDNS.begin("esp32")) {
        Serial.println("Cannot start MDNS.");
    }

    // Connect to MQTT broker
    connect_to_mqtt();

    mqttClient->publish(ESP_TOPIC, GLOBAL_RESET);
    mqttClient->publish(ESP_TIMER_TOPIC, "15:00");

    // Timer display
    timerDisplay.begin();
    timerDisplay.displayOn();
    timerDisplay.setDigits(4);
}

void loop()
{
    mqttClient->loop();

    // display remaining time
    displayRemainingTime();

    // handle compartments
    handleCompartments();

    // Blink stars constantly
    stars.blinkStars();

    switch (currentStage)
    {
    case READY:
    {
        handleKeypadInput();
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
        if (currentStage == SOLVED) {
            auto [minute, second] = calcTimePassed();
            mqttClient->publish(ESP_COMPLETION_TOPIC, formatTime(minute, second).c_str());   
        }
        break;
    }
    case SOLVED:
        handleKeypadInput();
        break;
    }
}
