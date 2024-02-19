

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char* home_ssid = "qwerty";
const char* home_password = "12345678";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600*3;
const int daylightOffset_sec = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

AsyncWebServer server(80);

int sunriseTime = 360;
int sunsetTime = 1320;
int brightness = 100;

const int UF_LED = D5; // Pin for the LED
const int RED_LED = D5; // Pin for the LED
const int GREEN_LED = D5; // Pin for the LED


void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(LED, OUTPUT);
  analogWrite(D6, 255);
  analogWrite(D7, 0);
  WiFi.begin(home_ssid, home_password);
  int check = 0;
  while (WiFi.status() != WL_CONNECTED && check < 10) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    check++;
  }
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected to WiFi");
    analogWrite(D6, 0);
    analogWrite(D7, 255);
  }
  else
  {
    Serial.println("Failed to connect to Wi-Fi");
    analogWrite(D6, 255);
    analogWrite(D7, 0);
  }


  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  timeClient.begin();
  timeClient.update();

  pinMode(LED, OUTPUT); // Assuming an LED is connected to pin D5

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request){
    String html = "<html>";
    html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>";
    html += "<body style=\"text-align: center;\">";
    html += "<h1>Fitolamp control page</h1>";
    html += "<p>Current time: " + timeClient.getFormattedTime() + "</p>";
    html += "<form action=\"/set_times\">";
    html += "Sunrise time (HH:MM): <input type=\"text\" name=\"sunrise_time\"><br>";
    html += "Sunset time (HH:MM) : <input type=\"text\" name=\"sunset_time\"><br>";
    html += "Brightness (0-100): <input type=\"number\" name=\"brightness\" min=\"0\" max=\"100\"><br>";
    html += "<input type=\"submit\" value=\"Submit\">";
    html += "</form>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/set_times", HTTP_GET, [](AsyncWebServerRequest* request){
    String sunriseValue = request->getParam("sunrise_time")->value();
    String sunsetValue = request->getParam("sunset_time")->value();
    String brightnessValue = request->getParam("brightness")->value();
    if(getTimeInMin(sunriseValue)!=0)
      sunriseTime = getTimeInMin(sunriseValue);
    if(getTimeInMin(sunsetValue)!=0)
      sunsetTime = getTimeInMin(sunsetValue);
    if(brightnessValue.toInt()!=0)
      brightness = brightnessValue.toInt();

    request->redirect("/");
  });

  server.begin();
}

void loop() {
  timeClient.begin();
  timeClient.update();

  int currentTime = getTimeInMin(timeClient.getFormattedTime());

  if (currentTime >= sunriseTime && currentTime <= sunsetTime) {
    analogWrite(LED, brightness * 255 / 100);
  } 
  else 
  {
    analogWrite(LED, 0);
  }


  Serial.print(currentTime);
  Serial.print(" ");
  Serial.print(sunriseTime);
  Serial.print(" ");
  Serial.println(sunsetTime);
  checkWiFiConnection();
  delay(1000);
}

// Function to convert a time string in HH:MM format to seconds
int getTimeInSeconds(String timeString) {
  int colonIndex = timeString.indexOf(":");
  int hours = timeString.substring(0, colonIndex).toInt();
  int minutes = timeString.substring(colonIndex + 1).toInt();
  return hours * 60 * 60 + minutes * 60;
}

int getTimeInMin(String timeString) {
  int colonIndex = timeString.indexOf(":");
  int hours = timeString.substring(0, colonIndex).toInt();
  int minutes = timeString.substring(colonIndex + 1).toInt();
  return hours * 60  + minutes;
}

void checkWiFiConnection() {
  int check = 0;
  if (WiFi.status() == WL_CONNECTED) 
  {
    //Serial.println("Connected to Wi-Fi");
    analogWrite(D6, 0);
    analogWrite(D7, 255);
  } else 
  {
    Serial.println("Failed to connect to Wi-Fi");
    analogWrite(D6, 255);
    analogWrite(D7, 0);
    while (WiFi.status() != WL_CONNECTED && check < 10) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
      check++;
    }
  }
}
