#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

char auth[] = "zwfZy5P7OMzqbGZ40WAaMYXZgjFDxq86";
char ssid[] = "NSJ";
char pass[] = "11111111";

#define soil A0         // Soil Moisture Sensor
#define PIR D5          // PIR Motion Sensor
#define rainDrop A1     // Raindrop Sensor
#define RELAY_PIN_1 D3  // Relay
#define PUSH_BUTTON_1 D7 // Button
#define VPIN_BUTTON_1 V4

DHT dht(D4, DHT11);

int PIR_ToggleValue;
int relay1State = LOW;
int pushButton1State = HIGH;

BlynkTimer timer;

void setup() {
  Serial.begin(9600);
  pinMode(PIR, INPUT);
  pinMode(rainDrop, INPUT);
  pinMode(RELAY_PIN_1, OUTPUT);
  digitalWrite(RELAY_PIN_1, LOW);
  pinMode(PUSH_BUTTON_1, INPUT_PULLUP);
  digitalWrite(RELAY_PIN_1, relay1State);

  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  dht.begin();

  timer.setInterval(100L, soilMoistureSensor);
  timer.setInterval(100L, DHT11sensor);
  timer.setInterval(100L, raindropSensor);
  timer.setInterval(500L, checkPhysicalButton);
}

void DHT11sensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
}

void soilMoistureSensor() {
  int value = analogRead(soil);
  value = map(value, 0, 1024, 0, 100);
  value = (value - 100) * -1;
  Blynk.virtualWrite(V3, value);
}

void PIRsensor() {
  bool value = digitalRead(PIR);
  if (value) {
    Blynk.logEvent("pirmotion", "WARNING! Motion Detected!");
    WidgetLED LED(V5);
    LED.on();
  } else {
    WidgetLED LED(V5);
    LED.off();
  }
}

void raindropSensor() {
  int rainValue = analogRead(rainDrop);
  if (rainValue < 500) { // Adjust threshold as per sensor
    Blynk.notify("It's raining! Take care of your plants.");
  }
}

BLYNK_WRITE(V6) {
  PIR_ToggleValue = param.asInt();
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(VPIN_BUTTON_1);
}

BLYNK_WRITE(VPIN_BUTTON_1) {
  relay1State = param.asInt();
  digitalWrite(RELAY_PIN_1, relay1State);
}

void checkPhysicalButton() {
  if (digitalRead(PUSH_BUTTON_1) == LOW) {
    if (pushButton1State != LOW) {
      relay1State = !relay1State;
      digitalWrite(RELAY_PIN_1, relay1State);
      Blynk.virtualWrite(VPIN_BUTTON_1, relay1State);
    }
    pushButton1State = LOW;
  } else {
    pushButton1State = HIGH;
  }
}

void loop() {
  if (PIR_ToggleValue == 1) {
    PIRsensor();
  }

  Blynk.run();
  timer.run();
}
