//Global Variables and Pin definitions
int PWR_Relay = 13;

void setup() {
  pinMode(PWR_Relay, OUTPUT);
}

void loop() {
  digitalWrite(PWR_Relay, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  digitalWrite(PWR_Relay, LOW);    // turn the LED off by making the voltage LOW
  delay(100);                       // wait for a second
}
