#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include "config.h"

// NodeMCU ESP8266 y LED RGB de cátodo común.
const uint8_t PIN_DHT = D1, PIN_R = D5, PIN_G = D6, PIN_B = D7;
DHT dht(PIN_DHT, DHT11);
unsigned long ultimoEnvio = 0;

void aplicarColor(const String& color) {
  digitalWrite(PIN_R, color == "rojo" ? HIGH : LOW);
  digitalWrite(PIN_G, color == "verde" ? HIGH : LOW);
  digitalWrite(PIN_B, color == "azul" ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_R, OUTPUT); pinMode(PIN_G, OUTPUT); pinMode(PIN_B, OUTPUT);
  aplicarColor("apagado");
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Conectando a WiFi...");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) { delay(100); return; }
  if (millis() - ultimoEnvio < 5000) return;
  ultimoEnvio = millis();
  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();
  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("DHT11: lectura invalida; revisar cableado"); return;
  }
  StaticJsonDocument<192> datos;
  datos["dispositivo"] = DISPOSITIVO;
  datos["temperatura"] = temperatura;
  datos["humedad"] = humedad;
  String cuerpo; serializeJson(datos, cuerpo);
  WiFiClient cliente;
  HTTPClient http;
  if (!http.begin(cliente, API_URL)) { Serial.println("URL invalida"); return; }
  http.setTimeout(3000);
  http.addHeader("Content-Type", "application/json");
  int codigo = http.POST(cuerpo);
  String respuesta = http.getString();
  Serial.printf("HTTP %d | T=%.1f C | H=%.1f %%\n", codigo, temperatura, humedad);
  Serial.println(respuesta);
  if (codigo == 201) {
    StaticJsonDocument<384> resultado;
    if (!deserializeJson(resultado, respuesta)) {
      const char* color = resultado["led"] | "apagado";
      aplicarColor(String(color));
    }
  }
  http.end();
}
