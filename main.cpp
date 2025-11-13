#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <LiquidCrystal.h>
#include <math.h>

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

// State buzzer untuk pola sirene
bool buzzer1State = false;
bool buzzer2State = false;
bool buzzer3State = false;

// Variabel untuk pola sirene
unsigned long lastSireneUpdate = 0;
int sireneFreq = 1000;  // Frekuensi awal sirene
bool sireneIncreasing = true;

// PWM channels untuk ESP32
const int pwmChannel1 = 0;
const int pwmChannel2 = 1;
const int pwmChannel3 = 2;
const int pwmFreq = 2000;     // Frekuensi PWM
const int pwmResolution = 8;  // Resolusi 8-bit (0-255)

// Deklarasi fungsi
void processCommand(String command);
void updateSirenePattern();

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(100);
  mpu.initialize();

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // Setup PWM untuk 3 buzzer tambahan (untuk efek sirene)
  ledcSetup(pwmChannel1, pwmFreq, pwmResolution);
  ledcAttachPin(buzzer1Pin, pwmChannel1);
  
  ledcSetup(pwmChannel2, pwmFreq, pwmResolution);
  ledcAttachPin(buzzer2Pin, pwmChannel2);
  
  ledcSetup(pwmChannel3, pwmFreq, pwmResolution);
  ledcAttachPin(buzzer3Pin, pwmChannel3);
  
  // Matikan semua buzzer di awal
  ledcWrite(pwmChannel1, 0);
  ledcWrite(pwmChannel2, 0);
  ledcWrite(pwmChannel3, 0);

  lcd.begin(16, 2);
  lcd.print("Initializing...");
  delay(1000);


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MPU Ready");
  delay(1000);
  lcd.clear();
}

void loop() {

  // Update pola sirene untuk buzzer yang aktif
  updateSirenePattern();

  // Cek apakah ada perintah dari serial (dari UI Python)
  while (Serial.available() > 0) {
    char inChar = (char)Serial.read();
    
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

// Fungsi untuk mengupdate pola sirene
void updateSirenePattern() {
  unsigned long currentMillis = millis();
  
  // Update frekuensi sirene setiap 10ms
  if (currentMillis - lastSireneUpdate >= 10) {
    lastSireneUpdate = currentMillis;
    
    // Pola naik turun untuk efek sirene
    if (sireneIncreasing) {
      sireneFreq += 20;  // Naikkan frekuensi
      if (sireneFreq >= 2000) {
        sireneIncreasing = false;
      }
    } else {
      sireneFreq -= 20;  // Turunkan frekuensi
      if (sireneFreq <= 800) {
        sireneIncreasing = true;
      }
    }
    
    // Terapkan pola sirene ke buzzer yang aktif
    if (buzzer1State) {
      ledcWriteTone(pwmChannel1, sireneFreq);
      ledcWrite(pwmChannel1, 128); // 50% duty cycle
    }
    
    if (buzzer2State) {
      ledcWriteTone(pwmChannel2, sireneFreq);
      ledcWrite(pwmChannel2, 128);
    }
    
    if (buzzer3State) {
      ledcWriteTone(pwmChannel3, sireneFreq);
      ledcWrite(pwmChannel3, 128);
    }
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
          buzzer1State = (state == 1);
          if (buzzer1State) {
            ledcWriteTone(pwmChannel1, sireneFreq);
            ledcWrite(pwmChannel1, 128);
          } else {
            ledcWrite(pwmChannel1, 0);
          }
          Serial.print("Buzzer 1 ");
          Serial.println(state == 1 ? "ON (Siren)" : "OFF");
          break;
        case 2:
          buzzer2State = (state == 1);
          if (buzzer2State) {
            ledcWriteTone(pwmChannel2, sireneFreq);
            ledcWrite(pwmChannel2, 128);
          } else {
            ledcWrite(pwmChannel2, 0);
          }
          Serial.print("Buzzer 2 ");
          Serial.println(state == 1 ? "ON (Siren)" : "OFF");
          break;
        case 3:
          buzzer3State = (state == 1);
          if (buzzer3State) {
            ledcWriteTone(pwmChannel3, sireneFreq);
            ledcWrite(pwmChannel3, 128);
          } else {
            ledcWrite(pwmChannel3, 0);
          }
          Serial.print("Buzzer 3 ");
          Serial.println(state == 1 ? "ON (Siren)" : "OFF");
          break;
        default:
          Serial.println("Error: Buzzer number invalid");
          break;
      }
    }
  }
}
