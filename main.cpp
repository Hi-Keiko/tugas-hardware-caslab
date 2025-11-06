#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>

// Buat dua bus I2C
TwoWire I2C_MPU = TwoWire(0);   // I2C untuk MPU6050
TwoWire I2C_LCD = TwoWire(1);   // I2C untuk LCD

Adafruit_MPU6050 mpu;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Ganti 0x27 jika LCD kamu beda alamat

const int buzzerPin = 13;
float vibrationThreshold = 0.15; // semakin kecil semakin sensitif

void setup() {
  Serial.begin(115200);

  // Inisialisasi dua bus I2C
  I2C_MPU.begin(21, 22); // SDA, SCL untuk MPU6050
  I2C_LCD.begin(16, 17); // SDA, SCL untuk LCD (port digital custom)

  // Inisialisasi LCD
  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Seismograf ESP32");
  delay(1500);
  lcd.clear();

  // Inisialisasi MPU6050
  if (!mpu.begin(0x68, &I2C_MPU)) {
    Serial.println("MPU6050 tidak terdeteksi!");
    lcd.setCursor(0, 0);
    lcd.print("MPU ERROR!");
    while (1);
  }

  // Atur rentang sensitivitas
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G); // paling sensitif
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Inisialisasi buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  Serial.println("MPU6050 Siap!");
  lcd.setCursor(0, 0);
  lcd.print("MPU6050 Siap!");
  delay(1000);
  lcd.clear();
}

void loop() {
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  // Hitung magnitudo getaran (tanpa arah)
  float magnitude = sqrt(accel.acceleration.x * accel.acceleration.x +
                         accel.acceleration.y * accel.acceleration.y +
                         accel.acceleration.z * accel.acceleration.z) - 9.81;

  // Konversi ke nilai absolut (m/s²)
  if (magnitude < 0) magnitude = -magnitude;

  // Konversi ke G-force
  float magnitude_g = magnitude / 9.81;

  // Cetak ke Serial Monitor
  Serial.print("Getaran: ");
  Serial.print(magnitude_g, 3);
  Serial.println(" g");

  // Tampilkan ke LCD
  lcd.setCursor(0, 0);
  lcd.print("Getaran: ");
  lcd.print(magnitude_g, 2);
  lcd.print("g   ");

  lcd.setCursor(0, 1);
  if (magnitude_g < vibrationThreshold)
    lcd.print("Tenang          ");
  else if (magnitude_g < 0.4)
    lcd.print("Getaran Ringan  ");
  else if (magnitude_g < 0.8)
    lcd.print("Getaran Sedang  ");
  else
    lcd.print("Getaran Kuat!!! ");

  // Kontrol buzzer
  if (magnitude_g > 0.6)
    digitalWrite(buzzerPin, HIGH);
  else
    digitalWrite(buzzerPin, LOW);

  delay(200);
}
