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
