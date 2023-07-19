#include <OneWire.h>
#include <DallasTemperature.h>

const int pinData = 4; // Ubah sesuai dengan pin yang Anda gunakan
OneWire oneWire(pinData);
DallasTemperature sensors(&oneWire);

const int turbidityPin = A0; // Pin analog untuk membaca nilai kekeruhan
const int relayPin = 13; // Pin digital untuk mengontrol relay

// Parameter kalibrasi sensor turbidity
const int jernihMin = 843;
const int keruhSedikitMin = 835;
const int keruhMin = 825;
const int sangatKeruhMin = 810;
const int sangatKeruhSekaliMin = 790;

// Konstanta kalibrasi
const int ntuMax = 200; // Nilai NTU maksimum yang akan dihasilkan

void setup() {
  Serial.begin(9600); // Mulai serial monitor
  sensors.begin();   // Mulai komunikasi dengan sensor suhu
  pinMode(relayPin, OUTPUT); // Mengatur pin relay sebagai output
}

void loop() {
  sensors.requestTemperatures(); // Minta sensor suhu untuk membaca suhu
  float temperature = sensors.getTempCByIndex(0); // Baca suhu dalam Celsius

  int turbidityValue = analogRead(turbidityPin); // Baca nilai kekeruhan
  int ntuValue = convertToNTU(turbidityValue); // Konversi ke NTU

  Serial.print("Suhu pada air: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Kekeruhan pada air: ");
  Serial.print(ntuValue);
  Serial.println(" NTU");
  
  // Cek kondisi suhu dan kekeruhan
  if (temperature > 35 || ntuValue > 400) {
    digitalWrite(relayPin, LOW); // Nyalakan relay
  } else {
    digitalWrite(relayPin, HIGH); // Matikan relay
  }

  delay(1000); // Tunda 1 detik sebelum membaca data berikutnya
}

int convertToNTU(int turbidityValue) {
  if (turbidityValue >= jernihMin) {
    return 0; // Jernih, nilai NTU = 0
  } else if (turbidityValue >= keruhSedikitMin) {
    // Persamaan linier untuk konversi ke NTU di rentang keruh sedikit
    return map(turbidityValue, keruhSedikitMin, jernihMin - 1, 1, ntuMax);
  } else if (turbidityValue >= keruhMin) {
    // Persamaan linier untuk konversi ke NTU di rentang keruh
    return map(turbidityValue, keruhMin, keruhSedikitMin - 1, ntuMax + 1, ntuMax * 2);
  } else if (turbidityValue >= sangatKeruhMin) {
    // Persamaan linier untuk konversi ke NTU di rentang sangat keruh
    return map(turbidityValue, sangatKeruhMin, keruhMin - 1, ntuMax * 2 + 1, ntuMax * 3);
  } else {
    // Persamaan linier untuk konversi ke NTU di rentang sangat keruh sekali
    return map(turbidityValue, sangatKeruhSekaliMin, sangatKeruhMin - 1, ntuMax * 3 + 1, ntuMax * 4);
  }
}
