#include "SPI.h"
#include "PlayingWithFusion_MAX31855_Wcorr.h"
#include "SoftwareSerial.h"

int8_t CS0_PIN = 10;
PWFusion_MAX31855_TC  thermocouple0(CS0_PIN);
unsigned long myTime;


void setup()
{
  
  Serial.begin(9600);
  SPI.begin();                        // begin SPI
  SPI.setDataMode(SPI_MODE1);         // MAX31865 is a Mode 1 device
  pinMode(CS0_PIN, OUTPUT);  // initalize the chip select pin
  //Serial.println("Playing With Fusion: MAX31855-4CH, SEN-30002");
  Serial.print("Time\tTemperature (C)\n");
}

void loop()
{
  delay(500);                                   // 500ms delay... can be much faster
  //Serial.print("Time: ");
  myTime = millis();
  float Seconds = (float) myTime/1000;

  Serial.print(String(Seconds) + "\t"); // prints time since program started
  //delay(1000);          // wait a second so as not to send massive amounts of data

  
  static struct var_max31855 TC_CH0 = {0, 0, 0, 3, 0}; //Specifies T-Type
  double tmp;
  struct var_max31855 *tc_ptr;
  
  // update T.C. Reading
  tc_ptr = &TC_CH0;
  thermocouple0.MAX31855_update(tc_ptr);        // Update MAX31855 readings   

  //Temp Reading
  tmp = (double)TC_CH0.ref_jcn_temp * 0.0625;  // convert fixed pt # to double
  if((-100 > tmp) || (150 < tmp)){Serial.println("unknown fault");}
  tmp = (double)TC_CH0.value * 0.25;           // convert fixed pt # toouble
  //Serial.print("Temp = ");                     // print TC temp heading
  if(0x00 == TC_CH0.status){Serial.println(tmp);}
  else if(0x01 == TC_CH0.status){Serial.println("OPEN");}
  else if(0x02 == TC_CH0.status){Serial.println("SHORT TO GND");}
  else if(0x04 == TC_CH0.status){Serial.println("SHORT TO Vcc");}
  else{Serial.println("unknown fault");
  }
  
}
