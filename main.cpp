#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>

Adafruit_MPU6050 mpu;            // Object sensor Adafruit MPU6050
const int buzzerPin = 25;        // Pin buzzer aktif di GPIO25 (ubah sesuai rangkaianmu)
float threshold = 1.5;           // Ambang batas getaran (g)

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin();
  if (!mpu.begin()) {
    Serial.println("MPU6050 tidak terdeteksi ❌ (Adafruit MPU6050)");
    while (1) delay(10); // Hentikan jika tidak terdeteksi
  }

  // (Opsional) konfigurasi sensor, bisa disesuaikan
  // mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  // mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  // mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  Serial.println("MPU6050 terdeteksi ✅ (Adafruit MPU6050)");
  Serial.println("\n--- Sistem Seismograf Siap ---");
  Serial.println("Membaca getaran...");
}

void loop() {
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;

  // Ambil event sensor (acceleration dalam m/s^2)
  mpu.getEvent(&accel, &gyro, &temp);

  // Konversi ke g (1 g = 9.80665 m/s^2)
  float Ax = accel.acceleration.x / 9.80665;
  float Ay = accel.acceleration.y / 9.80665;
  float Az = accel.acceleration.z / 9.80665;

  // Hitung besar getaran total (dalam g)
  float magnitude = sqrt(Ax * Ax + Ay * Ay + Az * Az);

  // Print ke Serial Monitor
  Serial.print("Getaran (g): ");
  Serial.print(magnitude, 3);

  // Klasifikasi sederhana skala getaran
  String skala;
  if (magnitude < 1.05) skala = "Stabil";
  else if (magnitude < 1.2) skala = "Getaran ringan";
  else if (magnitude < 1.5) skala = "Getaran sedang";
  else skala = "Getaran kuat ⚠️";

  Serial.print(" | Skala: ");
  Serial.println(skala);

  // Aktifkan buzzer jika melewati threshold
  if (magnitude > threshold) {
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(buzzerPin, LOW);
  }

  delay(200); // Delay pembacaan sensor
}
```
