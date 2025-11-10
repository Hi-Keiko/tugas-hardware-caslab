#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <LiquidCrystal.h>
#include <math.h>

LiquidCrystal lcd(14, 27, 26, 25, 33, 32);

MPU6050 mpu;            // Object sensor MPU6050
const int buzzerPin = 4; // Pin buzzer aktif di GPIO25 (ubah sesuai rangkaianmu)
float threshold = 1.1;    // Ambang batas getaran (g)
const float PI_VAL = 3.14159265359;

float smoothedAngle = 0;
float alpha = 0.1;

void setup() {
  Serial.begin(115200);
  Wire.begin(18,19);
  delay(100);
  mpu.initialize();

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  lcd.begin(16, 2);
  lcd.print("Initializing...");
  delay(1000);

  // Cek koneksi sensor
  // if (mpu.testConnection()) {
  //   Serial.println("MPU6050 terdeteksi ✅");
  // } else {
  //   Serial.println("MPU6050 tidak terdeteksi ❌");
  //   while (1); // Hentikan jika tidak terdeteksi
  // }

  // Serial.println("\n--- Sistem Seismograf Siap ---");
  // Serial.println("Membaca getaran...");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MPU Ready");
  delay(1000);
  lcd.clear();
}

void loop() {


  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Konversi ke g (1g = 9.81 m/s^2)
  float Ax = ax / 16384.0;
  float Ay = ay / 16384.0;
  float Az = az / 16384.0;

  float RollAngle = atan2(ay, az) * 180 / PI_VAL;
  float PitchAngle = atan2(-ax, sqrt(ay*ay + az*az)) * 180 / PI_VAL;



  // Hitung besar getaran total
  float magnitude = sqrt(Ax * Ax + Ay * Ay + Az * Az);

  // Print ke Serial Monitor
  // Serial.print("Getaran (g): ");
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

  // Serial.print(" | Skala: ");
  // Serial.println(skala);

  lcd.setCursor(0, 0);
  lcd.print("Getaran:         ");
  lcd.setCursor(8, 0);
  lcd.print("    "); // clear previous digits
  lcd.setCursor(8, 0);
  lcd.print((float)magnitude); // show whole number only
  // lcd.print((char)223); // degree symbol

  // Aktifkan buzzer jika melewati threshold
  if (magnitude > threshold) {
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(buzzerPin, LOW);
  }

  delay(200); // Delay pembacaan sensor
}
