#include "MQ135.h"
#include <Adafruit_BMP280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

Adafruit_BMP280 bme; // I2C
const char* ssid     = "Aleksa";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "";     // The password of the Wi-Fi network
// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "https://influxurl"
// InfluxDB v2 server or cloud API token (Use: InfluxDB UI -> Data -> API Tokens -> <select token>)
#define INFLUXDB_TOKEN ""
// InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG ""
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET "air"

WiFiClientSecure wifiClient;

#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
Point sensor("sensors");
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);        // Start the Serial communication to send messages to the computer
  delay(10);
    // Wait for serial to initialize.
  while(!Serial) { }
  Serial.println('\n');
  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());  
 
  sensor.addTag("sensors", "misin_park");
  //sensor.addTag("SSID", DHTTYPE);
  bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
  // Clear fields for reusing the point. Tags will remain untouched
  sensor.clearFields();
    float p = bme.readPressure();
    // Read temperature as Celsius (the default)
    float t = bme.readTemperature();
    float h = 40; // hudamity in % correction factor for mq135
    MQ135 gasSensor = MQ135(A0);
    float air_quality = gasSensor.getPPM();
    float rzero = gasSensor.getRZero();

    Serial.print("Preassure: ");
    Serial.print(p);
    Serial.print(" hPa\n");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" °C\n");
    Serial.print(" °Air quality: ");
    Serial.print(air_quality);
    Serial.print(" PPM ");
    Serial.print(" \n");
    Serial.print(rzero);
    Serial.print(" \n");
  // Store measured value into point
  // Report RSSI of currently connected network
  sensor.addField("temperature", t);
  sensor.addField("preassure", p);
  sensor.addField("ppm", air_quality);
  sensor.addField("rzero", rzero);
  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());


  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  //Wait 10s
  Serial.println("Wait 1min");
  delay(60000);
}
