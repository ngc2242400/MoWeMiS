// Import the necessary libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_BME680.h>
#include <SPI.h>

#include <Pinger.h>
#include <ESP8266WiFi.h>

int FirstLoop = 0;

// Define the pin where the temperature sensor is connected
const int oneWireBus = 0;

// Create a one-wire object
OneWire oneWire(oneWireBus);

// Create a DallasTemperature library object
DallasTemperature sensors(&oneWire);

Adafruit_BME680 bme; // I2C
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);
#define SEALEVELPRESSURE_HPA (1021.00)

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  //pinMode(vent_pin, OUTPUT);
  #define vent 14  // GPIO14 = Wemos Pin D5 
  pinMode(vent, OUTPUT);    // sets the digital pin D5 vent
  
  // Initialize the temperature sensor
  sensors.begin();

  while (!Serial);
  Serial.println("");
  Serial.println("BME680 async test");

  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}


void loop() {
  
   // This is needed at the top of the loop!
          //mqttClient.loop();
         if (FirstLoop < 1){
            Serial.println("Boot complete starting application");
            FirstLoop = 1;
            Serial.println("Flushing sensor box");
            digitalWrite(vent, HIGH); 
            delay(6000);//6000
            digitalWrite(vent, LOW); 
            }
  
  // Read the temperature from the sensor
  sensors.requestTemperatures();

  // Get the temperature in Celsius
  float temperatureC = sensors.getTempCByIndex(0);

  // Print the temperature to the serial monitor
  Serial.print("Externe temperature: ");
  Serial.print(temperatureC);
  Serial.println("C  ");
  
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V) by " * (5.0 / 1023.0)":
  float voltage = sensorValue;
  // print out the value you read:
  Serial.print("VOC value = ");
  Serial.print(voltage);
  Serial.println(" units");

    // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  
  Serial.print(F("Interne temperature = "));
  Serial.print(bme.temperature);
  Serial.println(F(" *C"));

  Serial.print(F("Pressure = "));
  Serial.print(bme.pressure / 100.0);
  Serial.println(F(" hPa"));

  Serial.print(F("Humidity = "));
  Serial.print(bme.humidity);
  Serial.println(F(" %"));

  Serial.print(F("Gas = "));
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(F(" KOhms"));

  Serial.print(F("Approx. Altitude = "));
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(F(" m"));

  Serial.println();  
  delay(1000);
}

