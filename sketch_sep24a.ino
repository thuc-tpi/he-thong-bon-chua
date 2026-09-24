#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// --- CẤU HÌNH WIFI ---
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// --- CẤU HÌNH FIREBASE ---
const char* firebaseHost = "https://he-thong-bon-chua-default-rtdb.asia-southeast1.firebasedatabase.app/tank_1.json";

#define ANALOG_PIN A0

unsigned long lastSend = 0;
const long interval = 1000; // Gửi dữ liệu mỗi 1 giây

void sendDataToFirebase() {
  int analogValue = analogRead(ANALOG_PIN);
  int percent = map(analogValue, 0, 1023, 0, 100);
  float voltage = (analogValue / 1023.0) * 3.3;

  String jsonPayload = "{";
  jsonPayload += "\"raw\":" + String(analogValue) + ",";
  jsonPayload += "\"percent\":" + String(percent) + ",";
  jsonPayload += "\"voltage\":" + String(voltage, 2);
  jsonPayload += "}";

  WiFiClientSecure client;
  client.setInsecure(); // Bỏ qua xác minh SSL Fingerprint để kết nối linh hoạt

  HTTPClient http;
  if (http.begin(client, firebaseHost)) {
    http.addHeader("Content-Type", "application/json");
    
    // Sử dụng PATCH để cập nhật hoặc tạo mới dữ liệu tại vị trí /tank_1
    int httpCode = http.PATCH(jsonPayload);

    if (httpCode > 0) {
      Serial.printf("[Firebase] HTTP Code: %d\n", httpCode);
    } else {
      Serial.printf("[Firebase] Lỗi gửi dữ liệu: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nĐã kết nối WiFi thành công!");
  Serial.print("Địa chỉ IP ESP8266: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastSend >= interval) {
      lastSend = currentMillis;
      sendDataToFirebase();
    }
  }
}