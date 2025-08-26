// esp32_rest.ino — ESP32 đọc DHT22 và gửi JSON (nhiệt độ + độ ẩm + timestamp) tới FastAPI server
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <time.h>

// ==== Cấu hình mạng ====
const char* WIFI_SSID = "B9313dientuvienthong";
const char* WIFI_PASS = "quatangcuocsong";
const char* SERVER_IP = "192.168.1.20";   // IP LAN server
const uint16_t SERVER_PORT = 8000;
const char* API_KEY = "changeme";         // trùng với server.py
// ========================

// ---- DHT22 ----
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ---- NTP config ----
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;  // GMT+7 cho Việt Nam
const int daylightOffset_sec = 0;     // không dùng DST

// Endpoint
String endpoint() {
  return String("http://") + SERVER_IP + ":" + String(SERVER_PORT) + "/sensor";
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("[wifi] Connecting");
  uint8_t tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
    if (++tries > 60) { Serial.println("\n[wifi] Rebooting"); ESP.restart(); }
  }
  Serial.printf("\n[wifi] OK. IP: %s\n", WiFi.localIP().toString().c_str());
}

void setup() {
  Serial.begin(115200);
  delay(300);

  connectWiFi();

  // Bắt đầu DHT
  dht.begin();
  Serial.println("[dht] started");

  // Bắt đầu đồng bộ thời gian qua NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("[ntp] syncing time...");
  delay(2000);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();

  // Đọc DHT
  float tempC = dht.readTemperature();
  float hum   = dht.readHumidity();

  // Lấy thời gian hiện tại (epoch seconds)
  time_t now = time(nullptr);
  if (now < 100000) {
    Serial.println("[ntp] chưa đồng bộ thời gian, bỏ qua lần gửi này");
    delay(2000);
    return;
  }

  // JSON
  StaticJsonDocument<192> doc;
  doc["id"] = "esp32";
  if (!isnan(tempC)) { doc["value"] = tempC; doc["unit"] = "C"; }
  if (!isnan(hum))   { doc["humidity"] = hum; }
  doc["ts"] = (int)now;   // thời gian hiện tại từ NTP

  String body;
  serializeJson(doc, body);

  // HTTP POST
  HTTPClient http;
  http.begin(endpoint());
  http.addHeader("Content-Type", "application/json");
  if (API_KEY && strlen(API_KEY) > 0) http.addHeader("X-API-Key", API_KEY);
  http.setTimeout(5000);

  int code = http.POST(body);
  String resp = http.getString();
  Serial.printf("[http] POST => %d, resp=%s | temp=%.2fC hum=%.2f%% ts=%ld\n",
                code, resp.c_str(),
                isnan(tempC)?-999:tempC,
                isnan(hum)?-999:hum,
                now);
  http.end();

  delay(5000);
}
