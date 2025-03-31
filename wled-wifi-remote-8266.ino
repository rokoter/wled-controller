#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "WLED-AP";
const char* password = "1234567890";
const IPAddress wledIP(4, 3, 2, 1); // ← IP van je WLED-matrix
const uint16_t wledPort = 21324;

WiFiUDP udp;

int vorigeWaarde = -1;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Verbonden met WiFi");
}

void loop() {
  int potRaw = analogRead(A0);  // ESP8266 heeft maar 1 analoge pin: A0 (0–1023)

  // Alleen reageren op duidelijke verandering
  if (abs(potRaw - vorigeWaarde) < 10) return;
  vorigeWaarde = potRaw;

  // Potmeter onder drempel = vuur uit
  if (potRaw < 50) {
    String msg = "{\"on\":false}";
    udp.beginPacket(wledIP, wledPort);
    udp.print(msg);
    udp.endPacket();
    Serial.println("🔥 Vuur UIT");
  } else {
    // 🔥 Potmeter boven drempel = vuur aan
    int vuurLevel = map(potRaw, 50, 1023, 0, 255);
    float scale = sqrt((float)vuurLevel / 255.0);

    int intensity = vuurLevel;
    if (vuurLevel < 10) intensity = 0;

    int speed     = 255 - (scale * 200);  // 🔁 omgekeerd
    int blur      = 20 + scale * 100;
    int boost     = 10 + scale * 180;


    // 🔄 JSON bericht opbouwen
    String msg = "{\"on\":true,\"seg\":[{\"id\":0,"
                 "\"i\":" + String(intensity) +
                 ",\"sx\":" + String(speed) +
                 ",\"c2\":" + String(blur) +
                 ",\"c3\":" + String(boost) +
                 "}]}";

    // ✉️ Versturen via UDP
    udp.beginPacket(wledIP, wledPort);
    udp.print(msg);
    udp.endPacket();

    Serial.println("🔥 Vuurlevel: " + String(vuurLevel));
  }

  delay(200); // Verzendfrequentie
}