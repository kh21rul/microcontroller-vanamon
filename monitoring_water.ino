#include <Arduino.h> // Library preprocessor directive
#include <OneWire.h> // Library ds18b20
#include <DallasTemperature.h>
#include "WiFiEsp.h" // Library untuk ESP8266-01

// Deklarasi variabel-variabel fuzzy logic
// deklarasi Keanggotaan himpunan oksigen
float oksigenRendah[] = {4, 8}; // <4, 4, 8
float oksigenTinggi[] = {4, 8}; // 4, 8, 8>
// deklarasi Keanggotaan himpunan suhu
float suhuDingin[] = {25, 30};     // <25, 25, 30
float suhuNetral[] = {25, 30, 35}; // 25, 30, 35
float suhuPanas[] = {30, 35};      // 30, 35, 35>
// deklarasi Keanggotaan himpunan output aerator
float aeratorMati[] = {500, 1200};  // <500, 500, 1200
float aeratorHidup[] = {500, 1200}; // 500, 1200, 1200>
// deklarasi Keanggotaan himpunan turbidity
float turbiJernih[] = {25, 400}; // <25, 25, 400
float turbiKeruh[] = {25, 400};  // 25, 400, 400>
// deklarasi Keanggotaan himpunan pH
float pHAsam[] = {6, 7};      // <6, 6, 7
float pHNetral[] = {6, 7, 8}; // 6, 7, 8
float pHBasa[] = {7, 8};      // 7, 8, 8>
// deklarasi Keanggotaan himpunan output waterpump
float waterpumpMati[] = {500, 1200};  // <500, 500, 1200
float waterpumpHidup[] = {500, 1200}; // 500, 1200, 1200>

float MUoksigenRen, MUoksigenTin;      // deklarasi variabel derajat keanggotaan oksigen
float MUsuhuDin, MUsuhuNet, MUsuhuPan; // deklarasi variabel derajat keanggotaan suhu
float MUturbiJer, MUturbiKer;          // deklarasi variabel derajat keanggotaan turbidity
float MUpHAs, MUpHNet, MUpHBas;        // deklarasi variabel derajat keanggotaan pH

float R1aerator, R2aerator, R3aerator, R4aerator, R5aerator, R6aerator;             // deklarasi variabel Rules untuk pengendalian Aerator
float Z1aerator, Z2aerator, Z3aerator, Z4aerator, Z5aerator, Z6aerator;             // deklarasi variabel Rules untuk pengendalian Aerator
float R1waterpump, R2waterpump, R3waterpump, R4waterpump, R5waterpump, R6waterpump; // deklarasi variabel Rules untuk pengendalian Water Pump
float Z1waterpump, Z2waterpump, Z3waterpump, Z4waterpump, Z5waterpump, Z6waterpump; // deklarasi variabel Rules untuk pengendalian Water Pump
float OutputAerator, OutputWaterpump;
String waterPump; // menampung string on/off Water Pump dan Aerator
String aerator;

// Deklarasi Variabel Sensor Suhu
const int pinData = 4;
OneWire oneWire(pinData);
DallasTemperature sensors(&oneWire);
float nilaiSuhu; // Menampung nilai Suhu pada function
float TempValue; // Menampung Value suhu

const int turbidityPin = A0; // Deklarasi Sensor Turbidity
const int jernihMin = 843;   // Parameter kalibrasi sensor turbidity
const int keruhSedikitMin = 835;
const int keruhMin = 825;
const int sangatKeruhMin = 810;
const int sangatKeruhSekaliMin = 790;
const int ntuMax = 100; // Nilai NTU maksimum yang akan dihasilkan
float nilaiKeruh;       // Menampung nilai kekeruhan pada function
float TurbiValue;       // Menampung nilai Keruh

// Deklarasi Sensor pH
const int ph_Pin = A1; // Pin analog untuk membaca sensor pH
float nilaipH = 0;
float PH_step;
int nilai_analog_PH;
double TeganganPh;
float PH4 = 1.6;
float PH7 = 2.5;
float pHValue; // Menampung nilai pH

