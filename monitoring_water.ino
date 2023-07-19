#include <OneWire.h>
#include <DallasTemperature.h>

const int pinData = 4; // Ubah sesuai dengan pin yang Anda gunakan
OneWire oneWire(pinData);
DallasTemperature sensors(&oneWire);

const int turbidityPin = A0; // Pin analog untuk membaca nilai kekeruhan
const int relayPin = 13; // Pin digital untuk mengontrol relay
const int pHpin = A1; // Pin analog untuk membaca sensor pH

// Parameter kalibrasi sensor turbidity
const int jernihMin = 843;
const int keruhSedikitMin = 835;
const int keruhMin = 825;
const int sangatKeruhMin = 810;
const int sangatKeruhSekaliMin = 790;

// Konstanta kalibrasi
const int ntuMax = 200; // Nilai NTU maksimum yang akan dihasilkan

// Nilai voltase kalibrasi untuk air pH 7 dan pH 4
const float voltage_pH7 = 2.51; 
const float voltage_pH4 = 2.20;

// Nilai pH kalibrasi untuk air pH 7 dan pH 4
const float pH7 = 7.0;
const float pH4 = 4.0;

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

  float voltage = analogRead(pHpin) * (5.0 / 1023.0); // Baca voltase dari sensor pH
  float pHValue = calibratepH(voltage); // Konversi voltase menjadi nilai pH

  Serial.print("Suhu pada air: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Kekeruhan pada air: ");
  Serial.print(ntuValue);
  Serial.println(" NTU");

  Serial.print("Voltase: ");
  Serial.print(voltage, 2);
  Serial.print(" V\t");
  
  Serial.print("pH: ");
  Serial.println(pHValue, 2);

  // Cek kondisi suhu, kekeruhan, dan pH
  if (temperature > 35 || ntuValue > 400 || pHValue < 5 || pHValue > 9) {
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

float calibratepH(float voltage) {
  // Interpolasi linear untuk mengkalibrasi nilai pH
  float pH = pH7 + ((voltage - voltage_pH7) / (voltage_pH4 - voltage_pH7)) * (pH4 - pH7);
  return pH;
}
