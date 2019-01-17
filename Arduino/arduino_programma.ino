/*
 * L.P. KLAVER MSc.
 * Kascontroller INKA4.0 2018.
 * Copyright (c) 2018 Lennart Klaver.
 * Software released under MIT license. 
 */
 
/* INCLUDES */
#include "DHTesp.h"

/* DEFINES */
#define LICHTSENSOR D2
#define VOCHTSENSOR A0  // Steeksensor.
#define RELAIS D1       // Relais is aan bij LOW (0) en uit bij HIGH (1).
#define LED_GROEN D7
#define LED_ROOD D8
#define TEMPSENSOR D0   // Temperatuur/vochtsensor.

/* GLOBALS */
DHTesp dht;

/* INIT */
// Wordt eenmalig gestart bij het opstarten van de controller.
void setup() {
  // Init pin richting.
  pinMode(LICHTSENSOR, INPUT);
  pinMode(VOCHTSENSOR, INPUT);
  pinMode(RELAIS, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GROEN, OUTPUT);
  pinMode(LED_ROOD, OUTPUT);
  //pinMode(TEMPSENSOR, INPUT); //Wordt hieronder in library gedaan.

  dht.setup(TEMPSENSOR, DHTesp::DHT11); // Init temperatuur/vochtsensor via library.

  // Init seriele communicatie naar pc.
  Serial.begin(9600);
}

/* MAIN */
// Onderstaande wordt continu opnieuw uitgevoerd als het einde van de functie wordt bereikt.
void loop() {

  //Ledje aan bij duisternis.
  if(digitalRead(LICHTSENSOR) == 1){
    digitalWrite(LED_BUILTIN, LOW);
  }else{
    digitalWrite(LED_BUILTIN, HIGH);
  }

  //Meting temperatuur en vochtigheid van de lucht.
  //float temperatuur = dht.getTemperature();
  //float luchtvochtigheid = dht.getHumidity();
  TempAndHumidity th = dht.getTempAndHumidity(); // Lees temp en vochtigheid in 1 leesactie i.p.v. 2.

  //Meting vochtigheid van de grond.
  int vochtwaarde = analogRead(VOCHTSENSOR);

  // Toon of we al water moeten geven.
  if (vochtwaarde < 512){
    digitalWrite(LED_GROEN, LOW);
    digitalWrite(LED_ROOD, HIGH);
  }else{
    digitalWrite(LED_GROEN, HIGH);
    digitalWrite(LED_ROOD, LOW);
  }

  //Tonen van waardes op de pc.
  Serial.println(th.temperature, DEC);
  Serial.println(th.humidity, DEC);
  Serial.println(vochtwaarde, DEC);
  Serial.println(digitalRead(LICHTSENSOR));
  Serial.println("=====");

  delay(2000);                       // wait for a second.
}
