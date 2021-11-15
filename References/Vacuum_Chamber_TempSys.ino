#include "PlayingWithFusion_MAX31855_J_correction.h"
#include "PlayingWithFusion_MAX31855_STRUCT_corr.h"
#include "PlayingWithFusion_MAX31855_T_correction.h"
#include "PlayingWithFusion_MAX31855_Wcorr.h"




/*
//The liquidcrystal library is a modified version for SPI communications.
//It can be downloaded from https://github.com/omersiar/ShiftedLCD

Pin assignments are as follows,
- bar to analog GND
+ bar to analog 5v
analog A1 to one resistor/cap +
analog A2 to other resistor/cap +
if buttons are reversed switch back
Digital pin 13 to HC595 pin 11
Digital pin 11 to HC595 pin 14
Digital pin 9 to HC595 pin 12
Digital pin 3 in use by thermocouple
Digital pin 4 in use by thermocouple
Digital pin 5 in use by thermocouple
Digital pin 6 in use by thermocouple
Digital pin 7 in use by thermocouple
Digital pin 8 in use by thermocouple
Digital pin 9 in use by thermocouple
Digital pin 10 in use by thermocouple
*/
#include <ShiftedLCD.h>
//#include <LiquidCrystal.h>
#include <SPI.h>

// include Playing With Fusion MAX31855 libraries
//#include "PlayingWithFusion_MAX31855_1CH.h"
//#include "PlayingWithFusion_MAX31855_STRUCT.h"
//#include "PlayingWithFusion_MAX31855_Wcorr.h"
//#include "PlayingWithFusion_MAX31855_STRUCT_corr.h"
#include "avr/pgmspace.h"


// setup CS pins used for the connection with the sensor
// other connections are controlled by the SPI library)

int8_t CS7_PIN =  10;
int8_t CS6_PIN =  9;
int8_t CS5_PIN =  8;
int8_t CS4_PIN =  7;;
int8_t CS3_PIN =  3;
int8_t CS2_PIN =  4;
int8_t CS1_PIN =  5;
int8_t CS0_PIN =  6;;


// pin to relay for heating element
int8_t ELEMENT_PIN = 13;

//this is the initial target setting for the system.
//it is set to 20 because that is close to the usual lab room temperature.

double temp_setting = 25;
//bool heater_on = false;

PWFusion_MAX31855_TC  thermocouple0(CS0_PIN);
PWFusion_MAX31855_TC  thermocouple1(CS1_PIN);
PWFusion_MAX31855_TC  thermocouple2(CS2_PIN);
PWFusion_MAX31855_TC  thermocouple3(CS3_PIN);
PWFusion_MAX31855_TC  thermocouple4(CS4_PIN);
PWFusion_MAX31855_TC  thermocouple5(CS5_PIN);
PWFusion_MAX31855_TC  thermocouple6(CS6_PIN);
PWFusion_MAX31855_TC  thermocouple7(CS7_PIN);

//names the pin that will send data through the shift register to the LCD.
LiquidCrystal lcd(11);
        //LiquidCrystal lcd(1,1,1,1,1,1);
int LCD_TC_rot_timer = 0;

//these three values will be used to store the current time at three different points in a cycle of operations.
int timer = 0;
  long timermillis;
int timeron = 0;
int timeroff = 0;
double timeon, timeoff, onratio;


//These values will be used to create smoothing and control arrays for the sensor data.
//The const is the size of the array, which is chosen as a const for the best balance of stability
//and refresh rate. The rest of these variables are initilized to zero for simplicity.

const int arr_len = 12;
double TCP_arr[arr_len];

double integrative_component = 0;
double derivative_component = 0;


bool safe = true;

double get_target_setting(){

  //double temp_setting;
  double volt_bits = analogRead(A1);
  double volts = volt_bits*5/1024;
  double volt2_bits = analogRead(A2);
  double volts2 = volt2_bits*5/1024;
  //Serial.print(volts);
  //Serial.println(volts2);
  int i = 0;

    if (volts > 2.1 && volts2 > 2.1) {

    return temp_setting;
    }
  else if (volts > 2.1)
    {temp_setting = temp_setting + 1;

    //This delay allows the signal at the capacitor to decay below the threshold for the while loop
    //unless the button is being held down.




      return temp_setting;
      }

  else if (volts2 > 2.1)
  {temp_setting = temp_setting - 1;
  volatile bool safe = true;
   //This delay allows the signal at the capacitor to decay below the threshold for the while loop
   //unless the button is being held down.



    return temp_setting;}

  else {
    return temp_setting;
  }


}

