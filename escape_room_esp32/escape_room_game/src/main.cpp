#include "../include/globals.h"
#include "../include/Wheels.h"

Wheels wheels;

/* CODE */
// Transferring Water
void updateDisplayWater() {
  int ledIndex = 0;
  for (int jug = 0; jug < numJugs; jug++){
    for (int i = 0; i < capacities[jug]; i++){
      if (i < currentValues[jug]){
        ws2812b.setPixelColor(ledMapping[ledIndex], ws2812b.Color(0, 0, 25));  // it only takes effect if pixels.show() is called
      } else {
        ws2812b.setPixelColor(ledMapping[ledIndex], ws2812b.Color(0, 0, 0));  // it only takes effect if pixels.show() is called
      }
      ledIndex++;
    }
  }
  ws2812b.show();   
}

void blinkJug(int jug) {
  int ledsOffset = 0;
  for(int i=0; i<jug; i++) {
    ledsOffset += capacities[i];
  }
  for(int j = 0; j < 10; j++) {
    for(int i = 0; i < target; i++) {
      ws2812b.setPixelColor(ledMapping[i+ledsOffset], ws2812b.Color(0, 25, 0));  // it only takes effect if pixels.show() is called
    }
    ws2812b.show();
    delay(100);

    for(int i = 0; i < target; i++) {
      ws2812b.setPixelColor(ledMapping[i+ledsOffset], ws2812b.Color(0, 0, 0));  // it only takes effect if pixels.show() is called
    }
    ws2812b.show();
    delay(100);
  }
  for(int i = 0; i < target; i++) {
    ws2812b.setPixelColor(ledMapping[i+ledsOffset], ws2812b.Color(0, 0, 25));  // it only takes effect if pixels.show() is called
  }
  ws2812b.show();
}

bool isTransferSolved() {
  for(int i=0; i<numJugs; i++){
    if(currentValues[i] == target){
      // Blink the LEDs rapidly in green for 3 seconds
      blinkJug(i);
      return true;
    }
  }
  return false;
}

void solveTransferringWater(bool isAdmin) {
  digitalWrite(relayPin2, HIGH);
  delay(1000);
  digitalWrite(relayPin2, LOW);
  Serial.print("Solved Transferring Water");
  currentStage = STARS;
  if (!isAdmin)
    mqttClient.publish(ESP_TOPIC, WATER_SOLVE);
  digitalWrite(transferPossibleLED, LOW);
}

void transfer(int from,int to){
  Serial.print("Transferring from ");
  Serial.print(from);
  Serial.print(" to ");
  Serial.println(to);

  //calculate how much water can be transferred
  int amountToTransfer = min(currentValues[from], capacities[to] - currentValues[to]);

  // perform the transfer (instantly)
  // currentValues[from] -= amountToTransfer;
  // currentValues[to] += amountToTransfer;
  // updateDisplay();

  // or, perform the transfer gradually
  int tragetValueFrom = currentValues[from] - amountToTransfer;
  int tragetValueTo = currentValues[to] + amountToTransfer;
  for(int i = 0; i < amountToTransfer; i++){
    currentValues[from]--;
    currentValues[to]++;
    updateDisplayWater();
    // speed of pouring
    delay(500);
  }

  // check if the puzzle is solved and open the door.
  if (isTransferSolved()){
    solveTransferringWater(false/*isAdmin*/);
  }
}

bool isConnected(byte OutputPin, byte InputPin) {
  // To test whether the pins are connected, set the first as output and the second as input
  pinMode(OutputPin, OUTPUT);
  pinMode(InputPin, INPUT_PULLUP);

  // Set the output pin LOW
  digitalWrite(OutputPin, LOW);
  delay(10);

  // If connected, the LOW signal should now be detected on the input pin
  // (Remember, we're using LOW not HIGH, because an INPUT_PULLUP will read HIGH by default)
  bool isConnected = !digitalRead(InputPin);

  // Set the output pin back to its default state
  pinMode(OutputPin, INPUT_PULLUP);

  return isConnected;
}

void resetTransferringWater(){
  currentValues[0] = isWaterHintGiven? 3 : 8;
  currentValues[1] = isWaterHintGiven? 2 : 0;
  currentValues[2] = isWaterHintGiven? 3 : 0;
  updateDisplayWater();
}

void playTransferWater() {
  int resetButtonState = digitalRead(transferringWaterResetButtonPin);
  if (resetButtonState == LOW) {
    mqttClient.publish(ESP_TOPIC, WATER_RESET);
    resetTransferringWater();
  }
  int transferButtonState = digitalRead(transferButtonPin);
  int fromJug = -1, toJug = -1;
  for (int i=0; i<numJugs; i++) {
    for (int j=0; j<numJugs; j++) {
      if (i == j)
        continue;
      if (isConnected(fillingPins[i], fillingPins[j])) {
        fromJug = j;
        toJug = i;
        break;
      }
    }
  }
  if (fromJug != -1 && toJug != -1) {
    if (currentValues[fromJug] > 0 && currentValues[toJug] < capacities[toJug]){
      digitalWrite(transferPossibleLED, HIGH);
      if (transferButtonState == LOW) {
        transfer(fromJug, toJug);
      }
    } else {
      digitalWrite(transferPossibleLED, LOW);
    }
  } else {
    digitalWrite(transferPossibleLED, LOW);
  }
}

bool blinkStarsisOn = false;
int blinkStarsledNum = 0;