// deklarasi sensor DO
#define DO_PIN A2
#define VREF 5000    // VREF (mv)
#define ADC_RES 1024 // ADC Resolution
// Single-point calibration Mode=0
// Two-point calibration Mode=1
#define TWO_POINT_CALIBRATION 0
#define READ_TEMP (30) // Current water temperature ℃, Or temperature sensor function
// Single point calibration needs to be filled CAL1_V and CAL1_T
#define CAL1_V (1870) // mv
#define CAL1_T (25)   // ℃
// Two-point calibration needs to be filled CAL2_V and CAL2_T
// CAL1 High temperature point, CAL2 Low temperature point
#define CAL2_V (1300) // mv
#define CAL2_T (15)   // ℃
const uint16_t DO_Table[41] = {
    14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
    11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
    9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
    7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410};
uint8_t Temperaturet;
uint16_t ADC_Raw;
uint16_t ADC_Voltage;
float DOValue, DO_mgL;

const int relayWaterPump = 13; // Deklarasi Pin Relay Water Pump
const int relayAerator = 12;   // Deklarasi Pin Relay Aerator

// Deklarasi Modul Wifi esp-01
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(2, 3); // RX, TX
#endif

char ssid[] = "Wifi Gratis"; // your network SSID (name)
char pass[] = "12345678";    // your network password
int status = WL_IDLE_STATUS; // the Wifi radio's status

char server[] = "monitoring.kh21rul.site";

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10000L; // delay between updates, in milliseconds

// Initialize the Ethernet client object
WiFiEspClient client;

void setup()
{
  pinMode(ph_Pin, INPUT); // Inisialisasi pH sebagai Input
  pinMode(relayWaterPump, OUTPUT);
  pinMode(relayAerator, OUTPUT);

  sensors.begin(); // Mulai komunikasi dengan sensor Suhu

  Serial.begin(115200); // initialize serial for debugging
  Serial1.begin(9600);  // initialize serial for ESP module
  WiFi.init(&Serial1);  // initialize ESP module

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true)
      ;
  }

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");

  printWifiStatus();
}

void loop()
{
  // TempValue = takeTemperature(); // ambil data suhu
  // TurbiValue = takeTurbidity();  // ambil data turbidity
  // pHValue = takepH();            // ambil data pH
  // DOValue = takeDO();            // ambil data DO

  // Input nilai Variabel
  float TempValue = 38;
  float TurbiValue = 90;
  float pHValue = 7;
  float DOValue = 7.5;

  // Hitung Fuzzy Output Aerator menggunakan fungsi
  OutputAerator = fuzzyTsukamotoAerator(DOValue, TempValue);
  // Hitung Fuzzy Output Water Pump menggunakan fungsi
  OutputWaterpump = fuzzyTsukamotoWaterPump(TurbiValue, pHValue);

  // Cek on/off Aerator
  if (OutputAerator >= 750)
  {
    aerator = "Hidup";
  }
  else
  {
    aerator = "Mati";
  }

  // Cek on/off Water Pump
  if (OutputWaterpump >= 750)
  {
    waterPump = "Hidup";
    digitalWrite(relayWaterPump, LOW);
  }
  else
  {
    waterPump = "Mati";
    digitalWrite(relayWaterPump, HIGH);
  }

  String data = String(TempValue, 2) + "/" +
                String(TurbiValue, 2) + "/" +
                String(pHValue, 2) + "/" +
                String(DOValue, 2) + "/" +
                waterPump + "/" +
                aerator;

  Serial.print("Suhu Air:\t\t");
  Serial.print(TempValue);
  Serial.println(" °C");

  Serial.print("Kekeruhan Air:\t\t");
  Serial.print(TurbiValue);
  Serial.println(" NTU");

  Serial.print("Kadar pH Air:\t\t");
  Serial.println(pHValue, 2);

  Serial.print("Kadar Oksigen Air:\t");
  Serial.print(DOValue, 2); // Print DO with 3 decimal places
  Serial.println(" mg/L");

  Serial.print("Pompa Air:\t\t");
  Serial.println(waterPump);

  Serial.print("Aerator:\t\t");
  Serial.println(aerator);
  Serial.println("========================");
  Serial.println();

  // if there's incoming data from the net connection send it out the serial port
  // this is for debugging purposes only
  while (client.available())
  {
    char c = client.read();
    Serial.write(c);
  }

  if (millis() - lastConnectionTime > postingInterval)
  {
    httpRequest(data);
  }

  delay(2000);
}

