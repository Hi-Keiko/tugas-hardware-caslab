#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <LiquidCrystal.h>
#include <math.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ===== KONFIGURASI WiFi =====
const char* ssid = "NAMA_WIFI_ANDA";          // Ganti dengan nama WiFi Anda
const char* password = "PASSWORD_WIFI_ANDA";  // Ganti dengan password WiFi Anda

// ===== KONFIGURASI MQTT (EMQX Public Broker) =====
const char* mqtt_broker = "broker.emqx.io";
const char* mqtt_topic_data = "seismograph/data";       // Topic untuk mengirim data sensor
const char* mqtt_topic_buzzer = "seismograph/buzzer";   // Topic untuk menerima perintah buzzer
const char* mqtt_username = "emqx";                     // Username (opsional untuk broker publik)
const char* mqtt_password = "public";                   // Password (opsional untuk broker publik)
const int mqtt_port = 1883;

// Client ID unik (gunakan MAC address atau ID acak)
String clientId = "ESP32-Seismograph-" + String(random(0xffff), HEX);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

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

unsigned long lastMqttPublish = 0;
const long mqttPublishInterval = 200; // Publish setiap 200ms (sama dengan delay sebelumnya)

// Deklarasi fungsi
void processCommand(String command);
void setupWiFi();
void reconnectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  
  // Setup WiFi
  setupWiFi();
  
  // Setup MQTT
  mqttClient.setServer(mqtt_broker, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  
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
  lcd.print("Initializing...");
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MPU Ready");
  delay(1000);
  lcd.clear();
  
  Serial.println("ESP32 Seismograph - MQTT Mode");
  Serial.print("Client ID: ");
  Serial.println(clientId);
}

void loop() {
  // Pastikan MQTT tetap terhubung
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  unsigned long currentMillis = millis();

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Konversi ke g (1g = 9.81 m/s^2)
  float Ax = ax / 16384.0;
  float Ay = ay / 16384.0;
  float Az = az / 16384.0;

  float RollAngle = atan2(ay, az) * 180 / PI_VAL;
  float PitchAngle = atan2(-ax, sqrt(ay*ay + az*az)) * 180 / PI_VAL;

  float magnitude = sqrt(Ax * Ax + Ay * Ay + Az * Az);

  // Publish data via MQTT setiap interval tertentu
  if (currentMillis - lastMqttPublish >= mqttPublishInterval) {
    lastMqttPublish = currentMillis;
    
    // Format: Ax,Ay,RollAngle,PitchAngle,Az
    String payload = String(Ax, 3) + "," + 
                     String(Ay, 3) + "," + 
                     String(RollAngle, 2) + "," + 
                     String(PitchAngle, 2) + "," + 
                     String(Az, 2);
    
    mqttClient.publish(mqtt_topic_data, payload.c_str());
    
    // Juga print ke serial untuk debugging
    Serial.println(payload);
  }

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

  delay(50); // Delay kecil untuk stabilitas
}

// Fungsi untuk setup WiFi
void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(0, 1);
    lcd.print("Attempt: ");
    lcd.print(attempts + 1);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000);
  } else {
    Serial.println("WiFi connection failed!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
    delay(2000);
  }
}

// Fungsi untuk reconnect MQTT
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MQTT Connecting");
    
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      
      // Subscribe ke topic buzzer untuk menerima perintah
      mqttClient.subscribe(mqtt_topic_buzzer);
      Serial.print("Subscribed to: ");
      Serial.println(mqtt_topic_buzzer);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MQTT Connected");
      delay(1000);
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MQTT Failed");
      lcd.setCursor(0, 1);
      lcd.print("Retry in 5s");
      
      delay(5000);
    }
  }
}

// Callback ketika menerima pesan MQTT
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  // Proses perintah buzzer
  if (String(topic) == mqtt_topic_buzzer) {
    processCommand(message);
  }
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
          Serial.print("Buzzer 1 ");
          Serial.println(state == 1 ? "ON" : "OFF");
          break;
        case 2:
          digitalWrite(buzzer2Pin, state == 1 ? HIGH : LOW);
          Serial.print("Buzzer 2 ");
          Serial.println(state == 1 ? "ON" : "OFF");
          break;
        case 3:
          digitalWrite(buzzer3Pin, state == 1 ? HIGH : LOW);
          Serial.print("Buzzer 3 ");
          Serial.println(state == 1 ? "ON" : "OFF");
          break;
        default:
          Serial.println("Error: Buzzer number invalid");
          break;
      }
    }
  }
}
