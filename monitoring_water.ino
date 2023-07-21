#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>

#define DO_PIN A1

// deklarasi sensor DO
#define VREF 5000    //VREF (mv)
#define ADC_RES 1024 //ADC Resolution

//Single-point calibration Mode=0
//Two-point calibration Mode=1
#define TWO_POINT_CALIBRATION 0

#define READ_TEMP (30) //Current water temperature ℃, Or temperature sensor function

//Single point calibration needs to be filled CAL1_V and CAL1_T
#define CAL1_V (1870) //mv
#define CAL1_T (25)   //℃
//Two-point calibration needs to be filled CAL2_V and CAL2_T
//CAL1 High temperature point, CAL2 Low temperature point
#define CAL2_V (1300) //mv
#define CAL2_T (15)   //℃

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
const int ntuMax = 100; // Nilai NTU maksimum yang akan dihasilkan

// Nilai voltase kalibrasi untuk air pH 7 dan pH 4
const float voltage_pH7 = 2.59; 
const float voltage_pH4 = 4.50;

// Nilai pH kalibrasi untuk air pH 7 dan pH 4
const float pH7 = 7.0;
const float pH4 = 4.0;

// parameter kalibrasi DO
const uint16_t DO_Table[41] = {
    14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
    11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
    9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
    7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410};

uint8_t Temperaturet;
uint16_t ADC_Raw;
uint16_t ADC_Voltage;
float DO_mgL;

int16_t readDO(uint32_t voltage_mv, uint8_t temperature_c)
{
#if TWO_POINT_CALIBRATION == 0
  uint16_t V_saturation = (uint32_t)CAL1_V + (uint32_t)35 * temperature_c - (uint32_t)CAL1_T * 35;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#else
  uint16_t V_saturation = (int16_t)((int8_t)temperature_c - CAL2_T) * ((uint16_t)CAL1_V - CAL2_V) / ((uint8_t)CAL1_T - CAL2_T) + CAL2_V;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#endif
}

void setup() {
  Serial.begin(115200); // Mulai serial monitor
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

  Temperaturet = (uint8_t)READ_TEMP;
  ADC_Raw = analogRead(DO_PIN);
  ADC_Voltage = uint32_t(VREF) * ADC_Raw / ADC_RES;
  DO_mgL = readDO(ADC_Voltage, Temperaturet) / 1000.0; // Convert DO to mg/L

  Serial.println("========================");
  Serial.print("Suhu Air:\t\t");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Kekeruhan Air:\t\t");
  Serial.print(ntuValue);
  Serial.println(" NTU");

  // Serial.print("Voltase: ");
  // Serial.print(voltage, 2);
  // Serial.print(" V\t");
  Serial.print("Kadar pH Air:\t\t");
  Serial.println(pHValue, 2);

  // Serial.print("Temperaturet:\t" + String(Temperaturet) + "\t");
  // Serial.print("ADC RAW:\t" + String(ADC_Raw) + "\t");
  // Serial.print("ADC Voltage:\t" + String(ADC_Voltage) + "\t");
  Serial.print("Kadar Oksigen Air:\t");
  Serial.print(DO_mgL, 2); // Print DO with 3 decimal places
  Serial.println(" mg/L");

  // pengolahan data menggunakan fuzzy logic

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
