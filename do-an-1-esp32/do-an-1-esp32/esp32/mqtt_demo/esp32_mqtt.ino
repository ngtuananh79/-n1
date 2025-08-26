// esp32_mqtt.ino — publish / subscribe MQTT qua LAN
#include <WiFi.h>
#include <PubSubClient.h>

// ==== Sửa cho phù hợp mạng của bạn ====
const char* WIFI_SSID = "YourWiFi";
const char* WIFI_PASS = "YourPass";
const char* MQTT_BROKER = "192.168.1.10";   // IP LAN broker Mosquitto
const uint16_t MQTT_PORT = 1883;
const char* MQTT_USER = "";                 // đặt nếu broker có user/pass
const char* MQTT_PASS = "";
// =====================================

WiFiClient espClient;
PubSubClient client(espClient);

const char* TOPIC_PUB = "lab/sensor/esp32";
const char* TOPIC_SUB = "lab/cmd/esp32";

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("[wifi] Connecting");
  while (WiFi.status() != WL_CONNECTED) { delay(400); Serial.print("."); }
  Serial.printf("\n[wifi] OK. IP: %s\n", WiFi.localIP().toString().c_str());
}

void onMessage(char* topic, byte* payload, unsigned int len) {
  Serial.print("[mqtt] cmd: ");
  for (unsigned int i=0;i<len;i++) Serial.print((char)payload[i]);
  Serial.println();
}

void reconnectMQTT() {
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(onMessage);
  while (!client.connected()) {
    Serial.print("[mqtt] Connecting...");
    bool ok;
    if (strlen(MQTT_USER)>0) ok = client.connect("esp32-client", MQTT_USER, MQTT_PASS);
    else ok = client.connect("esp32-client");
    if (ok) {
      Serial.println("OK");
      client.subscribe(TOPIC_SUB);
    } else {
      Serial.printf("fail rc=%d, retry in 1s\n", client.state());
      delay(1000);
    }
  }
}

float readSensorMock() {
  return 20.0 + (float)(millis() % 1000) / 100.0;
}

void setup() {
  Serial.begin(115200);
  delay(300);
  connectWiFi();
  reconnectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!client.connected()) reconnectMQTT();
  client.loop();

  char payload[64];
  snprintf(payload, sizeof(payload), "{\"id\":\"esp32\",\"value\":%.2f}", readSensorMock());
  client.publish(TOPIC_PUB, payload);
  Serial.printf("[mqtt] pub %s\n", payload);

  delay(5000);
}
