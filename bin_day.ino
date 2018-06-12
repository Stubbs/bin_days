#include <dummy.h>

#include "HTTPSRedirect.h"
#include <SPI.h>
#include <ESP8266WiFi.h>


#define NBR_EVENTS 3
//#define UPDATETIME 3600000
#define UPDATETIME 30000

#define BLACKBIN 0
#define BLUEBIN 1

#define PULSE_DELAY 15

const int REDPIN   = D2;
const int GREENPIN = D3;
const int BLUEPIN  = D4;

String possibleEvents[NBR_EVENTS] = {"black", "blue", "yellow"};
byte led_colour[NBR_EVENTS][3] = {{255, 255, 255}, {0, 0, 255}, {255, 255, 0}};
byte led_off[3] = {0, 0, 0};

//Connection Settings
const char* host = "script.google.com";
const char* googleRedirHost = "script.googleusercontent.com";
const int httpsPort = 443;

unsigned long entryUpdated = 0;

String url = String("/macros/s/") + calendar_id + "/exec";
int attempts = 0;
int binDay = -1;

HTTPSRedirect* client = nullptr;

void setColour(byte colour[3]) {
  analogWrite(REDPIN, colour[0]);
  analogWrite(GREENPIN, colour[1]);
  analogWrite(BLUEPIN, colour[2]);
}

void connectToWiFi() {
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print(" connected ");
  Serial.println(WiFi.localIP());
}

String getCalendar() {
  String calendarData = "";
  
  Serial.println("Start Request for calendar.");

  client = new HTTPSRedirect(httpsPort);
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  if (!client->connected()) client->connect(host, httpsPort);

  if(client->GET(url, host)) {
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
  setColour(led_off);

  for (int i = 0; i < NBR_EVENTS; i++) {
    Serial.println("Checking for " + possibleEvents[i] + " bin day");
    if (calendarData.indexOf(possibleEvents[i], 0) >= 0) {
      setColour(led_colour[i]);
      binDay = i;
    }
  }
}

void fadeWhite() {
  byte currentColour[3] = {255, 255, 255};
  for (int i = 255; i >= 50; i--) {
    currentColour[0] = i;
    currentColour[1] = i;
    currentColour[2] = i;
    setColour(currentColour);
    delay(PULSE_DELAY);
  }

  for (int i = 50; i <= 255; i++) {
    currentColour[0] = i;
    currentColour[1] = i;
    currentColour[2] = i;
    setColour(currentColour);
    delay(PULSE_DELAY);
  }
}

void fadeBlue() {
  byte currentColour[3] = {0, 0, 255};
  for (int i = 255; i >= 50; i--) {
    currentColour[2] = i;
    setColour(currentColour);
    delay(PULSE_DELAY);
  }

  for (int i = 50; i <= 255; i++) {
    currentColour[2] = i;
    setColour(currentColour);
    delay(PULSE_DELAY);
  }
}

void setup() {
  Serial.begin(9600);

  setColour(led_colour[2]);

  Serial.println();

  connectToWiFi();

  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);

  delay(5000);
}

void loop() {
  if (entryUpdated == 0 || millis() > entryUpdated + UPDATETIME) {
    String calendarData = getCalendar();
    manageStatus(calendarData);

    entryUpdated = millis();
  }

  switch (binDay) {
    case BLACKBIN:
      fadeWhite();
      break;
    case BLUEBIN:
      fadeBlue();
      break;
    
  }

  delay(10);
}
