#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;            // Object sensor MPU6050
const int buzzerPin = 25; // Pin buzzer aktif di GPIO25 (ubah sesuai rangkaianmu)
float threshold = 1.5;    // Ambang batas getaran (g)

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // Cek koneksi sensor
  if (mpu.testConnection()) {
    Serial.println("MPU6050 terdeteksi ✅");
  } else {
    Serial.println("MPU6050 tidak terdeteksi ❌");
    while (1); // Hentikan jika tidak terdeteksi
  }

  Serial.println("\n--- Sistem Seismograf Siap ---");
  Serial.println("Membaca getaran...");
}

void loop() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Konversi ke g (1g = 9.81 m/s^2)
  float Ax = ax / 16384.0;
  float Ay = ay / 16384.0;
  float Az = az / 16384.0;

  // Hitung besar getaran total
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
