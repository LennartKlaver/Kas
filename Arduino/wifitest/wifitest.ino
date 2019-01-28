/**
   BasicHTTPSClient.ino
    Created on: 20.08.2018
*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>
// Fingerprint for demo URL, expires on June 2, 2019, needs to be updated well before this date
const uint8_t fingerprint[20] = {0xD1, 0x45, 0x7D, 0x49, 0x7B, 0x99, 0xF6, 0x4C, 0x14, 0xFE, 0x97, 0x2F, 0xC0, 0x9A, 0xA1, 0xA3, 0xDC, 0x77, 0x01, 0x8C};

ESP8266WiFiMulti WiFiMulti;

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("KLAVER", "WIFIKLAVER1713");


  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    client->setFingerprint(fingerprint);

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://kas.itpweb.nl/measurement/create.php")) {  // HTTPS
      https.addHeader("Content-Type", "application/json");

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.POST("{\"apikey\":\"XSYUqxMRRMaJ5TeskMUKdxwmzATWaeJLpMVya59YLehF2gUw\",\"measurement\":{\"temperature\":19,\"moisture\":25,\"humidity\":23,\"light\":1,\"distance\":10}}");

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

  delay(10000);
}

void loop() {
}
