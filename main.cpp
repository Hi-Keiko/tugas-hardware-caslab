#include <Wire.h>
#include <MPU6050.h>
#include <LiquidCrystal_I2C.h>

// Buat dua instance I2C
TwoWire I2C_MPU = TwoWire(0);   // I2C channel 0 untuk MPU6050
TwoWire I2C_LCD = TwoWire(1);   // I2C channel 1 untuk LCD

MPU6050 mpu(&I2C_MPU);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Ganti 0x27 jika alamat berbeda

const int buzzerPin = 13;

float accelX, accelY, accelZ;
float vibrationMagnitude = 0;
float vibrationThreshold = 0.15;  // semakin kecil = semakin sensitif

void setup() {
  Serial.begin(115200);

  // --- Inisialisasi I2C untuk masing-masing perangkat ---
  I2C_MPU.begin(21, 22);   // SDA, SCL default untuk MPU6050
  I2C_LCD.begin(16, 17);   // SDA=16, SCL=17 (custom pin untuk LCD)

  // --- Inisialisasi MPU6050 ---
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 tidak terdeteksi!");
    while (1);
  }

  // --- Inisialisasi LCD ---
  lcd.begin(16, 2);  // penting untuk inisialisasi internal
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Seismograf ESP32");

  // --- Inisialisasi buzzer ---
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  delay(2000);
  lcd.clear();
}

void loop() {
  // --- Baca data akselerometer dari MPU6050 ---
  accelX = mpu.getAccelerationX() / 16384.0;
  accelY = mpu.getAccelerationY() / 16384.0;
  accelZ = mpu.getAccelerationZ() / 16384.0;

  // --- Hitung magnitude getaran ---
  vibrationMagnitude = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ) - 1.0;
  if (vibrationMagnitude < 0) vibrationMagnitude = -vibrationMagnitude;

  // --- Tampilkan hasil di Serial ---
  Serial.print("Getaran: ");
  Serial.print(vibrationMagnitude, 3);
  Serial.println(" g");

  // --- Tampilkan di LCD ---
  lcd.setCursor(0, 0);
  lcd.print("Getaran: ");
  lcd.print(vibrationMagnitude, 2);
  lcd.print("g   ");

  lcd.setCursor(0, 1);
  if (vibrationMagnitude < vibrationThreshold)
    lcd.print("Tenang          ");
  else if (vibrationMagnitude < 0.4)
    lcd.print("Getaran Ringan  ");
  else if (vibrationMagnitude < 0.8)
    lcd.print("Getaran Sedang  ");
  else
    lcd.print("Getaran Kuat!!! ");

  // --- Kontrol buzzer ---
  if (vibrationMagnitude > 0.6)
    digitalWrite(buzzerPin, HIGH);
  else
    digitalWrite(buzzerPin, LOW);

  delay(200);
}
