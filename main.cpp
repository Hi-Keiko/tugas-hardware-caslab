#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <LiquidCrystal_I2C.h>

MPU6050 mpu;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Ganti 0x27 dengan 0x3F jika LCD tidak tampil
const int buzzerPin = 25; 
float threshold = 1.05;  // Lebih sensitif
const int sampleCount = 10; 
unsigned long totalGetaran = 0; // Menyimpan jumlah getaran terdeteksi

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();

  // Atur range akselerometer ke ±2g agar lebih sensitif
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);

  // Setup buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // Setup LCD
  lcd.init();
  lcd.backlight();

  // Cek koneksi sensor
  if (mpu.testConnection()) {
    Serial.println("MPU6050 terdeteksi ✅");
    lcd.setCursor(0, 0);
    lcd.print("MPU6050 OK");
  } else {
    Serial.println("MPU6050 gagal ❌");
    lcd.setCursor(0, 0);
    lcd.print("MPU6050 Error");
    while (1);
  }

  delay(1000);
  lcd.clear();
  lcd.print("Seismograf Aktif");
  delay(1000);
  lcd.clear();

  Serial.println("\n--- Seismograf + LCD ---");
  Serial.println("Membaca getaran...");
}

void loop() {
  float totalMagnitude = 0;

  // Ambil beberapa sampel
  for (int i = 0; i < sampleCount; i++) {
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    float Ax = ax / 16384.0;
    float Ay = ay / 16384.0;
    float Az = az / 16384.0;

    float magnitude = sqrt(Ax * Ax + Ay * Ay + Az * Az);
    totalMagnitude += magnitude;
    delay(10);
  }

  float avgMagnitude = totalMagnitude / sampleCount;

  // Tampilkan di Serial Monitor
  Serial.print("Getaran (g): ");
  Serial.print(avgMagnitude, 4);

  String skala;
  if (avgMagnitude < 1.02) skala = "Stabil";
  else if (avgMagnitude < 1.08) skala = "Kecil";
  else if (avgMagnitude < 1.15) skala = "Ringan";
  else if (avgMagnitude < 1.3) skala = "Sedang";
  else skala = "Kuat ⚠️";

  Serial.print(" | Skala: ");
  Serial.println(skala);

  // Jika melewati ambang batas, hitung sebagai satu getaran
  if (avgMagnitude > threshold) {
    totalGetaran++;
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(buzzerPin, LOW);
  }

  // --- LCD OUTPUT ---
  lcd.setCursor(0, 0);
  lcd.print("G: ");
  lcd.print(avgMagnitude, 2); // Tampilkan 2 digit desimal
  lcd.print("g ");

  lcd.setCursor(0, 1);
  lcd.print("Getaran: ");
  lcd.print(totalGetaran);

  delay(300);
}