//Takes a value assigned elsewhere as the desired operational setting and
//calculates the return signals for the controller
double calculate_error(double setting, double primary_temp){

  //shifts each value of TCP_arr to the right by one
  //this is important for the derivative aspect of the controller.



  for (int op_count = arr_len; op_count >= 1; --op_count){
    TCP_arr[op_count] = TCP_arr[op_count - 1];
  }
  TCP_arr[0] = primary_temp;
  double total = 0;
  integrative_component = 0;
  //derivative_component = derivative_component*.9 + (primary_temp -  TCP_arr[0]);

  //derivative_component = 0;
  for(int op_count = 0; op_count < arr_len; op_count++)
  {
    total = total + TCP_arr[op_count];
    derivative_component = derivative_component*.99 + ((primary_temp - (TCP_arr[op_count]))/((op_count/2)+.5));

    integrative_component = integrative_component + ((setting - TCP_arr[op_count])/(op_count+2));
  }



  double avg_temp = total/(arr_len);
  double error = setting - avg_temp;

  return error;

}


// PID tuning variables.



void heater_control(double error, double current_temp){
    //Serial.println(heater);
    //This will allow the program to calculate and standardize amount of time
    //the heater is on vs off in a single cycle of operation of the loop.
    double Kp = 1;
    double Kd = -22;
    double Ki = 7.5;



    //Ki Kd and Kp are set to be conviniently tuned above


//Below is a cheat section that will are intended to allow the temperature to close large distance gaps
//more quickly than the close control algorithm can.

    if(error < -.5){
      Kd = 0;
    }

    if(error > 4){
      Ki = 1.4;
    }


    float ontime = error*(Kp) + derivative_component*(Kd) + integrative_component*(Ki);

    //Serial.println("ontime");
    //Serial.println(ontime);

    timeroff = micros()/1000000;
    long timeroffmillis = micros()/1000;
    
    float op_cycle = timeroffmillis - timermillis;

    //Serial.println("op_cycle");
    //Serial.println(op_cycle);
    //Serial.println(timeroffmillis);
    //Serial.println(timermillis);
    

    //This ensures that the operation of the heating element is performed a regular 4 times every second.
    //and ensures that the delay does not become longer if the controller
    double display_time = ontime;

    if(ontime >  30){
      delay(220 - op_cycle);
      ontime = 30;
      

    }

    else if(ontime < 0){
      delay(250 - op_cycle);
      //ontime = 0;

    }
    else{ //between 0 and 30
      delay(250 - op_cycle - ontime);

    }

    Serial.println((String)"\t" + temp_setting +  "\t"   + error + "\t"  +  integrative_component + "\t" + derivative_component + "\t" + ontime);





    //This is the actual operation of the heating element.
    //The if statement prevents a negative delay rolling over and delaying indefinitely, which would
    //leave the heating elements at full on until they melted
    if (ontime > 0){

      digitalWrite(A5,HIGH);
      delay(ontime);
      digitalWrite(A5, LOW);

      //Serial.println("WRITING IS HAPPENING");

    }
    else{
    digitalWrite(ELEMENT_PIN,LOW);

    }



}





void update_lcd(double setting, double primary_temp, double thermo_1, double thermo_2, double thermo_3){



  lcd.setCursor(0,1);
  // print the target temperature:
  lcd.print(int(setting));
  if (setting > 100){
    lcd.setCursor(2,1);
    lcd.print(" ");
  }
  lcd.setCursor(3,1);
  lcd.print("  ");


  lcd.setCursor(4,1);
  // print the temperature of the primary thermocouple:
  lcd.print(primary_temp);
  //rotate which thermocouple is displayed.
  if (LCD_TC_rot_timer <= 20)
  {
    lcd.setCursor(11,0);
    lcd.print("TC1");
    lcd.setCursor(11,1);
    lcd.print(thermo_1);
  }
  else if (LCD_TC_rot_timer <= 40)
  {
    lcd.setCursor(11,0);
    lcd.print("TC2");
    lcd.setCursor(11,1);
    lcd.print(thermo_2);
  }
  else if (LCD_TC_rot_timer <= 60)
  {
    lcd.setCursor(11,0);
    lcd.print("TC3");
    lcd.setCursor(11,1);
    lcd.print(thermo_3);
  }
  else if (LCD_TC_rot_timer > 60)
  {
    LCD_TC_rot_timer = 0;
  }

  LCD_TC_rot_timer += 1;
}

