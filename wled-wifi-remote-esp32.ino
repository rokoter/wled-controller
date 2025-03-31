#include <WiFi.h>
#include <WiFiUdp.h>

// 🛜 WiFi-instellingen
const char* ssid = "WLED-AP";
const char* password = "1234567890";

// 🔥 IP-adres van jouw WLED-controller
const IPAddress wledIP(4, 3, 2, 1); // ← IP van je WLED-matrix
const uint16_t wledPort = 21324;

WiFiUDP udp;

const int potPin = 34;  // Gebruik een ADC pin, zoals GPIO 34
int vorigeWaarde = -1;

void setup() {
  Serial.begin(115200);

  // 💤 Schakel WiFi power-saving uit voor stabiliteit
  WiFi.setSleep(false);  // <-- ESP32 equivalent van 'no sleep'
  
  WiFi.begin(ssid, password);
  Serial.print("🔌 Verbinden met WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Verbonden met WiFi");
}

void loop() {
  int potRaw = analogRead(potPin);
  if (abs(potRaw - vorigeWaarde) < 20) return;
  vorigeWaarde = potRaw;

  if (potRaw < 100) {
    String msg = "{\"on\":false}";
    udp.beginPacket(wledIP, wledPort);
    udp.print(msg);
    udp.endPacket();
    Serial.println("🔥 Vuur UIT");
  } else {
    int vuurLevel = map(potRaw, 100, 4095, 0, 255);
    vuurLevel = constrain(vuurLevel, 0, 255);

    int intensity = 0;
    int speed = 255;
    int blur = 0;
    int boost = 0;

    if (vuurLevel < 10) {
      intensity = 0;
      speed = 255;
      blur = 0;
      boost = 0;
    }
    else if (vuurLevel < 50) {
      intensity = map(vuurLevel, 10, 50, 5, 30);
      speed = map(vuurLevel, 10, 50, 200, 150);
      blur = map(vuurLevel, 10, 50, 5, 20);
      boost = map(vuurLevel, 10, 50, 10, 50);
    }
    else {
      float scale = sqrt((float)vuurLevel / 255.0);
      intensity = vuurLevel;
      speed  = 80 - scale * 50;
      blur   = 10 + scale * 90;
      boost  = 50 + scale * 200;
    }

    String msg = "{\"on\":true,\"seg\":[{\"id\":0,"
                 "\"i\":" + String(intensity) +
                 ",\"sx\":" + String(speed) +
                 ",\"c2\":" + String(blur) +
                 ",\"c3\":" + String(boost) +
                 "}]}";

    udp.beginPacket(wledIP, wledPort);
    udp.print(msg);
    udp.endPacket();

    Serial.println("🔥 VuurLevel: " + String(vuurLevel));
    Serial.println("    Intensity: " + String(intensity));
    Serial.println("    Speed (cooling): " + String(speed));
    Serial.println("    Blur: " + String(blur));
    Serial.println("    Boost: " + String(boost));
  }

  delay(200);
}
