//Global Variables and Pin definitions
int PWR_Relay = 3;
int Fan_Relay = 2;
int pinA = 6; // Connected to CLK on KY-040
int pinB = 5; // Connected to DT on KY-040
int pinC = 4; // Connected to SW on KY-040
int encoderPosCount = 0;
int pinALast;
int aVal;
bool cVal = 1;
bool cVal_old = 1;
boolean bCW;
int FanCount=0;
int OvenCount=0;
 
void setup() {
  pinMode(PWR_Relay, OUTPUT);
  pinMode(Fan_Relay,OUTPUT);
  pinMode (pinA,INPUT);
  pinMode (pinB,INPUT);
  pinMode (pinC,INPUT);
  pinALast = digitalRead(pinA);
  Serial.begin (9600);
}

void loop() {
  //Rotary Encoder Controls
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
Serial.println ("clockwise"); //Clockwise Rotation Case
}
else{
Serial.println("counterclockwise");
//Counter Clockwise Rotation Case
}
Serial.print("Encoder Position: ");
Serial.println(encoderPosCount);
}
pinALast = aVal;
 
//Push Button Controls
 cVal_old = cVal;
 delay(1);
 cVal = digitalRead(pinC);
if((cVal != cVal_old) && (cVal == 1)){
  Serial.println("Button pushed");
  //delay(500);
  FanCount++;
  Serial.println(FanCount);
 }
if (FanCount % 2 == 1) {
  Serial.println("Fan ON");
  digitalWrite(PWR_Relay, HIGH);
  digitalWrite(Fan_Relay, HIGH);
}
else if (FanCount % 2 == 0){
  Serial.println("Fan OFF");
  digitalWrite(PWR_Relay, LOW);
  digitalWrite(Fan_Relay, LOW);
}
}
