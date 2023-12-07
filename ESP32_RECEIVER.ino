#include "BluetoothSerial.h"
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

BluetoothSerial SerialBT;
DHT dht(4, DHT11); // Pin DHT11 terhubung ke pin 4 pada ESP32

// Inisialisasi objek LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // Alamat I2C LCD 0x27, 16 kolom, 2 baris

const int ledPin = 5; // Example pin for the LED
const float siaga1RangeStart = 8.1;
const float siaga1RangeEnd = 11.0;
const float siaga2RangeStart = 6.1;
const float siaga2RangeEnd = 8.0;
const float siaga3RangeStart = 2.0;
const float siaga3RangeEnd = 6.0;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32-BT-Slave"); // Nama Bluetooth slave device
  Serial.println("Bluetooth Slave is ready to receive data!");

  dht.begin(); // Inisialisasi sensor DHT11

  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ESP32-BT-Monitor");

  pinMode(ledPin, OUTPUT);
}

void displaySlide1(float temperature, float humidity) {
  Serial.println("Slide 1 - Temperature: " + String(temperature) + ", Humidity: " + String(humidity));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);

  delay(2000);
}

void displaySlide2(int soilMoisture, float ultrasonicDistance) {
  Serial.println("Slide 2 - Soil Moisture: " + String(soilMoisture) + ", Ultrasonic: " + String(ultrasonicDistance));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SoilMoisture: ");
  lcd.print(soilMoisture);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Ultrasonic: ");
  lcd.print(ultrasonicDistance);
  lcd.print(" cm");

  delay(2000);
}

void displaySlide3(int rainSensorValue) {
  Serial.println("Slide 3 - Rain Sensor: " + String(rainSensorValue));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print((rainSensorValue == 1) ? "Hujan" : "Tidak Hujan");

  delay(2000);
}

void displaySlide4(float ultrasonicDistance, int soilMoisture) {
  int alertLevel = 0;

  if (ultrasonicDistance >= siaga1RangeStart && ultrasonicDistance <= siaga1RangeEnd) {
    alertLevel = 1;
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Siaga 1");
  } else if (ultrasonicDistance >= siaga2RangeStart && ultrasonicDistance <= siaga2RangeEnd) {
    alertLevel = 2;
    for (int i = 0; i < 2; i++) {
      digitalWrite(ledPin, HIGH);
      delay(500);
      digitalWrite(ledPin, LOW);
      delay(500);
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Siaga 2");
  } else if (ultrasonicDistance >= siaga3RangeStart && ultrasonicDistance <= siaga3RangeEnd) {
    alertLevel = 3;
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH);
      delay(500);
      digitalWrite(ledPin, LOW);
      delay(500);
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Siaga 3");
  } else {
    alertLevel = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Normal");
  }

  lcd.setCursor(0, 1);
  lcd.print(" ");
  lcd.print((soilMoisture >= 31) ? "Tanah Kering" : "Tanah Basah");

  Serial.println("Slide 4 - Alert Level: " + String(alertLevel));
  delay(2000);
}

void loop() {
  static unsigned long lastUpdateTime = 0;
  static const unsigned long updateInterval = 2000; // Interval waktu update (ms)

  if (SerialBT.available()) {
    String data = SerialBT.readStringUntil('\n');
    Serial.println("Received data: " + data);

    // Contoh: Memisahkan data dan mengonversi ke variabel
    float temperature, humidity, ultrasonicDistance;
    int soilMoisture, rainSensorValue;
    sscanf(data.c_str(), "%f,%f,%d,%f,%d", &temperature, &humidity, &soilMoisture, &ultrasonicDistance, &rainSensorValue);

    // Tampilkan data di Serial Monitor dan LCD
    displaySlide1(temperature, humidity);
    displaySlide2(soilMoisture, ultrasonicDistance);
    displaySlide3(rainSensorValue);
    displaySlide4(ultrasonicDistance, soilMoisture);

    // Perbarui waktu terakhir pembaruan
    lastUpdateTime = millis();
  }

  // Tunggu 2 detik sejak waktu terakhir pembaruan sebelum membaca lagi
  if (millis() - lastUpdateTime >= updateInterval) {
    delay(2000); // Waktu delay sebelum membaca data lagi
  }
}