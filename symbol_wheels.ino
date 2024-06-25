const byte puzzlePin = 22;
const byte relayPin = BUILTIN_LED;

void solve() {
  digitalWrite(relayPin, HIGH);
  delay(1000);
  digitalWrite(relayPin, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(puzzlePin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
}

void loop() {
  int solvedState = digitalRead(puzzlePin);
  if (solvedState == HIGH) {
    solve();
  }
}