// this method makes a HTTP connection to the server
void httpRequest(String data)
{
  Serial.println();

  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80))
  {
    Serial.println("Connecting...");

    // send the HTTP GET request with the provided data
    client.print(F("GET /simpan/"));
    client.print(data);
    client.println(F(" HTTP/1.1"));
    client.println(F("Host: monitoring.kh21rul.site"));
    client.println(F("Connection: close"));
    client.println();

    // note the time that the connection was made
    lastConnectionTime = millis();
  }
  else
  {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

float takeTemperature()
{                                         // Fungsi pengambilan suhu dari sensor
  sensors.requestTemperatures();          // Minta sensor untuk membaca suhu
  nilaiSuhu = sensors.getTempCByIndex(0); // Baca suhu dalam Celsius
  return nilaiSuhu;
}

float takeTurbidity()
{                                        // Fungsi pengambilan kekeruhan dari sensor
  nilaiKeruh = analogRead(turbidityPin); // Baca nilai kekeruhan dari sensor
  if (nilaiKeruh >= jernihMin)           // konversi nilai kekeruhan ke NTU
  {
    return 0; // Jernih, nilai NTU = 0
  }
  else if (nilaiKeruh >= keruhSedikitMin)
  {
    // Persamaan linier untuk konversi ke NTU di rentang keruh sedikit
    return map(nilaiKeruh, keruhSedikitMin, jernihMin - 1, 1, ntuMax);
  }
  else if (nilaiKeruh >= keruhMin)
  {
    // Persamaan linier untuk konversi ke NTU di rentang keruh
    return map(nilaiKeruh, keruhMin, keruhSedikitMin - 1, ntuMax + 1, ntuMax * 2);
  }
  else if (nilaiKeruh >= sangatKeruhMin)
  {
    // Persamaan linier untuk konversi ke NTU di rentang sangat keruh
    return map(nilaiKeruh, sangatKeruhMin, keruhMin - 1, ntuMax * 2 + 1, ntuMax * 3);
  }
  else
  {
    // Persamaan linier untuk konversi ke NTU di rentang sangat keruh sekali
    return map(nilaiKeruh, sangatKeruhSekaliMin, sangatKeruhMin - 1, ntuMax * 3 + 1, ntuMax * 4);
  }
}

float takepH()
{                                       // Fungsi pengambilan pH dari sensor
  nilai_analog_PH = analogRead(ph_Pin); // Baca nilai analog dari sensor pH
  TeganganPh = 5 / 1024.0 * nilai_analog_PH;
  PH_step = (PH4 - PH7) / 3;
  nilaipH = 7.00 + ((PH7 - TeganganPh) / PH_step);
  return nilaipH;
}

float takeDO()
{
  // Ambil data dari sensor DO
  Temperaturet = (uint8_t)READ_TEMP;
  ADC_Raw = analogRead(DO_PIN);
  ADC_Voltage = uint32_t(VREF) * ADC_Raw / ADC_RES;
  DO_mgL = readDO(ADC_Voltage, Temperaturet) / 1000.0; // Convert DO to mg/L
  return DO_mgL;
}

// Function baca DO
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

// Deklarasi fungsi untuk menghitung Fuzzy Tsukamoto Aerator
float fuzzyTsukamotoAerator(float DOValue, float TempValue)
{
  // Proses Fuzzyfikasi Kendali Aerator
  // Mencari Fungsi Keanggotaan Oksigen Rendah
  if (DOValue >= oksigenRendah[1])
  {
    MUoksigenRen = 0;
  }
  else if (DOValue > oksigenRendah[0] && DOValue < oksigenRendah[1])
  {
    MUoksigenRen = (oksigenRendah[1] - DOValue) / (oksigenRendah[1] - oksigenRendah[0]);
  }
  else if (DOValue <= oksigenRendah[0])
  {
    MUoksigenRen = 1;
  }
  // Mencari Fungsi Keanggotaan Oksigen Tinggi
  if (DOValue <= oksigenTinggi[0])
  {
    MUoksigenTin = 0;
  }
  else if (DOValue > oksigenTinggi[0] && DOValue < oksigenTinggi[1])
  {
    MUoksigenTin = (DOValue - oksigenTinggi[0]) / (oksigenTinggi[1] - oksigenTinggi[0]);
  }
  else if (DOValue >= oksigenTinggi[1])
  {
    MUoksigenTin = 1;
  }
  // Mencari Fungsi Keanggotaan Suhu Dingin
  if (TempValue >= suhuDingin[1])
  {
    MUsuhuDin = 0;
  }
  else if (TempValue > suhuDingin[0] && TempValue < suhuDingin[1])
  {
    MUsuhuDin = (suhuDingin[1] - TempValue) / (suhuDingin[1] - suhuDingin[0]);
  }
  else if (TempValue <= suhuDingin[0])
  {
    MUsuhuDin = 1;
  }
  // Mencari Fungsi Keanggotaan Suhu Netral
  if (TempValue <= suhuNetral[0] || TempValue >= suhuNetral[2])
  {
    MUsuhuNet = 0;
  }
  else if (TempValue > suhuNetral[0] && TempValue < suhuNetral[1])
  {
    MUsuhuNet = (TempValue - suhuNetral[0]) / (suhuNetral[1] - suhuNetral[0]);
  }
  else if (TempValue >= suhuNetral[1] && TempValue < suhuNetral[2])
  {
    MUsuhuNet = (suhuNetral[2] - TempValue) / (suhuNetral[2] - suhuNetral[1]);
  }
  // Mencari Fungsi Keanggotaan Suhu Panas
  if (TempValue <= suhuPanas[0])
  {
    MUsuhuPan = 0;
  }
  else if (TempValue > suhuPanas[0] && TempValue < suhuPanas[1])
  {
    MUsuhuPan = (TempValue - suhuPanas[0]) / (suhuPanas[1] - suhuPanas[0]);
  }
  else if (TempValue >= suhuPanas[1])
  {
    MUsuhuPan = 1;
  }

  // Proses Inferensi Kendali aerator
  // [R1] Jika oksigen rendah & suhu dingin, maka aerator hidup
  if (MUoksigenRen < MUsuhuDin)
  {
    R1aerator = MUoksigenRen;
  }
  else if (MUsuhuDin < MUoksigenRen)
  {
    R1aerator = MUsuhuDin;
  }
  Z1aerator = aeratorHidup[0] + ((aeratorHidup[1] - aeratorHidup[0]) * R1aerator); // Mencari Z1 dari Rule 1

  // [R2] Jika oksigen rendah & suhu netral, maka aerator hidup
  if (MUoksigenRen < MUsuhuNet)
  {
    R2aerator = MUoksigenRen;
  }
  else if (MUsuhuNet < MUoksigenRen)
  {
    R2aerator = MUsuhuNet;
  }
  Z2aerator = aeratorHidup[0] + ((aeratorHidup[1] - aeratorHidup[0]) * R2aerator); // Mencari Z2 dari Rule 2

  // [R3] Jika oksigen rendah & suhu panas, maka aerator hidup
  if (MUoksigenRen < MUsuhuPan)
  {
    R3aerator = MUoksigenRen;
  }
  else if (MUsuhuPan < MUoksigenRen)
  {
    R3aerator = MUsuhuPan;
  }
  Z3aerator = aeratorHidup[0] + ((aeratorHidup[1] - aeratorHidup[0]) * R3aerator); // Mencari Z3 dari Rule 3

  // [R4] Jika oksigen tinggi & suhu dingin, maka aerator hidup
  if (MUoksigenTin < MUsuhuDin)
  {
    R4aerator = MUoksigenTin;
  }
  else if (MUsuhuDin < MUoksigenTin)
  {
    R4aerator = MUsuhuDin;
  }
  Z4aerator = aeratorHidup[0] + ((aeratorHidup[1] - aeratorHidup[0]) * R4aerator); // Mencari Z4 dari Rule 4

  // [R5] Jika oksigen tinggi & suhu netral, maka aerator mati
  if (MUoksigenTin < MUsuhuNet)
  {
    R5aerator = MUoksigenTin;
  }
  else if (MUsuhuNet < MUoksigenTin)
  {
    R5aerator = MUsuhuNet;
  }
  Z5aerator = aeratorMati[1] - ((aeratorMati[1] - aeratorMati[0]) * R5aerator); // Mencari Z5 dari Rule 5

  // [R6] Jika oksigen tinggi & suhu panas, maka aerator hidup
  if (MUoksigenTin < MUsuhuPan)
  {
    R6aerator = MUoksigenTin;
  }
  else if (MUsuhuPan < MUoksigenTin)
  {
    R6aerator = MUsuhuPan;
  }
  Z6aerator = aeratorHidup[0] + ((aeratorHidup[1] - aeratorHidup[0]) * R6aerator); // Mencari Z6 dari Rule 6

  // Defuzzyfikasi Kendali Aerator
  OutputAerator = ((Z1aerator * R1aerator) + (Z2aerator * R2aerator) + (Z3aerator * R3aerator) + (Z4aerator * R4aerator) + (Z5aerator * R5aerator) + (Z6aerator * R6aerator)) / (R1aerator + R2aerator + R3aerator + R4aerator + R5aerator + R6aerator);

  return OutputAerator;
}

// Deklarasi fungsi untuk menghitung Fuzzy Tsukamoto Water Pump
float fuzzyTsukamotoWaterPump(float TurbiValue, float pHValue)
{
  // Proses Fuzzyfikasi Kendali Waterpump
  // Mencari Fungsi Keanggotaan Turbidity jernih
  if (TurbiValue >= turbiJernih[1])
  {
    MUturbiJer = 0;
  }
  else if (TurbiValue > turbiJernih[0] && TurbiValue < turbiJernih[1])
  {
    MUturbiJer = (turbiJernih[1] - TurbiValue) / (turbiJernih[1] - turbiJernih[0]);
  }
  else if (TurbiValue <= turbiJernih[0])
  {
    MUturbiJer = 1;
  }
  // Mencari FUngsi Keanggotaan Turbidity Keruh
  if (TurbiValue <= turbiKeruh[0])
  {
    MUturbiKer = 0;
  }
  else if (TurbiValue > turbiKeruh[0] && TurbiValue < turbiKeruh[1])
  {
    MUturbiKer = (TurbiValue - turbiKeruh[0]) / (turbiKeruh[1] - turbiKeruh[0]);
  }
  else if (TurbiValue >= turbiKeruh[1])
  {
    MUturbiKer = 1;
  }
  // Mencari Fungsi Keanggotaan pH Asam
  if (pHValue >= pHAsam[1])
  {
    MUpHAs = 0;
  }
  else if (pHValue > pHAsam[0] && pHValue < pHAsam[1])
  {
    MUpHAs = (pHAsam[1] - pHValue) / (pHAsam[1] - pHAsam[0]);
  }
  else if (pHValue <= pHAsam[0])
  {
    MUpHAs = 1;
  }
  // // Mencari Fungsi Keanggotaan pH Netral
  if (pHValue <= pHNetral[0] || pHValue >= pHNetral[2])
  {
    MUpHNet = 0;
  }
  else if (pHValue > pHNetral[0] && pHValue < pHNetral[1])
  {
    MUpHNet = (pHValue - pHNetral[0]) / (pHNetral[1] - pHNetral[0]);
    // MUpHNet = 100;
  }
  else if (pHValue >= pHNetral[1] && pHValue < pHNetral[2])
  {
    MUpHNet = (pHNetral[2] - pHValue) / (pHNetral[2] - pHNetral[1]);
  }
  // // Mencari Fungsi Keanggotaan pH Basa
  if (pHValue <= pHBasa[0])
  {
    MUpHBas = 0;
  }
  else if (pHValue > pHBasa[0] && pHValue < pHBasa[1])
  {
    MUpHBas = (pHValue - pHBasa[0]) / (pHBasa[1] - pHBasa[0]);
  }
  else if (pHValue >= pHBasa[1])
  {
    MUpHBas = 1;
  }

  // Proses Inferensi Kendali Water Pump
  //[R1] Jika kekeruhan jernih & pH asam, maka waterpump hidup
  if (MUturbiJer < MUpHAs)
  {
    R1waterpump = MUturbiJer;
  }
  else if (MUpHAs < MUturbiJer)
  {
    R1waterpump = MUpHAs;
  }
  Z1waterpump = waterpumpHidup[0] + ((waterpumpHidup[1] - waterpumpHidup[0]) * R1waterpump); // Mencari Z1 dari Rule 1

  // [R2] Jika kekeruhan jernih & pH netral, maka waterpump mati
  if (MUturbiJer < MUpHNet)
  {
    R2waterpump = MUturbiJer;
  }
  else if (MUpHNet < MUturbiJer)
  {
    R2waterpump = MUpHNet;
  }
  Z2waterpump = waterpumpHidup[1] - ((waterpumpHidup[1] - waterpumpHidup[0]) * R2waterpump); // Mencari Z2 dari Rule 2

  // [R3] Jika kekeruhan jernih & pH basa, maka waterpump hidup
  if (MUturbiJer < MUpHBas)
  {
    R3waterpump = MUturbiJer;
  }
  else if (MUpHBas < MUturbiJer)
  {
    R3waterpump = MUpHBas;
  }
  Z3waterpump = waterpumpHidup[0] + ((waterpumpHidup[1] - waterpumpHidup[0]) * R3waterpump); // Mencari Z3 dari Rule 3

  // [R4] Jika kekeruhan keruh & pH asam, maka waterpump hidup
  if (MUturbiKer < MUpHAs)
  {
    R4waterpump = MUturbiKer;
  }
  else if (MUpHAs < MUturbiKer)
  {
    R4waterpump = MUpHAs;
  }
  Z4waterpump = waterpumpHidup[0] + ((waterpumpHidup[1] - waterpumpHidup[0]) * R4waterpump); // Mencari Z4 dari Rule 4

  // [R5] Jika kekeruhan keruh & pH netral, maka waterpump hidup
  if (MUturbiKer < MUpHNet)
  {
    R5waterpump = MUturbiKer;
  }
  else if (MUpHNet < MUturbiKer)
  {
    R5waterpump = MUpHNet;
  }
  Z5waterpump = waterpumpHidup[0] + ((waterpumpHidup[1] - waterpumpHidup[0]) * R5waterpump); // Mencari Z5 dari Rule 5

  // [R6] Jika kekeruhan keruh & pH basa, maka waterpump hidup
  if (MUturbiKer < MUpHBas)
  {
    R6waterpump = MUturbiKer;
  }
  else if (MUpHBas < MUturbiKer)
  {
    R6waterpump = MUpHBas;
  }
  Z6waterpump = waterpumpHidup[0] + ((waterpumpHidup[1] - waterpumpHidup[0]) * R6waterpump); // Mencari Z6 dari Rule 6

  // Defuzzyfikasi Kendali Water Pump
  OutputWaterpump = ((Z1waterpump * R1waterpump) + (Z2waterpump * R2waterpump) + (Z3waterpump * R3waterpump) + (Z4waterpump * R4waterpump) + (Z5waterpump * R5waterpump) + (Z6waterpump * R6waterpump)) / (R1waterpump + R2waterpump + R3waterpump + R4waterpump + R5waterpump + R6waterpump);

  return OutputWaterpump;
}