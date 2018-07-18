#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme; // I2C

int timezone = 1;
int dst = 0;

const char* ssid = "***********";
const char* password = "************";
const char* host = "********.execute-api.eu-west-1.amazonaws.com";
const int httpsPort = 443;
const char* fingerprint = "ff 67 36 7c 5c d4 de 4a e1 8b cc e1 d7 0f da bd 7c 86 61 35";

static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];
static char hoursminutes[5];
int hours;
int minutes;
int year;
int month;
int day;

void setup() {
  Serial.begin(115200);
  delay(10);

  Wire.begin(D3, D4);
  Wire.setClock(100000);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  configTime(timezone * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");

  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

}

void loop() {

  time_t now;
  struct tm * timeinfo;
  time(&now);
  timeinfo = localtime(&now);  
  Serial.print(timeinfo->tm_hour);
  Serial.print(":");
  Serial.println(timeinfo->tm_min);

  year=timeinfo->tm_year + 1900;

  month=timeinfo->tm_mon +1;
  String s_month = String(month);
  if (s_month.length() < 2) {
    s_month = "0" + s_month;
  } 

  day=timeinfo->tm_mday;
  String s_day = String(day);
  if (s_day.length() < 2) {
    s_day = "0" + s_day;
  } 

  String s_date = year + s_month + s_day;

  hours=timeinfo->tm_hour;
  minutes=timeinfo->tm_min;
  String s_hours = String(hours);
  if (s_hours.length() < 2) {
    s_hours = "0" + s_hours;
  } 
  String s_minutes = String(minutes);
  if (s_minutes.length() < 2) {
    s_minutes = "0" + s_minutes;
  } 
  String hoursminutes = s_hours + ":" + s_minutes;

  Serial.println();
  Serial.println("Current date and time - fixed:");
  Serial.print(s_date);
  Serial.print(" ");
  Serial.print(hoursminutes);

  float h = bme.readHumidity();
  float t = bme.readTemperature();

  dtostrf(h, 5, 1, humidityTemp);
  dtostrf(t, 5, 1, celsiusTemp);
      
  StaticJsonBuffer<400> JSONbuffer;
  JsonObject& meranie = JSONbuffer.createObject();
  meranie["date"] = s_date;
  meranie["time"] = hoursminutes;
  meranie["humidity"] = humidityTemp;
  meranie["temperature"] = celsiusTemp;
  meranie["room"] = "bedroom";

  meranie.printTo(Serial);

  char JSONmessageBuffer[400];
  meranie.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println(JSONmessageBuffer);

  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  String url = "/Demo/sensors";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("POST ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n");
  client.println("Content-Type: application/json");
  // Content length
  int length = meranie.measureLength();
  client.print("Content-Length:"); client.println(length);
  // End of headers
  client.println();
  // POST message body
  String out;
  meranie.printTo(out);
  client.println(out);

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");

  Serial.println("Going into deep sleep for 5 minutes");
  //ESP.deepSleep(300e6); // 300e6 is 300 microseconds
  delay(60000);  //<--- either delay or deepSleep (not working right now)
}   
