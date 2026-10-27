#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <DHT.h>

const char* WIFI_SSID   = "Wokwi-GUEST";   // red virtual de Wokwi
const char* WIFI_PASS   = "";
// URL publica del tunel + /lecturas (NO /docs)
const char* API_URL     = "URL/lecturas";
const char* DISPOSITIVO = "equipo01";

const int PIN_DHT = 4;
const int PIN_R = 25, PIN_G = 26, PIN_B = 27;
const unsigned long INTERVALO_MS = 5000;

DHT dht(PIN_DHT, DHT22);

void aplicarColor(const String& color) {
  digitalWrite(PIN_R, color == "rojo");
  digitalWrite(PIN_G, color == "verde");
  digitalWrite(PIN_B, color == "azul");
}

// Busca en la respuesta el color que viene en el campo "led"
String extraerColor(const String& cuerpo) {
  int k = cuerpo.indexOf("\"led\"");
  if (k < 0) return "apagado";
  String resto = cuerpo.substring(k);
  if (resto.indexOf("rojo")  >= 0) return "rojo";
  if (resto.indexOf("verde") >= 0) return "verde";
  if (resto.indexOf("azul")  >= 0) return "azul";
  return "apagado";
}

void conectarWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS, 6);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  aplicarColor("apagado");
  dht.begin();
  conectarWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) conectarWiFi();

  // Hasta 3 intentos de lectura (el DHT necesita ~2 s entre lecturas)
  float temperatura = NAN, humedad = NAN;
  for (int i = 0; i < 3; i++) {
    temperatura = dht.readTemperature();
    humedad = dht.readHumidity();
    if (!isnan(temperatura) && !isnan(humedad)) break;
    Serial.println("DHT sin datos, reintentando...");
    delay(2000);
  }
  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("Lectura invalida del DHT: no se envia");
    delay(INTERVALO_MS);
    return;
  }

  char json[160];
  snprintf(json, sizeof(json),
    "{\"dispositivo\":\"%s\",\"temperatura\":%.1f,\"humedad\":%.1f}",
    DISPOSITIVO, temperatura, humedad);

  WiFiClientSecure client;
  client.setInsecure();  // solo laboratorio: no valida el certificado
  HTTPClient http;
  if (http.begin(client, API_URL)) {
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Tunnel-Skip-AntiPhishing-Page", "true");
    int codigo = http.POST(json);
    Serial.printf("POST %s -> HTTP %d\n", json, codigo);
    if (codigo > 0) {
      String cuerpo = http.getString();
      Serial.println(cuerpo);
      if (codigo == 201) aplicarColor(extraerColor(cuerpo));
    }
    http.end();
  } else {
    Serial.println("No se pudo iniciar la conexion");
  }
  delay(INTERVALO_MS);
}
