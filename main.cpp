#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <LiquidCrystal.h>
#include <math.h>
#include <WiFi.h>

// Konfigurasi WiFi - GANTI SESUAI KEBUTUHAN
const char* ssid = "ESP32_Seismograph";     // Nama WiFi Access Point
const char* password = "12345678";          // Password WiFi (min 8 karakter)

// Konfigurasi Server
WiFiServer server(8888);  // Port untuk komunikasi
WiFiClient client;

LiquidCrystal lcd(14, 27, 26, 25, 33, 32);

MPU6050 mpu;            // Object sensor MPU6050
const int buzzerPin = 4; // Pin buzzer aktif di GPIO4 (buzzer utama)

// Pin untuk 3 buzzer tambahan yang dapat dikontrol dari UI
const int buzzer1Pin = 15; // GPIO15 untuk Buzzer 1
const int buzzer2Pin = 2;  // GPIO2 untuk Buzzer 2
const int buzzer3Pin = 16; // GPIO16 untuk Buzzer 3

float threshold = 1.1;    // Ambang batas getaran (g)
const float PI_VAL = 3.14159265359;

float smoothedAngle = 0;
float alpha = 0.1;

// Buffer untuk membaca perintah dari client
String wifiCommand = "";

// Deklarasi fungsi
void processCommand(String command);
void setupWiFi();

void setup() {
  Serial.begin(115200);
  
  // Setup WiFi
  setupWiFi();
  
  Wire.begin();
  delay(100);
  mpu.initialize();

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // Setup pin untuk 3 buzzer tambahan
  pinMode(buzzer1Pin, OUTPUT);
  pinMode(buzzer2Pin, OUTPUT);
  pinMode(buzzer3Pin, OUTPUT);
  
  // Matikan semua buzzer di awal
  digitalWrite(buzzer1Pin, LOW);
  digitalWrite(buzzer2Pin, LOW);
  digitalWrite(buzzer3Pin, LOW);

  lcd.begin(16, 2);
  lcd.print("WiFi Ready!");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MPU Ready");
  delay(1000);
  lcd.clear();
}

void setupWiFi() {
  Serial.println("Starting WiFi Access Point...");
  
  // Mode: Access Point (ESP32 sebagai WiFi hotspot)
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  // Tampilkan IP di LCD
  lcd.begin(16, 2);
  lcd.print("WiFi AP Ready");
  lcd.setCursor(0, 1);
  lcd.print(IP);
  delay(3000);
  
  // Start server
  server.begin();
  Serial.println("Server started on port 8888");
  Serial.println("Waiting for client connection...");
}

void loop() {

  // Cek koneksi client
  if (!client.connected()) {
    client = server.available();
    if (client) {
      Serial.println("New client connected!");
      lcd.clear();
      lcd.print("Client Connect!");
      delay(1000);
      lcd.clear();
    }
  }

  // Cek apakah ada perintah dari WiFi client
  if (client && client.connected()) {
    while (client.available() > 0) {
      char inChar = (char)client.read();
      
      if (inChar == '\n') {
        // Proses perintah yang diterima
        processCommand(wifiCommand);
        wifiCommand = ""; // Reset buffer
      } else {
        wifiCommand += inChar;
      }
    }
  }

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Konversi ke g (1g = 9.81 m/s^2)
  float Ax = ax / 16384.0;
  float Ay = ay / 16384.0;
  float Az = az / 16384.0;

  float RollAngle = atan2(ay, az) * 180 / PI_VAL;
  float PitchAngle = atan2(-ax, sqrt(ay*ay + az*az)) * 180 / PI_VAL;

  float magnitude = sqrt(Ax * Ax + Ay * Ay + Az * Az);

  // Kirim data ke client via WiFi (jika terhubung)
  if (client && client.connected()) {
    client.print(Ax, 3);
    client.print(",");
    client.print(Ay, 3);
    client.print(",");
    client.print(RollAngle, 2);
    client.print(",");
    client.print(PitchAngle, 2);
    client.print(",");
    client.println(Az, 2);
  }

  // Tetap kirim ke Serial untuk debugging
  Serial.print(Ax, 3);
  Serial.print(",");
  Serial.print(Ay, 3);
  Serial.print(",");
  Serial.print(RollAngle, 2);
  Serial.print(",");
  Serial.print(PitchAngle, 2);
  Serial.print(",");
  Serial.println(Az, 2);



  // Klasifikasi sederhana skala getaran
  String skala;
  if (magnitude < 1.05) skala = "Stabil";
  else if (magnitude < 1.2) skala = "Getaran ringan";
  else if (magnitude < 1.5) skala = "Getaran sedang";
  else skala = "Getaran kuat ⚠️";


  lcd.setCursor(0, 0);
  lcd.print("Getaran:         ");
  lcd.setCursor(8, 0);
  lcd.print("    "); 
  lcd.setCursor(8, 0);
  lcd.print((float)magnitude); 


  // Aktifkan buzzer jika melewati threshold
  if (magnitude > threshold) {
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(buzzerPin, LOW);
  }

  delay(200); // Delay pembacaan sensor
}

// Fungsi untuk memproses perintah kontrol buzzer
void processCommand(String command) {
  command.trim(); // Hapus whitespace
  
  // Format perintah: BUZ<nomor>,<state>
  // Contoh: BUZ1,1 (buzzer 1 ON), BUZ2,0 (buzzer 2 OFF)
  
  if (command.startsWith("BUZ")) {
    // Ambil nomor buzzer (karakter ke-4)
    int buzzerNum = command.charAt(3) - '0';
    
    // Ambil state (karakter setelah koma)
    int commaIndex = command.indexOf(',');
    if (commaIndex > 0 && commaIndex < command.length() - 1) {
      int state = command.charAt(commaIndex + 1) - '0';
      
      // Kontrol buzzer sesuai nomor
      switch(buzzerNum) {
        case 1:
          digitalWrite(buzzer1Pin, state == 1 ? HIGH : LOW);
          if (client && client.connected()) {
            client.print("Buzzer 1 ");
            client.println(state == 1 ? "ON" : "OFF");
          }
          Serial.print("Buzzer 1 ");
          Serial.println(state == 1 ? "ON" : "OFF");
          break;
        case 2:
          digitalWrite(buzzer2Pin, state == 1 ? HIGH : LOW);
          if (client && client.connected()) {
            client.print("Buzzer 2 ");
            client.println(state == 1 ? "ON" : "OFF");
          }
          Serial.print("Buzzer 2 ");
          Serial.println(state == 1 ? "ON" : "OFF");
          break;
        case 3:
          digitalWrite(buzzer3Pin, state == 1 ? HIGH : LOW);
          if (client && client.connected()) {
            client.print("Buzzer 3 ");
            client.println(state == 1 ? "ON" : "OFF");
          }
          Serial.print("Buzzer 3 ");
          Serial.println(state == 1 ? "ON" : "OFF");
          break;
        default:
          if (client && client.connected()) {
            client.println("Error: Buzzer number invalid");
          }
          Serial.println("Error: Buzzer number invalid");
          break;
      }
    }
  }
}
