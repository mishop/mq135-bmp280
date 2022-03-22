# mq135-bmp280
ESP8266 with mq134 and BMP280 sensors send datat to InfluxDB

## Setup paramters
const char* ssid     = ;         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = ;     // The password of the Wi-Fi network
#define INFLUXDB_URL 
#define INFLUXDB_TOKEN 
#define INFLUXDB_ORG 
#define INFLUXDB_BUCKET

# MQ135 Setup 
In mq135.h lib file set rload - resistence, rzero correction factor.
MQ135(uint8_t pin, float rzero=4.0, float rload=10.0);
