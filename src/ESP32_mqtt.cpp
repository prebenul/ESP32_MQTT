#include <Arduino.h>
#include <WiFi.h>
#include <WiFiSTA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Button2.h>
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include <SSD1306Wire.h>        // legacy: #include "SSD1306.h"
#include <ESPRotary.h>
#include <ESP32Servo.h>
#include <config.h>

/////////////////////////////////////////////////////////////////


SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_32);   // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h

/////////////////////////////////////////////////////////////////


Button2 button = Button2(BUTTON_PIN); // connect button

ESPRotary r = ESPRotary(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP); //define rotary encoder

Servo myservo;

/////////////////////////////////////////////////////////////////

int pos = 0;
int servoPin = 13;
char ssid[] = SECRET_SSID;
char password[] = SECRET_PASS;
char mqttServer[] = MQTT_ADDRESS;
char mqttUser[] = MQTT_USERNAME;
char mqttPassword[] = MQTT_PASS;
double a = 0;
double b = 0;


WiFiClient espClient;
PubSubClient client(espClient);

/////////////////////////////////////////////////////////////////

void push(unsigned int pos) {
  client.publish("rpi", String(pos).c_str(), true);
}

void rotate(ESPRotary& r) {
  Serial.println(r.getPosition());
  push(r.getPosition());
}

void tap(Button2& btn) {
  myservo.write(b);
}
void resetPosition(Button2& btn) {
  r.resetPosition();
  Serial.println("Reset!");
  push(r.getPosition());
}

void callback(char* topic, byte* payload, unsigned int length) {

  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload, length);

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  long id = doc["id"];
  long row = doc["row"];
  double  width = doc["width"];
  double  outerdiam = doc["outerdiam"];
  long segs = doc["segs"];

  a = outerdiam * PI;
  b = a / segs;
  Serial.println(b);

  display.clear();
  display.drawString(0, 0, "Rw: " + String(row) + " -Lng " + String(b));
  display.drawString(0, 11, "Width: " + String(width));
  display.drawString(0, 22, String(outerdiam) + " - segs: " + String(segs));
  display.display();

  Serial.println(id);
  Serial.println(row);
  Serial.println(width, 6);
  Serial.println();
  Serial.println("-----------------------");

}

/////////////////////////////////////////////////////////////////

void oled(String f) {
  display.clear();
  display.drawString(0, 0, String(f));
  display.drawString(0, 48, String(WiFi.localIP().toString()));
  display.display();
}

void setup() {

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(servoPin, 500, 2500);

  button.setTapHandler(tap);
  button.setLongClickHandler(resetPosition);
  r.setChangedHandler(rotate);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);


  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
    oled("WiFi starting..");
  }
  Serial.println("Connected to the WiFi network");
  oled("WiFi OK");
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    oled("MQTT starting");
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {

      Serial.println("connected");
      oled("Wifi & MQTT OK");
    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }

  client.subscribe("esp/test");

}




void loop() {
  client.loop();
  button.loop();
  r.loop();
}
