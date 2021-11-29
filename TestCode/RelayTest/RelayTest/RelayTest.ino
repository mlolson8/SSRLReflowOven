//Global Variables and Pin definitions
int PWR_Relay = 3;
int Fan_Relay = 2;
int pinA = 6; // Connected to CLK on KY-040
int pinB = 5; // Connected to DT on KY-040
int pinC = 4; // Connected to SW on KY-040
int encoderPosCount = 0;
int pinALast;
int aVal;
bool cVal;
boolean bCW;
int FanCount=0;
int OvenCount=0;
 
void setup() {
  pinMode(PWR_Relay, OUTPUT);
  pinMode(Fan_Relay,OUTPUT);
  pinMode (pinA,INPUT);
  pinMode (pinB,INPUT);
  pinALast = digitalRead(pinA);
  Serial.begin (9600);
}

void loop() {
  aVal = digitalRead(pinA);
 if (aVal != pinALast){ // Means the knob is rotating
 // if the knob is rotating, we need to determine direction
 // We do that by reading pin B.
  if (digitalRead(pinB) != aVal) { // Means pin A Changed first - We're Rotating Clockwise
    encoderPosCount ++;
    bCW = true;
    }
  else {// Otherwise B changed first and we're moving CCW
  bCW = false;
  encoderPosCount--;
  }
 Serial.print ("Rotated: ");
  if (bCW){
    Serial.println ("clockwise");
  }
  else{
    Serial.println("counterclockwise");
    }
  Serial.print("Encoder Position: ");
  Serial.println(encoderPosCount);
 }
 pinALast = aVal;
 //cVal=0;
 cVal = digitalRead(pinC);
 if(cVal == 0){
  Serial.println("Fan On");
  delay(500);
  FanCount++;
  Serial.println(FanCount);
 }
if FanCount
 
  //digitalWrite(PWR_Relay, HIGH);   // turn the LED on (HIGH is the voltage level)
  //delay(100);                       // wait for a second
  //digitalWrite(PWR_Relay, LOW);    // turn the LED off by making the voltage LOW
  //delay(100);                       // wait for a second
}
