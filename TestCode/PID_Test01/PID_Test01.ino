#include "SPI.h"
#include "PlayingWithFusion_MAX31855_Wcorr.h"
#include "SoftwareSerial.h"

int PWR_Relay = 3;
int Fan_Relay = 2;
int CLK = 6; // Connected to CLK on KY-040
int DT = 5; // Connected to DT on KY-040
int SW = 4; // Connected to SW on KY-040
int encoderPosCount = 0;
int CLKLast;
int aVal;
bool cVal = 1;
bool cVal_old = 1;
boolean bCW;
int FanCount = 0;
int OvenCount = 0;
int DispCount = 0;
int8_t CS0_PIN = 10;
PWFusion_MAX31855_TC  thermocouple0(CS0_PIN);
unsigned long previousMillis = 0; //Helps with keeping track of the time elapsed
const long SampleInterval = 500; //Interval in which the serial monitor will read an output
static struct var_max31855 TC_CH0 = {0, 0, 0, 3, 0}; //Specifies T-Type
bool Temp_init = true; //Initializes Starting Headers

float DutyCycle_percent = .4; //PWM Duty Cycle Percentage from 0-1
int DutyCycle = DutyCycle_percent*255;


void read_encoder()
{
  //Rotary Encoder Controls
  aVal = digitalRead(CLK);
  if (aVal != CLKLast && aVal % 2 == 0){ // Means the knob is rotating
    aVal = aVal / 2;
   // if the knob is rotating, we need to determine direction
   // We do that by reading pin DT.
    if (digitalRead(DT) != aVal) { // Means pin A Changed first - We're Rotating Clockwise
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
CLKLast = aVal;
 
//Relay Push Button Controls
   cVal_old = cVal;
   delay(1);
   cVal = digitalRead(SW);
  if((cVal != cVal_old) && (cVal == 1)){
    FanCount++;
    //Serial.println("Button pushed:" + String(Fancount));
   }
  if (FanCount % 3 == 0) {
    //Serial.println("Fan OFF;Heat OFF");
    analogWrite(PWR_Relay, 0);
    digitalWrite(Fan_Relay, LOW);
  }
  else if (FanCount % 3 == 1){
    //Serial.println("Fan ON;Heat ON");
    analogWrite(PWR_Relay, DutyCycle);
    digitalWrite(Fan_Relay, HIGH);
  }
   else if (FanCount % 3 == 2){
    //Serial.println("Fan ON;Heat OFF");
    analogWrite(PWR_Relay, 0);
    digitalWrite(Fan_Relay, HIGH);
  }
}


double read_Temp()
{
    double tmp;
    struct var_max31855 *tc_ptr;
    // update T.C. Reading
    tc_ptr = &TC_CH0;
    thermocouple0.MAX31855_update(tc_ptr);        // Update MAX31855 readings   
    //Temp Reading
    tmp = (double)TC_CH0.ref_jcn_temp * 0.0625;  // convert fixed pt # to double
    if((-100 > tmp) || (150 < tmp)){Serial.println("unknown fault");}
    tmp = (double)TC_CH0.value * 0.25;           // convert fixed pt # toouble
    //Serial.print("Temp = ");                   // print TC temp heading
    return tmp;
}


void temp_logging(unsigned long currentMillis)
{
  if (Temp_init == true){
    //Logging Data Header
    Serial.println("Time\tTemperature (C)");
    Temp_init = false;
  }
  float Seconds = (float) currentMillis/1000;
  if(currentMillis - previousMillis >= SampleInterval){ //Reduces the number of read datapoints so its doesn't spam the serial monitor
    previousMillis = currentMillis;
    //Serial.print(String(Seconds) + "\t"); // prints time since program started
    double current_temp = read_Temp();
    Serial.print(String(Seconds)+"\t");
    if(0x00 == TC_CH0.status){
      Serial.println(current_temp);
      }
    else if(0x01 == TC_CH0.status){
      Serial.println("OPEN");
      }
    else if(0x02 == TC_CH0.status){
      Serial.println("SHORT TO GND");
      }
    else if(0x04 == TC_CH0.status){
      Serial.println("SHORT TO Vcc");
      }
    else{
      Serial.println("unknown fault");
    }
  }
}


void setup()
{
  Serial.begin(9600);
  SPI.begin();                        // begin SPI
  SPI.setDataMode(SPI_MODE1);         // MAX31865 is a Mode 1 device
  pinMode(CS0_PIN, OUTPUT);  // initalize the chip select pin
  //Serial.println("Playing With Fusion: MAX31855-4CH, SEN-30002");
  pinMode(PWR_Relay, OUTPUT);
  pinMode(Fan_Relay,OUTPUT);
  pinMode (CLK,INPUT);
  pinMode (DT,INPUT);
  pinMode (SW,INPUT);
  CLKLast = digitalRead(CLK);
}


void loop()
{
  read_encoder();
  unsigned long currentMillis = millis();
  temp_logging(currentMillis);
 
  
}
