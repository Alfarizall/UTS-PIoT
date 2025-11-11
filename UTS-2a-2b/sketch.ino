#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// ==================== PIN SETUP ====================
#define DHTPIN 27
#define DHTTYPE DHT22
#define LDR_PIN 35      // pin analog untuk LDR
#define RELAY_PIN 33    // pin digital untuk relay

// ==================== WIFI & MQTT ====================
const char* ssid = "Wokwi-GUEST";     
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";  

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastMsg = 0;

// ==================== WIFI SETUP ====================
void setup_wifi() {
  delay(10);
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
  Serial.println(WiFi.localIP());
}

// ==================== MQTT CALLBACK ====================
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.println(msg);

  // jika ada pesan dari topic relay, kendalikan relay manual
  if (String(topic) == "esp32/ijal/relay") {
    if (msg.indexOf("ON") >= 0) {
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Relay ON");
    } else if (msg.indexOf("OFF") >= 0) {
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Relay OFF");
    }
  }
}

// ==================== MQTT RECONNECT ====================
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32ClientWokwi")) {
      Serial.println("connected");
      client.subscribe("esp32/ijal/relay");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // relay awal OFF

  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// ==================== LOOP ====================
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) { // kirim setiap 5 detik
    lastMsg = now;

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int ldrValue = analogRead(LDR_PIN);  // baca nilai cahaya (0-4095)
    float lux = ldrValue * 100.0 / 4095.0; // konversi ke persentase float

    if (isnan(temp) || isnan(hum)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // ===== Format relay sebagai "ON"/"OFF" =====
    String relayState = digitalRead(RELAY_PIN) ? "ON" : "OFF";

    // ===== Kirim data ke MQTT =====
    String payload = "{\"suhu\": " + String(temp, 2) +
                     ", \"humidity\": " + String(hum, 2) +
                     ", \"lux\": " + String(lux, 2) +
                     ", \"relay_state\": \"" + relayState + "\"" +
                     "}";

    Serial.print("Publishing message: ");
    Serial.println(payload);

    client.publish("esp32/ijal/data", payload.c_str());
  }
}
