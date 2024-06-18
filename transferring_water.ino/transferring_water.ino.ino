#include <Adafruit_NeoPixel.h>

const int numJugs = 3;
const int capacities[] = {8,5,3};
const int target = 4;

const byte transferButtonPin = 14;
const byte fillingPins[] = {18,19,21};
const byte relayPin = 5;
const byte ledsPin = 32;

const int ledMapping[] = {0,1,2,3,4,5,6,7, 12,11,10,9,8, 13,14,15};
Adafruit_NeoPixel ws2812b(16, ledsPin, NEO_GRB + NEO_KHZ800);

int currentValues[] = {8,0,0};

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
  if (isSolved()){
    digitalWrite(relayPin, HIGH);
    delay(250);
    digitalWrite(relayPin, HIGH);
  }
}

bool isSolved() {
  for(int i=0; i<numJugs; i++){
    if(currentValues[i] == target){
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
        ws2812b.setPixelColor(ledMapping[ledIndex], ws2812b.Color(0, 0, 50));  // it only takes effect if pixels.show() is called
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode (ledsPin, OUTPUT);
  ws2812b.begin();

  pinMode(transferButtonPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  updateDisplay();
}

void loop() {
  int transferButtonState = digitalRead(transferButtonPin);
  int fromJug = -1, toJug = -1;
  for (int i=0; i<numJugs; i++) {
    for (int j=0; j<numJugs; j++) {
      if (i == j)
        continue;
      if (isConnected(fillingPins[i], fillingPins[j])) {
        Serial.println("CONNECTED:");
        Serial.println(j);
        Serial.println(i);
        fromJug = j;
        toJug = i;
        break;
      }
    }
  }
  if (fromJug != -1 && toJug != -1) {
    if (currentValues[fromJug] > 0 && currentValues[toJug] < capacities[toJug] && transferButtonState == LOW){
      Serial.println("TRANSFER:");
      transfer(fromJug, toJug);
    }
  }
  
  delay(10); // this speeds up the simulation
}