void emergency_stop(){
  digitalWrite(ELEMENT_PIN,LOW);
  volatile bool safe = false;
  lcd.setCursor(0,0);
  lcd.println("RESETRESETRESET");
  //lcd.setCursor(0,1);
  lcd.print("errorerrorerror");


}


void setup() {

  lcd.begin(16,2);
  //print temperature formatting to LCD
  lcd.print("Set TcP    Tc1");


  Serial.begin(9600);

  // setup for the the SPI library:
  SPI.begin();                        // begin SPI
  SPI.setDataMode(SPI_MODE1);         // MAX31865 is a Mode 1 device
                                      //    --> clock starts low, read on rising edge

  // initalize the thermocouple chip select pins
  pinMode(CS0_PIN, OUTPUT);
  pinMode(CS1_PIN, OUTPUT);
  pinMode(CS2_PIN, OUTPUT);
  pinMode(CS3_PIN, OUTPUT);
  pinMode(CS4_PIN, OUTPUT);
  pinMode(CS5_PIN, OUTPUT);
  pinMode(CS6_PIN, OUTPUT);
  pinMode(CS7_PIN, OUTPUT);

  pinMode(A5, OUTPUT);
  
  pinMode(ELEMENT_PIN, OUTPUT);
  digitalWrite(ELEMENT_PIN, LOW);


  //This pinout will be an interrupt to prevent an error leaving the heating elements on and damaging
  //them, the component being tested, or the vacuum chamber itself.

  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), emergency_stop, RISING);

  Serial.println("Playing With Fusion: MAX31855-4CH, SEN-30002");
  
  Serial.println("secs/4\tTC1\tTC2\tTC3\tTC4\tTC5\tTC6\tTC7\tTC8\tSetting\tError\tInt\tDer\tOntime\t");

  //sets average arrays to 0
  for (int thisReading = 0; thisReading < arr_len; thisReading ++){
    TCP_arr[thisReading] = temp_setting;

  }


}