// STARRY NIGHT
// Blink the four star leds. Total time: 8 seconds.
bool blinkStars(void *) {
  const float colorLow = 0.1;
  if (blinkStarsisOn) {
    ws2812b.setPixelColor(blinkingStars[blinkStarsledNum], ws2812b.Color(245*colorLow, 100*colorLow, 10*colorLow));  // it only takes effect if pixels.show() is called
    blinkStarsledNum = (blinkStarsledNum + 1) % 4;
  } else {
    ws2812b.setPixelColor(blinkingStars[blinkStarsledNum], ws2812b.Color(245, 100, 10));  // it only takes effect if pixels.show() is called
  }
  ws2812b.show();
  blinkStarsisOn = !blinkStarsisOn;

  return true; // repeat
}

void displayPasscodeLeds(int inputLen) {
  for(int i=0; i<numKeypadLeds; ++i) {
    if (i < inputLen) {
      ws2812b.setPixelColor(keypadLeds[i], ws2812b.Color(25, 0, 0));  // it only takes effect if pixels.show() is called
    } else {
      ws2812b.setPixelColor(keypadLeds[i], ws2812b.Color(0, 0, 0));  // it only takes effect if pixels.show() is called
    }
    ws2812b.show();
  }
}

void blinkPassword(bool correct) {
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

void solveStars(bool isAdmin) {
  digitalWrite(relayPin3, HIGH);
  delay(1000);
  digitalWrite(relayPin3, LOW);
  currentStage = SOLVED;
  if (!isAdmin)
    mqttClient.publish(ESP_TOPIC, STARS_SOLVE);
  Serial.print("solved stars");
}

void playStarryNight() {
  char keys[] = "123 456 789 *0# N";
  uint8_t index = keypad.getKey();
  unsigned long currentTime = millis();

  if (currentTime - keypadLastDebounceTime > 50) {
    if (keys[prevKeyIndex] == 'N' && keys[index] != 'N') { // N = Not pressed
      char key = keys[index];
      if (key) {
        inputString += key;
        displayPasscodeLeds(inputString.length());
      }
      if (inputString.length() >= 4) {
        if(inputString == starSolution) {
          blinkPassword(true/*correct*/);
          solveStars(false/*isAdmin*/);
        } else {
          inputString = "";
          blinkPassword(false/*correct*/);
        }
      }
    }
    keypadLastDebounceTime = currentTime;
    prevKeyIndex = index;
  }
}


void resetGlobal() {
  wheels.reset();

  isWaterHintGiven = false;
  resetTransferringWater();

  isStarHintGiven = false;

  currentStage = WHEELS;
}

// Wifi and MQTT functions
void callback(char* topic, byte* payload, unsigned int length) {
  const String payloadString = String((char*)payload);
  Serial.println(payloadString);
  if (payloadString.indexOf(WHEELS_HINT) != -1) {
    wheels.hint();
  } else if (payloadString.indexOf(WHEELS_SOLVE) != -1) {
    wheels.solve(true/*isAdmin*/);
  } else if (payloadString.indexOf(WATER_RESET) != -1) {
    resetTransferringWater();
  } else if (payloadString.indexOf(WATER_HINT) != -1) {
    resetTransferringWater();
    transfer(0, 1); // 8, 0, 0 -> 3, 5, 0
    transfer(1, 2); // 3, 5, 0 -> 3, 2, 3
    isWaterHintGiven = true;
  } else if (payloadString.indexOf(WATER_SOLVE) != -1) {
    solveTransferringWater(true/*isAdmin*/);
  } else if (payloadString.indexOf(STARS_HINT) != -1) {
    isStarHintGiven = true;
    // TODO: Think about possible hint
  } else if (payloadString.indexOf(STARS_SOLVE) != -1) {
    solveStars(true/*isAdmin*/);
  } else if (payloadString.indexOf(GLOBAL_RESET) != -1) {
    resetGlobal();
  }
}

void setup_wifi() {
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

void connect_to_mqtt() {
  int tries = 3;
  while (!mqttClient.connected() && tries-- > 0) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe to the admin topic
      mqttClient.subscribe("admin");
      blinkPassword(true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      blinkPassword(false);
    }
  }
}

void displayRemainingTime() {
  timerDisplay.loop();
  const int MINUTE = 60;

  uint32_t remainingSeconds = timerCountDown.remaining();
  uint32_t second = remainingSeconds % MINUTE;
  uint32_t minute = remainingSeconds / MINUTE;

  const uint32_t timerDigits[] = {minute/10, minute%10, second/10, second%10};
  
  timerDisplay.clear();
  for(int i=0; i<4; ++i) {
    timerDisplay.setNumber(i+1, timerDigits[i]);
  }
  timerDisplay.setDot(2);
  timerDisplay.show();
}

/* Main Code */
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Transferring Water
  pinMode (ledsPin, OUTPUT); // Also sets up starry night pins
  ws2812b.begin();

  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin2, LOW);
  pinMode(transferringWaterResetButtonPin, INPUT_PULLUP);
  pinMode(transferButtonPin, INPUT_PULLUP);
  pinMode(transferPossibleLED, OUTPUT);
  digitalWrite(transferPossibleLED, LOW);

  // Spinning wheels
  wheels.setup();

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
  if (keypad.begin() == false) {
    Serial.println("\nERROR: cannot communicate to keypad.\n");
  }

  currentStage = WHEELS;

  updateDisplayWater();

  // connect to wifi
  setup_wifi();

  // set MQTT server and callback function
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(callback);

  // Connect to MQTT broker
  connect_to_mqtt();
}

void loop() {
  mqttClient.loop();

  // display remaining time
  displayRemainingTime();

  // Blink stars constantly
  blinkTimer.tick();

  switch(currentStage) {
    case WHEELS:
    {
      wheels.checkSolved();
      break;
    }
    case WATER:
    {
      playTransferWater();
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
