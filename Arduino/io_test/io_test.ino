/*
 * L.P. KLAVER MSc.
 * Kascontroller INKA4.0.
 * Copyright (c) 2019 Lennart Klaver.
 * Software released under MIT license. 
 */
/* INCLUDES */
#include "DHTesp.h"

/* DEFINES */
//Stepper
#define STEPPIN D1
#define DIRPIN D2
#define MOTORSLEEPPIN D3
//Moist sensor
#define MOISTPIN A0
//Light sensor
#define LIGHTPIN D0
//Humidity and temperature
#define DHT11DATAPIN D5
#define DHT11ENABLEPIN D6
//Sonar
#define SONARTRIGPIN D8
#define SONARDATAPIN D7

/* PROTOTYPES */
int getMoistureLevel();
bool isDark();
float getTemperature();
float getHumidity();
float getDistance();

/* GLOBAL VARIABLES */
DHTesp dht;

/* INIT */ 
void setup() {
  //Zet IO richtingen.
  //pinMode(STEPPIN, OUTPUT);
  //pinMode(DIRPIN, OUTPUT);
  //pinMode(MOTORSLEEPPIN, OUTPUT);
  pinMode(MOISTPIN, INPUT);
  pinMode(LIGHTPIN, INPUT);
  pinMode(DHT11DATAPIN, INPUT);
  pinMode(DHT11ENABLEPIN, OUTPUT);
  pinMode(SONARTRIGPIN, OUTPUT);
  pinMode(SONARDATAPIN, INPUT);

  //Init de dht temperatuur sensor.
  dht.setup(DHT11DATAPIN, DHTesp::DHT11);

  //Start seriele poort voor info.
  Serial.begin(9600);
}

/* MAIN */
void loop() {
  delay(5000);
  Serial.println((String) "m:" + getMoistureLevel() + " d:" + getDistance() + " l:" + isDark() + " t:" + getTemperature() + " h:" + getHumidity());
}

/* getMoistureLevel
 * returns: The measured conductivity between the two plates (dimensionless).
 */
int getMoistureLevel(){
  return analogRead(MOISTPIN);
}

/* isDark
 * returns: 1 if dark or 0 if light. Set the threshold using the potentiometer.
 */
bool isDark(){
  return digitalRead(LIGHTPIN);
}

float getTemperature(){
  float result = 0;
  //Zet de sensor aan.
  enableDHT();
  //Lees de waarde uit.
  result = dht.getTemperature();
  //Zet de sensor uit.
  disableDHT();
  return result;
}

float getHumidity(){
  float result = 0;
  //Zet de sensor aan.
  enableDHT();
  //Lees de waarde uit.
  result = dht.getHumidity();
  //Zet de sensor uit.
  disableDHT();
  return result;
}

void enableDHT(){
  digitalWrite(DHT11ENABLEPIN, HIGH);
}

void disableDHT(){
  digitalWrite(DHT11ENABLEPIN, LOW);
}

/* getDistance
 * returns: Distance in centimeter as float.
 */
float getDistance(){
  float duration = 0;
  float distance = 0;

  //Set the trigger to low and then trigger an 8 cycle burst (10 ms).
  digitalWrite(SONARTRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(SONARTRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(SONARTRIGPIN, LOW);

  //Count the number of incoming pulses.
  duration = pulseIn(SONARDATAPIN, HIGH);

  //Convert time to distance using speed of sound in air.
  //Divide by 2 because the sound travels to the object and back.
  distance = (duration*.0343)/2;
  
  return distance;
}
