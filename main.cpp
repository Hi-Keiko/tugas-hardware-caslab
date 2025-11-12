#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <LiquidCrystal.h>
#include <math.h>
#include "BluetoothSerial.h"

// Cek apakah Bluetooth tersedia
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
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

// Buffer untuk membaca perintah serial
String serialCommand = "";

// Deklarasi fungsi
void processCommand(String command);

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi Bluetooth Serial
  SerialBT.begin("ESP32_Seismograph"); // Nama Bluetooth device
  Serial.println("Bluetooth Device is ready to pair");
  
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
  lcd.print("BT: ESP32_Seis");
  lcd.setCursor(0, 1);
  lcd.print("Ready to pair");
  delay(2000);
  lcd.clear();
}

void loop() {

  // Cek apakah ada perintah dari Bluetooth (dari UI Python)
  while (SerialBT.available() > 0) {
    char inChar = (char)SerialBT.read();
    
    if (inChar == '\n') {
      // Proses perintah yang diterima
      processCommand(serialCommand);
      serialCommand = ""; // Reset buffer
    } else {
      serialCommand += inChar;
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

  // Kirim data ke Bluetooth dan Serial Monitor
  SerialBT.print(Ax, 3);
  SerialBT.print(",");
  SerialBT.print(Ay, 3);
  SerialBT.print(",");
  SerialBT.print(RollAngle, 2);
  SerialBT.print(",");
  SerialBT.print(PitchAngle, 2);
  SerialBT.print(",");
  SerialBT.println(Az, 2);
  
  // Debug ke Serial Monitor (opsional)
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
          SerialBT.print("Buzzer 1 ");
          SerialBT.println(state == 1 ? "ON" : "OFF");
          Serial.print("Buzzer 1 ");
          Serial.println(state == 1 ? "ON" : "OFF");
          break;
        case 2:
          digitalWrite(buzzer2Pin, state == 1 ? HIGH : LOW);
          SerialBT.print("Buzzer 2 ");
          SerialBT.println(state == 1 ? "ON" : "OFF");
          Serial.print("Buzzer 2 ");
          Serial.println(state == 1 ? "ON" : "OFF");
          break;
        case 3:
          digitalWrite(buzzer3Pin, state == 1 ? HIGH : LOW);
          SerialBT.print("Buzzer 3 ");
          SerialBT.println(state == 1 ? "ON" : "OFF");
          Serial.print("Buzzer 3 ");
          Serial.println(state == 1 ? "ON" : "OFF");
          break;
        default:
          SerialBT.println("Error: Buzzer number invalid");
          Serial.println("Error: Buzzer number invalid");
          break;
      }
    }
  }
}
