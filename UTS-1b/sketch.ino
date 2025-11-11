#include "DHT.h"

#define DHTPIN 8
#define DHTTYPE DHT22 

DHT dht(DHTPIN, DHTTYPE);

#define LED_HIJAU 5
#define LED_KUNING 10
#define LED_MERAH 12
#define RELAY_POMPA 7
#define BUZZER 9

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_KUNING, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);
  pinMode(RELAY_POMPA, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(RELAY_POMPA, LOW); // Pompa awal OFF
  digitalWrite(BUZZER, LOW);
}

void loop() {
  float suhu = dht.readTemperature();
  float lembab = dht.readHumidity();

  if (isnan(suhu) || isnan(lembab)) {
    Serial.println("Sensor tidak terbaca!");
    return;
  }

  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" °C, Kelembapan: ");
  Serial.print(lembab);
  Serial.println(" %");

  // Logika indikator berdasarkan suhu
  if (suhu > 35) {
    digitalWrite(LED_MERAH, HIGH);
    digitalWrite(LED_KUNING, LOW);
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(BUZZER, HIGH);  
  } 
  else if (suhu >= 30 && suhu <= 35) {
    digitalWrite(LED_MERAH, LOW);
    digitalWrite(LED_KUNING, HIGH);
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(BUZZER, LOW);
  } 
  else {
    digitalWrite(LED_MERAH, LOW);
    digitalWrite(LED_KUNING, LOW);
    digitalWrite(LED_HIJAU, HIGH);
    digitalWrite(BUZZER, LOW);
  }

  delay(2000); // baca tiap 2 detik
}