void loop() {
  //This is the first timer that the will be compared to the other two.
  timer = micros()/1000000;
  timermillis = micros()/1000;
  //delay(50);                             // 500ms delay... can be much faster

   double setting = get_target_setting();

  static struct var_max31855 TC_CH0 = {0,0,0, 3, 0};
  static struct var_max31855 TC_CH1 = {0,0,0, 3, 0};
  static struct var_max31855 TC_CH2 = {0,0,0, 3, 0};
  static struct var_max31855 TC_CH3 = {0,0,0, 3, 0};
  static struct var_max31855 TC_CH4 = {0,0,0, 3, 0};
  static struct var_max31855 TC_CH5 = {0,0,0, 3, 0};
  static struct var_max31855 TC_CH6 = {0,0,0, 3, 0};
  static struct var_max31855 TC_CH7 = {0,0,0, 3, 0};
  double tmp, primary_temp, thermo_1, thermo_2, thermo_3, thermo_4, thermo_5, thermo_6, thermo_7;

  // update TC0
  struct var_max31855 *tc_ptr;
  tc_ptr = &TC_CH0;
  thermocouple0.MAX31855_update(tc_ptr);        // Update MAX31855 readings

  // update TC1
  tc_ptr = &TC_CH1;
  thermocouple1.MAX31855_update(tc_ptr);        // Update MAX31855 readings

  // update TC2
  tc_ptr = &TC_CH2;
  thermocouple2.MAX31855_update(tc_ptr);        // Update MAX31855 readings

  // update TC3
  tc_ptr = &TC_CH3;
  thermocouple3.MAX31855_update(tc_ptr);        // Update MAX31855 readings

  // update TC4
  tc_ptr = &TC_CH4;
  thermocouple4.MAX31855_update(tc_ptr);        // Update MAX31855 readings

  // update TC5
  tc_ptr = &TC_CH5;
  thermocouple5.MAX31855_update(tc_ptr);        // Update MAX31855 readings

  // update TC6
  tc_ptr = &TC_CH6;
  thermocouple6.MAX31855_update(tc_ptr);        // Update MAX31855 readings

  // update TC7
  tc_ptr = &TC_CH7;
  thermocouple7.MAX31855_update(tc_ptr);        // Update MAX31855 readings


  // Print information to serial port
  Serial.print(timer);
  //Serial.print(" ");
  //Serial.print(timeroff);
  Serial.print("\t");


  // TC0
  // MAX31855 External (thermocouple) Temp
  tmp = (double)TC_CH0.Tcorr;           // convert fixed pt # to double

  if(0x00 == TC_CH0.status){ primary_temp = tmp; Serial.print(primary_temp);}
  else if(0x01 == TC_CH0.status){Serial.print("OPEN");}
  else if(0x02 == TC_CH0.status){Serial.print("S2G");} //Short To GND
  else if(0x04 == TC_CH0.status){Serial.print("S2V");} //Short To VCC
  else{Serial.print("UF");} //Unknown Fault

  // TC1
  Serial.print("\t");                      // print internal temp heading
  // MAX31855 External (thermocouple) Temp
  tmp = (double)TC_CH1.Tcorr;
  if(0x00 == TC_CH1.status){ thermo_1 = tmp; Serial.print(thermo_1);}
  else if(0x01 == TC_CH1.status){Serial.print("OPEN");}
  else if(0x02 == TC_CH1.status){Serial.print("S2G");}
  else if(0x04 == TC_CH1.status){Serial.print("S2V");}
  else{Serial.print("UF");}

  // TC2
  Serial.print("\t");
  tmp = (double)TC_CH2.Tcorr;  // convert fixed pt # to double

  //Serial.print("TC Temp = ");                   // print TC temp heading
  if(0x00 == TC_CH2.status){ thermo_2 = tmp; Serial.print(thermo_2);}
  else if(0x01 == TC_CH2.status){Serial.print("OPEN");}
  else if(0x02 == TC_CH2.status){Serial.print("S2G");}
  else if(0x04 == TC_CH2.status){Serial.print("S2V");}
  else{Serial.print("UF");}

  // TC3
  Serial.print("\t");                      // print internal temp heading
  tmp = (double)TC_CH3.Tcorr;           // convert fixed pt # to double
  //Serial.print("TC Temp = ");                   // print TC temp heading
  if(0x00 == TC_CH3.status){thermo_3 = tmp; Serial.print(thermo_3);}
  else if(0x01 == TC_CH3.status){Serial.print("OPEN");}
  else if(0x02 == TC_CH3.status){Serial.print("S2G");}
  else if(0x04 == TC_CH3.status){Serial.print("S2V");}
  else{Serial.print("UF");}

  // TC4
  Serial.print("\t");                      // print internal temp heading
  tmp = (double)TC_CH4.Tcorr;           // convert fixed pt # to double
  //Serial.print("TC Temp = ");                   // print TC temp heading
  if(0x00 == TC_CH4.status){thermo_4 = tmp; Serial.print(thermo_4);}
  else if(0x01 == TC_CH4.status){Serial.print("OPEN");}
  else if(0x02 == TC_CH4.status){Serial.print("S2G");}
  else if(0x04 == TC_CH4.status){Serial.print("S2V");}
  else{Serial.print("UF");}

  // TC5
  Serial.print("\t");                      // print internal temp heading
  tmp = (double)TC_CH5.Tcorr;           // convert fixed pt # to double
  //Serial.print("TC Temp = ");                   // print TC temp heading
  if(0x00 == TC_CH5.status){thermo_5 = tmp; Serial.print(thermo_5);}
  else if(0x01 == TC_CH5.status){Serial.print("OPEN");}
  else if(0x02 == TC_CH5.status){Serial.print("S2G");}
  else if(0x04 == TC_CH5.status){Serial.print("S2V");}
  else{Serial.print("UF");}

  // TC6
  Serial.print("\t");                      // print internal temp heading
  tmp = (double)TC_CH6.Tcorr;           // convert fixed pt # to double
  //Serial.print("TC Temp = ");                   // print TC temp heading
  if(0x00 == TC_CH6.status){thermo_6 = tmp; Serial.print(thermo_6);}
  else if(0x01 == TC_CH6.status){Serial.print("OPEN");}
  else if(0x02 == TC_CH6.status){Serial.print("S2G");}
  else if(0x04 == TC_CH6.status){Serial.print("S2V");}
  else{Serial.print("UF");}

  // TC7
  Serial.print("\t");                      // print internal temp heading
  tmp = (double)TC_CH7.Tcorr;           // convert fixed pt # to double
  //Serial.print("TC Temp = ");                   // print TC temp heading
  if(0x00 == TC_CH7.status){thermo_7 = tmp; Serial.print(thermo_7);}
  else if(0x01 == TC_CH7.status){Serial.print("OPEN");}
  else if(0x02 == TC_CH7.status){Serial.print("S2G");}
  else if(0x04 == TC_CH7.status){Serial.print("S2V");}
  else{Serial.print("UF");}


 
  if (safe){

  //update_lcd(setting, primary_temp, thermo_1, thermo_2, thermo_3);

  double error = calculate_error(setting, primary_temp);

  heater_control(error, primary_temp);
}
 


}
