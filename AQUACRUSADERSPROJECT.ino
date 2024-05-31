#include <WiFi.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ThingSpeak.h>

const char* ssid = "Redmi";
const char* password = "dina2003";
const char* server = "api.thingspeak.com";
const char* apiKey = "SR8HFW54ZOBHI3GL";
const long channelID = 2477079; // Replace with your ThingSpeak channel ID

// Declare a global WiFiClient object
WiFiClient client;

// Analog pin connected to the TDS sensor
const int TDS_SENSOR_PIN = 34;

// Analog pin connected to the pH sensor
const int pH_SENSOR_PIN = 35;

// Data wire is connected to GPIO 4 on ESP32
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Calibration variables
float pHCalibrationOffset = 0.0; // Initial offset for pH calibration
float TDS_CALIBRATION_FACTOR = 1.0; // Initial calibration factor for TDS sensor

void setup() {
  Serial.begin(9600);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");

  // Initialize ThingSpeak
  ThingSpeak.begin(client);

  // Initialize Dallas Temperature sensor
  sensors.begin();
  
  // Calibrate pH sensor
  calibratePHSensor();
  
  // Calibrate TDS sensor
  calibrateTDSSensor();
}

void loop() {
  // Read sensor values
  float tdsValue = readTDSSensor();
  float pHValue = readPHSensor();
  float waterTempC = readWaterTemperature();

  // Apply calibration offset to pH value
  pHValue += pHCalibrationOffset;

  // Apply calibration factor to TDS value
  tdsValue *= TDS_CALIBRATION_FACTOR;

  // Print sensor values to serial monitor
  Serial.print("TDS Value: ");
  Serial.println(tdsValue);
  Serial.print("pH Value: ");
  Serial.println(pHValue);
  Serial.print("Water Temperature: ");
  Serial.print(waterTempC);
  Serial.println(" °C");

  // Send data to ThingSpeak
  if (sendToThingSpeak(pHValue, tdsValue, waterTempC)) {
    Serial.println("Data sent to ThingSpeak!");
  } else {
    Serial.println("Failed to send data to ThingSpeak!");
  }

  delay(10000); // Wait for 15 seconds before sending next data
}

float readTDSSensor() {
  int sensorValue = analogRead(TDS_SENSOR_PIN);
  float voltage = sensorValue * (5.0 / 4095.0);
  float tdsValue = voltage * 100; // Example conversion, adjust according to your sensor
  return tdsValue;
}

float readPHSensor() {
  int sensorValue = analogRead(pH_SENSOR_PIN);
  float voltage = sensorValue * (5.0 / 4095.0);
  float pH = mapVoltageToPH(voltage); // Convert voltage to pH
  return pH;
}

float mapVoltageToPH(float voltage) {
  // Implement your mapping function here
  // This function should accurately map voltage readings to pH values based on your sensor's calibration
  // Consult your pH sensor's datasheet or calibration instructions for the appropriate mapping
  return voltage; // Placeholder value, replace with actual mapping
}

float readWaterTemperature() {
  sensors.requestTemperatures();
  float waterTempC = sensors.getTempCByIndex(0);
  return waterTempC;
}

bool sendToThingSpeak(float pHValue, float tdsValue, float waterTempC) {
  // Create a data structure to hold the field data
  ThingSpeak.setField(1, pHValue);
  ThingSpeak.setField(2, tdsValue);
  ThingSpeak.setField(3, waterTempC);
  
  // Send data to ThingSpeak
  if (ThingSpeak.writeFields(2477079, "SR8HFW54ZOBHI3GL")) {
    return true; // Data sent successfully
  } else {
    return false; // Failed to send data
  }
}

void calibratePHSensor() {
  Serial.println("Calibrating pH sensor...");
  
  // You need to perform a calibration procedure here
  // This could involve placing the sensor in known pH buffer solutions and adjusting the pH value accordingly
  // For simplicity, let's assume we have a known offset value for calibration
  
  // For example, if your sensor consistently reads 0.2 pH units lower than the actual pH value, you can set the calibration offset to 0.2
  pHCalibrationOffset = 7.0;
  
  Serial.println("pH sensor calibrated.");
}

void calibrateTDSSensor() {
  Serial.println("Calibrating TDS sensor to 500 ppm...");
  
  // Measure the sensor readings for the standard solution
  float standardSolutionConcentration = 500; // Standard solution concentration in ppm
  float measuredReading = readTDSSensor(); // Measure the sensor reading
  
  // Calculate the calibration factor
  TDS_CALIBRATION_FACTOR = standardSolutionConcentration / measuredReading;
  
  Serial.print("TDS sensor calibrated with a calibration factor of ");
  Serial.println(TDS_CALIBRATION_FACTOR);
}
