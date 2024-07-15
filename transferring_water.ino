#include <Adafruit_NeoPixel.h>
#include <Keypad.h>
#include <WiFi.h>
#include <PubSubClient.h>

enum stage {WHEELS, WATER, STARS, SOLVED};
stage currentStage;

/* CONSTANTS */
// Wi-Fi
const char* ssid = "ESP32_AP";
const char* password = "12345678";
WiFiClient espClient;

// MQTT
const char* mqtt_server = "192.168.4.2";
const int mqtt_port = 1883;
PubSubClient mqttClient(mqtt_server, mqtt_port, espClient);

// TRANSFERRING WATER
const int numJugs = 3;
const int capacities[] = {8,5,3};
const int target = 4;

const byte transferButtonPin = 35;
const byte transferringWaterResetButtonPin = 34;
const byte fillingPins[] = {25,26,27};
const byte transferPossibleLED = 32;
const byte ledsPin = 33;

const int ledMapping[] = {0,1,2,3,4,5,6,7, 12,11,10,9,8, 13,14,15};
Adafruit_NeoPixel ws2812b(16, ledsPin, NEO_GRB + NEO_KHZ800);

int currentValues[] = {8,0,0};

// SPINNING WHEELS
const byte spinningWheelsPin = 15;

// STARRY NIGHT
String inputString;
String starSolution = "1234";

// KEYPAD
const byte  rowsCount = 4;
const byte  columsCount = 3;

char keys[rowsCount][columsCount] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte columnPins[columsCount] = {21, 23, 18 };
byte   rowPins[rowsCount] = {22, 4, 5, 19};
Keypad keypad = Keypad(makeKeymap(keys),rowPins,columnPins,rowsCount,columsCount);

// RELAY PINS
const byte relayPin1 = 13;
const byte relayPin2 = 12;
const byte relayPin3 = 14;

/* CODE */
// Transferring Water
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
    updateDisplay();
    // speed of pouring
    delay(500);
  }

  // check if the puzzle is solved and open the door.
  if (isTransferSolved()){
    digitalWrite(relayPin2, HIGH);
    delay(1000);
    digitalWrite(relayPin2, LOW);
    Serial.print("Solved Transferring Water");
    currentStage = STARS;
    digitalWrite(transferPossibleLED, LOW);
  }
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

void updateDisplay() {
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
  currentValues[0] = 8;
  currentValues[1] = 0;
  currentValues[2] = 0;
  updateDisplay();
}

void playTransferWater() {
  int resetButtonState = digitalRead(transferringWaterResetButtonPin);
  if (resetButtonState == LOW) {
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

// SPINNING WHEELS
void solveWheels() {
  digitalWrite(relayPin1, HIGH);
  delay(1000);
  digitalWrite(relayPin1, LOW);
  currentStage = WATER;
  Serial.print("solved wheels");
}

// STARRY NIGHT
void solveStars() {
  digitalWrite(relayPin3, HIGH);
  delay(1000);
  digitalWrite(relayPin3, LOW);
  currentStage = SOLVED;
  Serial.print("solved stars");
}

void playStarryNight() {
  char key = keypad.getKey();
  if (key) {
    inputString += key;
    Serial.println(inputString);
  }
  if (inputString.length() >= 4) {
    if(inputString == starSolution) {
      solveStars();
    } else {
      inputString = "";
      Serial.println("WRONG PASSWORD");
    }
  }
}

// Wifi and MQTT functions
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  // TODO: Handle different messages
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup_wifi() {
  Serial.println();
  Serial.print("Setting up AP: ");
  Serial.println(ssid);
  
  // Set the ESP32 as an access point
  WiFi.softAP(ssid, password);

  // Print the IP address of the access point
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

void connect_to_mqtt() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe to the admin topic
      mqttClient.subscribe("admin");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/* Main Code */
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Transferring Water
  pinMode (ledsPin, OUTPUT);
  ws2812b.begin();

  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin2, LOW);
  pinMode(transferringWaterResetButtonPin, INPUT_PULLUP);
  pinMode(transferButtonPin, INPUT_PULLUP);
  pinMode(transferPossibleLED, OUTPUT);
  digitalWrite(transferPossibleLED, LOW);

  // Spinning wheels
  pinMode(spinningWheelsPin, INPUT_PULLUP);
  pinMode(relayPin1, OUTPUT);
  digitalWrite(relayPin1, LOW);

  // Starry night
  pinMode(relayPin3, OUTPUT);
  digitalWrite(relayPin3, LOW);

  currentStage = WHEELS;

  updateDisplay();

  // connect to wifi
  setup_wifi();

  // set MQTT server and callback function
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(callback);

  // Connect to MQTT broker
  connect_to_mqtt();
}

void loop() {
  if (!mqttClient.connected()) {
    connect_to_mqtt();
  }
  mqttClient.loop();
  switch(currentStage) {
    case WHEELS:
    {
      int solvedWheels = digitalRead(spinningWheelsPin);
      if (solvedWheels == HIGH) {
        solveWheels();
      }
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
  
  delay(10); // this speeds up the simulation
}
