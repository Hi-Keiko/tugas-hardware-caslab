#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>

Adafruit_MPU6050 mpu;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Ubah alamat jika perlu
const int buzzerPin = 13;
float vibrationThreshold = 0.15;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // Satu bus I2C

  // Inisialisasi LCD
  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Seismograf ESP32");

  // Inisialisasi MPU6050
  if (!mpu.begin(0x68, &Wire)) {
    Serial.println("MPU6050 tidak terdeteksi!");
    lcd.setCursor(0, 1);
    lcd.print("MPU ERROR!");
    while (1);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  delay(1500);
  lcd.clear();
}

void loop() {
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  float magnitude = sqrt(accel.acceleration.x * accel.acceleration.x +
                         accel.acceleration.y * accel.acceleration.y +
                         accel.acceleration.z * accel.acceleration.z) - 9.81;
  if (magnitude < 0) magnitude = -magnitude;

  float magnitude_g = magnitude / 9.81;

  Serial.print("Getaran: ");
  Serial.print(magnitude_g, 3);
  Serial.println(" g");

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

  digitalWrite(buzzerPin, magnitude_g > 0.6 ? HIGH : LOW);
  delay(200);
}
