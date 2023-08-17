// Import the necessary libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_BME680.h>
#include <SPI.h>

#include <Pinger.h>
#include <ESP8266WiFi.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <elapsedMillis.h>

// global var
int FirstLoop = 0;
int livePuls;
int beat = 0;
int connectionAtemped = 0;
int mobSensHoldPreBreath = 1;
int mobSensBreath = 10;
int mobSensHoldPostBreath = 5;
WiFiClient WIFI_CLIENT;
char buffer[12];         //the ASCII of the integer will be stored in this char array

// Function prototypes
void subscribeReceive(char* topic, byte* payload, unsigned int length);

// Ethernet and MQTT related objects
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
Pinger pinger;
int pingResult;

// DEFINE HERE THE KNOWN NETWORKS
const char* KNOWN_SSID[] = {"MoWeMiS","WiFi-2.4-09A8 Accespoint", "WiFi-2.4-09A8","Cat S62 Pro_1587"};
const char* KNOWN_PASSWORD[] = {"******","*******", "*******","*******"};
const int   KNOWN_SSID_COUNT = sizeof(KNOWN_SSID) / sizeof(KNOWN_SSID[0]); // number of known networks

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

void wait(int seconds) //no cpu lock wait
{
  elapsedMillis waiting;     // "waiting" starts at zero
  waiting = 0; 
    while (waiting < 1000 * seconds){ 
      //Serial.println(waiting);
      yield();
    }
    waiting = 0; 
}
void subscribeReceive(char* topic, byte* payload, unsigned int length)
{
  // Print the topic
  Serial.print("Topic: ");
  Serial.println(topic);
 
  // Print the message
  Serial.print("Message: ");
  for(int i = 0; i < length; i ++)
  {
    Serial.print(char(payload[i]));
  }
  String strTopic(*topic);
  if (strTopic == "r"){
    Serial.println("topic x");
    if (payload[0] =='1'){
      Serial.println("Reset ontvangen");
      Serial.println(payload[0]);                                                                                                                                                                                              
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("Sens2")) {
      Serial.println("connected");
      connectionAtemped = 0;
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      connectionAtemped = connectionAtemped +1;
      if (connectionAtemped > 0){
        Serial.println("Atempting restart");
        ESP.restart();
      }
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  boolean wifiFound = false;
  // Initialize serial communication
  Serial.begin(115200);

  //pinMode(vent_pin, OUTPUT);
  #define vent 14  // GPIO14 = Wemos Pin D5 
  pinMode(vent, OUTPUT);    // sets the digital pin D5 vent


  // ----------------------------------------------------------------
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  // ----------------------------------------------------------------
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("Setup done");

  // ----------------------------------------------------------------
  // WiFi.scanNetworks will return the number of networks found
  // ----------------------------------------------------------------
  Serial.println(F("scan start"));
  int nbVisibleNetworks = WiFi.scanNetworks();
  Serial.println(F("scan done"));
  if (nbVisibleNetworks == 0) {
    Serial.println(F("no networks found. Reset to try again"));
    while (true); // no need to go further, hang in there, will auto launch the Soft WDT reset
  }

  // ----------------------------------------------------------------
  // if you arrive here at least some networks are visible
  // ----------------------------------------------------------------
  Serial.print(nbVisibleNetworks);
  Serial.println(" network(s) found");

  // ----------------------------------------------------------------
  // check if we recognize one by comparing the visible networks
  // one by one with our list of known networks
  // ----------------------------------------------------------------
  int i;
  int n;
  for (i = 0; i < nbVisibleNetworks; ++i) {
    Serial.println(WiFi.SSID(i)); // Print current SSID
    for (n = 0; n < KNOWN_SSID_COUNT; n++) { // walk through the list of known SSID and check for a match
      if (strcmp(KNOWN_SSID[n], WiFi.SSID(i).c_str())) {
        Serial.print(F("\tNot matching "));
        Serial.println(KNOWN_SSID[n]);
      } else { // we got a match
        wifiFound = true;
        break; // n is the network index we found
      }
    } // end for each known wifi SSID
    if (wifiFound) break; // break from the "for each visible network" loop
  } // end for each visible network

  if (!wifiFound) {
    Serial.println(F("no Known network identified. Reset to try again"));
    while (true); // no need to go further, hang in there, will auto launch the Soft WDT reset
  }

  // ----------------------------------------------------------------
  // if you arrive here you found 1 known SSID
  // ----------------------------------------------------------------
  Serial.print(F("\nConnecting to "));
  Serial.println(KNOWN_SSID[n]);
  
  // ----------------------------------------------------------------
  // We try to connect to the WiFi network we found
  // ----------------------------------------------------------------
  WiFi.begin(KNOWN_SSID[n], KNOWN_PASSWORD[n]);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  // ----------------------------------------------------------------
  // SUCCESS, you are connected to the known WiFi network
  // ----------------------------------------------------------------
  WiFi.hostname("MoWeMiS");
  Serial.println(F("WiFi connected, your IP address is "));
  Serial.println(WiFi.localIP());

  // ----------------------------------------------------------------
  // Set MQTT server based on WifiNetwork
  // ----------------------------------------------------------------
  Serial.println("Wifi counter: ");
  Serial.println(n);
  if (n>0){
    mqttClient.setServer("192.168.1.43", 1883);
    Serial.println("MQTT server: 192.168.1.43");
 }
 if (n=0){
    mqttClient.setServer("192.168.1.58", 1884);
    Serial.println("MQTT server: 192.168.2.2");
    }
  mqttClient.setClient(WIFI_CLIENT);
  mqttClient.setCallback(subscribeReceive);
  mqttClient.connect("MoWeMiS");

  while (!mqttClient.connected()) {
  Serial.print("Connecting to MQTT...");
  Serial.print("connection state: ");
  Serial.println(mqttClient.state());
  delay(300);
  mqttClient.connect("MoWeMiS");
  connectionAtemped = connectionAtemped +1;
    if (connectionAtemped > 3){
      Serial.println("Atempting restart");
      ESP.restart();
    }
 }
  if (mqttClient.connect("MoWeMiS")) {
    // connection succeeded
    Serial.println("Connected ");
    connectionAtemped = 0;
    mqttClient.publish("CommStatus", "(re)connected");
    
  } 
  else {
    // connection failed
    // mqttClient.state() will provide more information
    // on why it failed.
    Serial.println("Connection failed ");
    
  }
  
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
  mqttClient.loop();
          
  if (FirstLoop < 1){
    Serial.println("Boot complete starting application");
    FirstLoop = 1;
    Serial.println("Flushing sensor box");
    digitalWrite(vent, HIGH); 
    delay(6000);//6000
    digitalWrite(vent, LOW); 
    livePuls = -1;
    }

    if (!mqttClient.connected()) {
    reconnect();   
   } 
  
  // Read the temperature from the sensor
  sensors.requestTemperatures();

  // Get the temperature in Celsius
  float temperatureC = sensors.getTempCByIndex(0);

  // Print the temperature to the serial monitor
  Serial.print("Externe temperature: ");
  Serial.print(temperatureC);
  Serial.println("C  ");
  itoa(temperatureC,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("MoWeMiSextTemp", buffer,1);
//  
  
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V) by " * (5.0 / 1023.0)":
  float voltage = sensorValue;
  // print out the value you read:
  Serial.print("VOC value = ");
  Serial.print(voltage);
  Serial.println(" units");
  itoa(voltage,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("MoWeMiSVOC", buffer,1);

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
  itoa(bme.temperature,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("MoWeMiStemperature", buffer,1);


  Serial.print(F("Pressure = "));
  Serial.print(bme.pressure / 100.0);
  Serial.println(F(" hPa"));
  itoa(bme.pressure,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("MoWeMiSpressure", buffer,1);


  Serial.print(F("Humidity = "));
  Serial.print(bme.humidity);
  Serial.println(F(" %"));
  itoa(bme.humidity,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("MoWeMiShumidity", buffer,1);

  Serial.print(F("Gas = "));
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(F(" KOhms"));
  itoa(bme.gas_resistance,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("MoWeMiSgas_resistance", buffer,1);

  Serial.print(F("Approx. Altitude = "));
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(F(" m"));

  Serial.println(); 
  // send alive puls
  livePuls = livePuls * -1;
  itoa(livePuls,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("MoWeMiSPuls", buffer,1);
  Serial.print("livePuls: "); 
  Serial.println(livePuls); 
  delay(1000);
}

