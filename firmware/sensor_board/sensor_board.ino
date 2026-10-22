// Sensor board (ESP8266): VEML7700 lux sensor + 2x VL53L0X laser distance sensors.
// Reads the sensors every second and posts them to the FastAPI backend, plus a
// light/heavy vehicle classification. Original version as used in the demo.
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "Arduino.h"
#include "Wire.h"
#include "DFRobot_VL53L0X.h"
#include "DFRobot_VEML7700.h"
#include <Adafruit_Sensor.h>
#include "secrets.h"   // STASSID, STAPSK, BACKEND_HOST (see secrets.example.h)

const String DATA_API_URL = String(BACKEND_HOST) + "/datos";
const String VEHICLE_API_URL = String(BACKEND_HOST) + "/vehiculo";

const char* HEAVY = "{\"vehiculo\": \"pesado\"}";
const char* LIGHT = "{\"vehiculo\": \"vehiculo\"}";
const char* NONE = "{}";

DFRobot_VL53L0X sensor_distancia_1;
DFRobot_VL53L0X sensor_distancia_2;
DFRobot_VEML7700 sensor_luz;

WiFiClient client;

bool has_vehicle = false;

void setup() {
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  
  Wire.begin();

  // Both VL53L0X boot at the same I2C address: hold one in reset (XSHUT),
  // re-address the other, then release and address the second one.
  digitalWrite(D5, HIGH);
  digitalWrite(D6, LOW);
  delay(500);
  sensor_distancia_1.begin(0x30);
  delay(150);
  sensor_distancia_1.setMode(sensor_distancia_1.eSingle,sensor_distancia_1.eHigh);
  delay(150);
  digitalWrite(D5, HIGH);
  digitalWrite(D6, HIGH);
  delay(500);
  sensor_distancia_2.begin(0x31);
  delay(150);
  sensor_distancia_2.setMode(sensor_distancia_2.eSingle,sensor_distancia_2.eHigh);
  delay(150);

  sensor_luz.begin();
  sensor_luz.setGain(sensor_luz.ALS_GAIN_d4);
  sensor_luz.setIntegrationTime(sensor_luz.ALS_INTEGRATION_100ms);

  Serial.begin(9600);
  delay(100);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(STASSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  float luz;
  sensor_luz.getALSLux(luz);

  delay(150);
  sensor_distancia_1.start();
  delay(150);
  float distancia1 = sensor_distancia_1.getDistance();
  delay(150);
  sensor_distancia_1.stop();
  delay(300);
  sensor_distancia_2.start();
  delay(150);
  float distancia2 = sensor_distancia_2.getDistance();
  delay(150);
  sensor_distancia_2.stop();


  Serial.println(distancia1);
  Serial.println(distancia2);

  Serial.println("");
  Serial.println("Enviando datos al servidor...");

  if (WiFi.status() == WL_CONNECTED) {
    // Send sensor data
    HTTPClient http_data;
    http_data.begin(client, DATA_API_URL);
    

    http_data.addHeader("Content-Type", "application/json");

    String jsonBody = "{";
    jsonBody += "\"dato1\":" + String(luz, 2) + ",";
    jsonBody += "\"dato2\":" + String(0.0, 2) + ",";
    jsonBody += "\"dato3\":" + String(0.0, 2) + ",";
    jsonBody += "\"dato4\":" + String(0.0, 2) + ",";
    jsonBody += "\"dato5\":" + String(distancia2) + ",";
    jsonBody += "\"dato6\":" + String(distancia1);
    jsonBody += "}";

    int httpCode_data = http_data.POST(jsonBody);
    
    if (httpCode_data > 0) {
      Serial.println("Datos actualizados. Código HTTP: " + String(httpCode_data));
    } else {
      Serial.println("Error enviando datos: " + http_data.errorToString(httpCode_data));
    }
    
    http_data.end();

    // SEND VEHICLE DATA
    HTTPClient http_vehi;
    http_vehi.begin(client, VEHICLE_API_URL);
    

    http_vehi.addHeader("Content-Type", "application/json");

    int httpCode_vehi;
    if (luz > 100.)
      has_vehicle = false;
      httpCode_vehi = http_vehi.POST(NONE);
    if (distancia1 < 60. && distancia2 < 60.) {
      // pesado
      if (has_vehicle = false) httpCode_vehi = http_vehi.POST(HEAVY);

      has_vehicle = true;
    } else {
      // ligero
      if (has_vehicle = false) httpCode_vehi = http_vehi.POST(LIGHT);

      has_vehicle = true;
    }
    
    if (httpCode_vehi > 0) {
      Serial.println("Datos actualizados. Código HTTP: " + String(httpCode_vehi));
    } else {
      Serial.println("Error enviando datos: " + http_vehi.errorToString(httpCode_vehi));
    }
    
    http_vehi.end();
  } else {
    Serial.println("Error: WiFi desconectado");
  }

  delay(1000);
}
