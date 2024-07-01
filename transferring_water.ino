#include <Adafruit_NeoPixel.h>

enum stage {WHEELS, WATER, STARS, SOLVED};
stage currentStage;

/* CONSTANTS */
// TRANSFERRING WATER
const int numJugs = 3;
const int capacities[] = {8,5,3};
const int target = 5;

const byte transferButtonPin = 14;
const byte fillingPins[] = {18,19,21};
const byte transferPossibleLED = 22;
const byte ledsPin = 32;

const int ledMapping[] = {0,1,2,3,4,5,6,7, 12,11,10,9,8, 13,14,15};
Adafruit_NeoPixel ws2812b(16, ledsPin, NEO_GRB + NEO_KHZ800);

int currentValues[] = {8,0,0};

// SPINNING WHEELS
const byte spinningWheelsPin = 23;

// RELAY PINS
const byte relayPin1 = 26;
const byte relayPin2 = BUILTIN_LED;
const byte relayPin3 = BUILTIN_LED;

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
    // digitalWrite(relayPin, HIGH);
    // delay(1000);
    // digitalWrite(relayPin, LOW);
    Serial.print("Solved Transferring Water");
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

// SPINNING WHEELS
void solveWheels() {
  digitalWrite(relayPin1, HIGH);
  delay(1000);
  digitalWrite(relayPin1, LOW);
  currentStage = WATER;
  Serial.print("solved wheels");
}

void setup() {
  // put your setup code here, to run once:
  // Transferring water
  Serial.begin(115200);
  pinMode (ledsPin, OUTPUT);
  ws2812b.begin();

  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin2, LOW);
  pinMode(transferButtonPin, INPUT_PULLUP);
  pinMode(transferPossibleLED, OUTPUT);
  digitalWrite(transferPossibleLED, LOW);

  // Spinning wheels
  pinMode(spinningWheelsPin, INPUT_PULLUP);
  pinMode(relayPin1, OUTPUT);
  digitalWrite(relayPin1, LOW);

  currentStage = WHEELS;

  updateDisplay();
}

void loop() {
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
      break;
    }
    case STARS:
    case SOLVED:
      break;
  }
  
  delay(10); // this speeds up the simulation
}
