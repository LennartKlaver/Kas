/*
 * L.P. KLAVER MSc.
 * Kascontroller INKA4.0.
 * Copyright (c) 2019 Lennart Klaver.
 * Software released under MIT license. 
 * 
 * Uses ArduinoJson library 5.13.4 by Benoit Blanchon.
 */
/* INCLUDES */
//WiFi
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
//Other
#include "DHTesp.h"
#include "ArduinoJson.h"

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
//Wifi
#define JSONMSGLEN 350

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
void wifisetup();
void sendToSite(char *);
void getStatisticsJSON(char *,unsigned int);

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
//Wifi
// Fingerprint for demo URL, expires on June 2, 2019, needs to be updated well before this date
const uint8_t fingerprint[20] = {0xD1, 0x45, 0x7D, 0x49, 0x7B, 0x99, 0xF6, 0x4C, 0x14, 0xFE, 0x97, 0x2F, 0xC0, 0x9A, 0xA1, 0xA3, 0xDC, 0x77, 0x01, 0x8C};
ESP8266WiFiMulti WiFiMulti;
unsigned int counter;

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
  
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  wifisetup();

  // wait for WiFi connection
  if ((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.print(".");
  }
  Serial.println();
 
}

/* MAIN */
void loop() {
  char jsonmsg[JSONMSGLEN];

  // Send statistics after x minutes.
  Serial.println(counter);
  if(counter >= 40){
    counter = 0;

    Serial.println("start");
    //Get the JSON.
    getStatisticsJSON(jsonmsg, JSONMSGLEN);
    Serial.println(jsonmsg);

    //Send them.
    sendToSite(jsonmsg);
  }
  
  Serial.println((String) "m:" + getMoistureLevel() + " d:" + getDistance() + " l:" + isDark() + " t:" + getTemperature() + " h:" + getHumidity());

  //Check if the soil is dry.
  if(getMoistureLevel() < 50){
    Serial.println("Wetting...");
    //Yes it is dry, give some water.
    doGiveWater(); 
  }
  //Wait some time to let the water flow.
  delay(15000);
  counter++;
}

/* Do connect to the WiFi.
 */
void wifisetup(){
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("KLAVER", "WIFIKLAVER1713");  
}

void sendToSite(char * msg){

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    client->setFingerprint(fingerprint);

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://kas.itpweb.nl/measurement/create.php")) {  // HTTPS
      https.addHeader("Content-Type", "application/json");

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      //int httpCode = https.POST("{\"apikey\":\"XSYUqxMRRMaJ5TeskMUKdxwmzATWaeJLpMVya59YLehF2gUw\",\"measurement\":{\"temperature\":19,\"moisture\":25,\"humidity\":23,\"light\":1,\"distance\":10}}");
      int httpCode = https.POST(msg);

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }  
}


/* Send statistics to the server.
 */
void getStatisticsJSON(char * result, unsigned int len){
  DynamicJsonBuffer JSONbuffer;
  JsonObject& root = JSONbuffer.createObject();
  root["apikey"] = "XSYUqxMRRMaJ5TeskMUKdxwmzATWaeJLpMVya59YLehF2gUw";

  JsonObject& measurement = root.createNestedObject("measurement");
  measurement["temperature"] = getTemperature();
  measurement["moisture"] = getMoistureLevel();
  measurement["humidity"] = getHumidity();
  measurement["light"] = isDark();
  measurement["distance"] = getDistance();
  measurement["water"] = 0;

  //To string.
  root.prettyPrintTo(result, len);
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
