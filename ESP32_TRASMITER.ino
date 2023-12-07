#include "BluetoothSerial.h"
#include <DHT.h>

BluetoothSerial SerialBT;

#define USE_NAME // Comment this to use MAC address instead of a slaveName
const char *pin = "1234"; // Change this to reflect the pin expected by the real slave BT device

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

#ifdef USE_NAME
String slaveName = "ESP32-BT-Slave"; // Change this to reflect the real name of your slave BT device
#else
String MACadd = "AA:BB:CC:11:22:33"; // This only for printing
uint8_t address[6] = {0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33}; // Change this to reflect the real MAC address of your slave BT device
#endif

String myName = "ESP32-BT-Master";

// DHT Setup
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Soil Moisture Setup
#define SOIL_MOISTURE_PIN 34 // Ganti dengan pin soil moisture yang sesuai

// Ultrasonic Sensor Setup
#define TRIGGER_PIN 5
#define ECHO_PIN 18
long duration;
int distance;

// Rain Sensor Setup
#define RAIN_SENSOR_PIN 32

void setup() {
  bool connected;
  Serial.begin(115200);
  SerialBT.begin(myName, true);
  Serial.printf("The device \"%s\" started in master mode, make sure slave BT device is on!\n", myName.c_str());

#ifndef USE_NAME
  SerialBT.setPin(pin);
  Serial.println("Using PIN");
#endif

  // connect(address) is fast (up to 10 secs max), connect(slaveName) is slow (up to 30 secs max) as it needs
  // to resolve slaveName to address first, but it allows connecting to different devices with the same name.
  // Set CoreDebugLevel to Info to view devices Bluetooth address and device names
#ifdef USE_NAME
  connected = SerialBT.connect(slaveName);
  Serial.printf("Connecting to slave BT device named \"%s\"\n", slaveName.c_str());
#else
  connected = SerialBT.connect(address);
  Serial.print("Connecting to slave BT device with MAC ");
  Serial.println(MACadd);
#endif

  if (connected)
  {
    Serial.println("Connected Successfully!");
  }
  else
  {
    while (!SerialBT.connected(10000))
    {
      Serial.println("Failed to connect. Make sure the remote device is available and in range, then restart the app.");
    }
  }

  // Setup pin modes
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);
}

void loop() {
  // Read sensor data
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int soilMoistureRaw = analogRead(SOIL_MOISTURE_PIN);

  // Convert soil moisture to percentage (0-100)
  int soilMoisture = map(soilMoistureRaw, 0, 4095, 0, 100);

  // Read ultrasonic sensor data
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;

  // Read rain sensor data
  int rainSensorValue = digitalRead(RAIN_SENSOR_PIN);

  // Send sensor data via Bluetooth
  SerialBT.print(temperature);
  SerialBT.print(",");
  SerialBT.print(humidity);
  SerialBT.print(",");
  SerialBT.print(soilMoisture);  // Mengirim soil moisture dalam bentuk persentase
  SerialBT.print(",");
  SerialBT.print(distance);
  SerialBT.print(",");
  SerialBT.print(rainSensorValue);
  SerialBT.println();

  // Echo sensor data to Serial Monitor
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Soil Moisture: ");
  Serial.print(soilMoisture);
  Serial.println("%");
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  Serial.print("Rain Sensor: ");
  Serial.println(rainSensorValue);

  delay(2000); // Delay for 2 seconds before reading the sensor again
}
