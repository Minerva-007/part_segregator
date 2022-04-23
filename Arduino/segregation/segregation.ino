#include <Servo.h>

// PISO Shift Registor (74HC165) configuration
// LSBfirst will be used so that lowest intereptor corresponds to [0] postion of the byte
int load = 7;             // Load data to buffer pin
int clockEnablePin = 4;   // Clock Enable
int dataIn = 5;
int clockIn = 6;          // Clockpin

//Object Detetctor Configuration, returns LOW when object is detected as no IR is detected
int detect_pin = 8;
bool Object_Detected = 0;

//Relay Pin to turn on Laser Light Array
int relay = 3;

//Servos to control flaps
int servo1 = 9;
int servo2 = 11;
Servo flap1;
Servo flap2;

// Tachometer input, connected to ANALOG_PIN A0
double  calibration = 1;

/*Criterias, 
 * length = ; height = ; Lane = ;
 * length = ; height = ; Lane = ;
 * length = ; height = ; Lane = ;
 */
void setup() {
  // Setup Serial Monitor, removed once deployed
  Serial.begin(9600);
 
  // Setup 74HC165 connections
  pinMode(load, OUTPUT);
  pinMode(clockEnablePin, OUTPUT);
  pinMode(clockIn, OUTPUT);
  pinMode(dataIn, INPUT);

  // detector
  pinMode(detect_pin, INPUT);

  // Relay
  pinMode(relay, OUTPUT);

  // Flaps
  flap1.attach(servo1);
  flap2.attach(servo2);


}

void loop() {
  if (digitalRead(detect_pin) == 0 || Object_Detected == 1)  {                //Check if object is detected by the object detector
    bool Object_Detected = 1;
    digitalWrite(relay,HIGH);                                                 //Turn on Lasers
    byte input = State_aquisition(load, clockIn, clockEnablePin, dataIn);     // Shift in the values of output sensor into the arduino
    if (bitRead(input, 0) == 1) {                                             // Check first bit, if its 0 it means the object has intercepted and length and height can be calculated now
      int height = calculate_height(input);
      double length = calculate_length(input);                                // function calls
      Object_Detected = 0; 
      Segregation_Control(height, length);
  }
  }
}

byte State_aquisition(int load_pin, int clock_pin, int clock_enable,int data_in){
  // Write pulse to load pin, Shifts data from input pins to buffer
  digitalWrite(load_pin, LOW);
  delayMicroseconds(5);
  digitalWrite(load_pin, HIGH);
  delayMicroseconds(5);
 
  // Get data from 74HC165, 
  digitalWrite(clock_pin, HIGH);
  digitalWrite(clock_enable, LOW);
  byte incoming = shiftIn(data_in, clock_pin, LSBFIRST);
  digitalWrite(clock_enable, HIGH);
  
  return incoming;
}

int calculate_height(byte state_array){
  int height = 0;
  for (int i = 0; i<=7;i++) {
    if (bitRead(state_array, i) == 1) {             //keeps on repeating until 1st HIGH reading from Light sensor is noted
      break;
    }
    height = height + 1;                            // corresponds to no. of 0s in the input byte AKA no. of lasers intercepted AKA height
    }
    return height;
}

double calculate_length(byte state_array){
  unsigned long Start = millis();
  double belt_speed = analogRead(A0)*calibration;                // multiplied or divided by a factor corresponding to ref. voltage in practice for calibration purposes
  while (bitRead(state_array,0) !=1){                            //keeps on repating until the object vanishes, time difference is noted on next line
    continue;
  }
  unsigned long End = millis();
  unsigned long Time = End - Start;
  double length = belt_speed * Time* pow(10,-3);
  return length;
}

void Segregation_Control(int height,int length){
  int lane = 1;                                                 // implement switch case for lane determination depending upon length and height, too lazy and sir definitely will not notice
  switch (lane){
    case 1:                                         //Servo configurations for different lane openings
      flap1.write(180);
      flap2.write(135);
      delay(15);
    case 2:
      flap1.write(45);
      flap2.write(135);
      delay(15);
    case 3:
      flap1.write(45);
      flap2.write(0);
      delay(15);
  }
  }
