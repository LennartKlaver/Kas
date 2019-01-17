/*
 * L.P. KLAVER MSc.
 * Kascontroller INKA4.0.
 * Copyright (c) 2019 Lennart Klaver.
 * Software released under MIT license. 
 */
/* INCLUDES */
#include "DHTesp.h"
#include "ESP8266WiFi.h"
#include <ESP8266HTTPClient.h>

/* DEFINES */
//Stepper
#define STEPPIN D1
#define DIRPIN D2
#define MOTORSLEEPPIN D3
#define LEFT HIGH
#define RIGHT LOW
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
//WiFi
#define WIFI_SSID "KLAVER"
#define WIFI_PASSWORD "WIFIKLAVER1713"

/* FUNCTION PROTOTYPES */
int getMoistureLevel();
bool isDark();
float getTemperature();
float getHumidity();
float getDistance();
void doSteps(int);
bool doStep();
void setSpeed(unsigned int, unsigned long *, unsigned int *);
void rampUp(unsigned int);
void rotateLeft();
void rotateRight();
void disablePump();
void enablePump();
void doGiveWater(); 

/* GLOBAL VARIABLES */
DHTesp dht;
const unsigned int uiStepsPerRevolution = 800; // Is set on the driver chip.
int iStepCount = 0;
unsigned int uiDesiredSpeedRpm; //rpm
unsigned int uiSpeedRpm; //rpm
unsigned long ulDesiredStepDelay = 0;
unsigned long ulStepDelay = 0;
unsigned long ulLastStepTime = 0;
unsigned long ulLastRampTime = 0;
WiFiClient client;

/* INIT */ 
void setup() {
  //Zet IO richtingen.
  pinMode(STEPPIN, OUTPUT);
  pinMode(DIRPIN, OUTPUT);
  pinMode(MOTORSLEEPPIN, OUTPUT);
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

  //Start wifi.
  Serial.println("starting");
  WiFi.disconnect();
  Serial.println("disconnected");
  wifiConnect();
  Serial.println("Connected");
  Serial.println("googling");
  sendStatistics();
  Serial.println("done");
}

/* MAIN */
void loop() {
  Serial.println((String) "m:" + getMoistureLevel() + " d:" + getDistance() + " l:" + isDark() + " t:" + getTemperature() + " h:" + getHumidity());

  //Check if the soil is dry.
  if(getMoistureLevel() < 50){
    Serial.println("Wetting...");
    //Yes it is dry, give some water.
    doGiveWater(); 
    //Wait some time to let the water flow.
    delay(10000);
  }

  delay(5000);
}

/* Do connect to the WiFi.
 */
void wifiConnect() {
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
}

/* Send statistics to the server.
 */
void sendStatistics(){
 /* HTTPClient http;
  http.begin("http://www.google.nl");
  //http.addHeader("Content-Type", "application/json");
  int httpCode = http.GET();
  Serial.println(httpCode);
  if(httpCode == HTTP_CODE_OK) {
    Serial.print("HTTP response code ");
    Serial.println(httpCode);
    String response = http.getString();
    Serial.println(response);
  }
  http.end();*/
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

/* Read the temperature using the library.
 * returns: temperature as float in Celsius.
 */
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

/* Read the humidity using the library.
 * returns: humidity in percentage (relative humidity).
 */
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

/* Enable the humidity/temperature sensor.
 */
void enableDHT(){
  digitalWrite(DHT11ENABLEPIN, HIGH);
}

/* Disable the humidity/temperature sensor.
 */
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

void doGiveWater(){
  setSpeed(45, &ulDesiredStepDelay, &uiDesiredSpeedRpm);
  enablePump();
  doSteps(8000);
  disablePump();
}

/* Do multiple steps.
 * This is a blocking function untill all steps are done.
 */
void doSteps(int steps){
  //Input check.
  if(steps == 0) 
  {
    return;
  }

  unsigned int stepsToDo = abs(steps);

  //Determine direction.
  if(steps < 0) {
    rotateLeft();
  } else {
    rotateRight();
  }

  //Set start speed
  setSpeed(1, &ulStepDelay, &uiSpeedRpm);

  //Do steps.
  while(stepsToDo > 0){
    rampUp(uiDesiredSpeedRpm);
    
    if(doStep()){
      stepsToDo--;
      //Update current position.
      if(steps < 0) {
        iStepCount--;
      } else {
        iStepCount++;
      } 
    }
    yield();
  }
}

/* Do one step.
 */
bool doStep(){
  unsigned long now = micros();
  if (now - ulLastStepTime >= ulStepDelay)
  {
    ulLastStepTime = now;
    digitalWrite(STEPPIN, HIGH);
    delayMicroseconds(1);
    digitalWrite(STEPPIN, LOW);
    delayMicroseconds(1);
   
    return true;
  }
  return false;
}

/* Set the speed setpoint.
 */
void setSpeed(unsigned int desiredspeed, unsigned long * myDelay, unsigned int * mySpeed){
  if(desiredspeed > 0 && desiredspeed <= 360){
    *mySpeed = desiredspeed; 
    *myDelay = 60L * 1000L * 1000L / uiStepsPerRevolution / desiredspeed;
  }
}

/* Gradually increase motor speed to deliver more torque during power on.
 */
void rampUp(unsigned int speed_desired){
  unsigned long now = micros();
  unsigned int speed_rpm;

  speed_rpm = uiSpeedRpm;
  
  if(now - ulLastRampTime >= 100000){
    ulLastRampTime = now;
    if(speed_rpm < speed_desired){
      speed_rpm++; 
    }else{
      speed_rpm = speed_desired;
    }
  }
  setSpeed(speed_rpm, &ulStepDelay, &uiSpeedRpm);
}

/* Set pump direction to counter clockwise.
 */
void rotateLeft(){
  digitalWrite(DIRPIN,LEFT);
  delayMicroseconds(1);
}

/* Set pump direction to clockwise.
 */
void rotateRight() {
  digitalWrite(DIRPIN,RIGHT);
  delayMicroseconds(1);
}

/* Disable the magnetic field of the motor.
 * The motor will not provide any force anymore.
 */
void disablePump(){
  digitalWrite(MOTORSLEEPPIN, LOW);
  delay(2);
}

/* Enable the magnetic field of the motor.
 * The motor delivers a continuous force on the shaft.
 */
void enablePump(){
  digitalWrite(MOTORSLEEPPIN, HIGH);
  delay(2);
}
