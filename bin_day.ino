#include <WiFiManager.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "HTTPSRedirect.h"
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FastLED.h>

const char* calendar_id = "AKfycbxdxppvk0O9S7zMECsDGRG2gpjgjt74SJt4UFd5jgZHIYr1bJk";

#define NBR_EVENTS 6
#define UPDATETIME 3600000
//#define UPDATETIME 30000

#define PULSE_DELAY 15

#define OTA_NODE_NAME "BIN_01"
#define OTA_PASSWORD "woowb2018"

const int REDPIN   = D2;
const int GREENPIN = D3;
const int BLUEPIN  = D4;

String possibleEvents[NBR_EVENTS] = {"black", "blue", "yellow", "red", "green", "brown"};
CHSV led_colour[NBR_EVENTS] = {
  CHSV(192, 0, 255), 
  CHSV(160, 255, 255),
  CHSV(64, 255, 255),
  CHSV(0, 255, 255),
  CHSV(96, 255, 255),
  CHSV(192, 255, 255)
 };

//Connection Settings
const char* host = "script.google.com";
const char* googleRedirHost = "script.googleusercontent.com";
const int httpsPort = 443;

unsigned long entryUpdated = 0;

String url = String("/macros/s/") + calendar_id + "/exec";
int attempts = 0;
int binDay = -1;

HTTPSRedirect* client = nullptr;

void setColour(const CRGB& colour) {
  analogWrite(REDPIN, colour.r);
  analogWrite(GREENPIN, colour.g);
  analogWrite(BLUEPIN, colour.b);
}

void connectToWiFi() {
  WiFiManager wfm;
  // wfm.resetSettings();
  wfm.autoConnect("BinDay");
}

String getCalendar() {
  String calendarData = "";
  
  setColour(CRGB::Red);
  
  Serial.println("Start Request for calendar.");

  client = new HTTPSRedirect(httpsPort);
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  if (!client->connected()) client->connect(host, httpsPort);

  if (client->GET(url, host)) {
    calendarData = client->getResponseBody();

    Serial.print("Calender Data: ");
    Serial.println(calendarData);

    attempts++;
  } else {
    String reason = client->getStatusCode() + ": " + client->getReasonPhrase();
    Serial.println(reason);
  }

  delete client;

  return calendarData;
}

void manageStatus(String calendarData) {
  setColour(CRGB::Black);

  for (int i = 0; i < NBR_EVENTS; i++) {
    Serial.println("Checking for " + possibleEvents[i] + " bin day");
    if (calendarData.indexOf(possibleEvents[i], 0) >= 0) {
      setColour(led_colour[i]);
      binDay = i;
    }
  }
}

void fade(CHSV colour) {
  for(int i = 0; i <= 512; i++) {
    colour.value = sin8(i);
    setColour(colour);
    delay(PULSE_DELAY);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);

  setColour(CRGB::Yellow);

  Serial.println();

  connectToWiFi();

  ArduinoOTA.setHostname(OTA_NODE_NAME);
  ArduinoOTA.setPassword((const char *)(OTA_PASSWORD));
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  delay(5000);
}

void loop() {
  ArduinoOTA.handle();
  
  if (entryUpdated == 0 || millis() > entryUpdated + UPDATETIME) {
    String calendarData = getCalendar();
    manageStatus(calendarData);

    entryUpdated = millis();
  }

  if(binDay >= 0) {
    fade(led_colour[binDay]);
  }

  delay(10);
}
