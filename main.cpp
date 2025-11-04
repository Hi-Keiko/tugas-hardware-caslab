#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;
const int buzzerPin = 25; 
float threshold = 1;   // Lebih rendah → lebih sensitif
const int sampleCount = 10; // Untuk rata-rata data (mengurangi noise)

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();

  // Set range akselerometer ke ±2g (paling sensitif)
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  if (mpu.testConnection()) {
    Serial.println("MPU6050 terdeteksi ✅");
  } else {
    Serial.println("MPU6050 tidak terdeteksi ❌");
    while (1);
  }

  Serial.println("\n--- Seismograf Sensitif Aktif ---");
  Serial.println("Membaca getaran dengan sensitivitas tinggi...");
}

void loop() {
  float totalMagnitude = 0;

  // Ambil beberapa sampel untuk stabilitas
  for (int i = 0; i < sampleCount; i++) {
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    float Ax = ax / 16384.0; // Karena ±2g
    float Ay = ay / 16384.0;
    float Az = az / 16384.0;

    float magnitude = sqrt(Ax * Ax + Ay * Ay + Az * Az);
    totalMagnitude += magnitude;

    delay(10); // sedikit delay antar sampel
  }

  // Rata-rata dari beberapa sampel
  float avgMagnitude = totalMagnitude / sampleCount;

  Serial.print("Getaran (g): ");
  Serial.print(avgMagnitude, 4);

  String skala;
  if (avgMagnitude < 1.02) skala = "Sangat stabil";
  else if (avgMagnitude < 1.08) skala = "Getaran sangat kecil";
  else if (avgMagnitude < 1.15) skala = "Getaran ringan";
  else if (avgMagnitude < 1.3) skala = "Getaran sedang";
  else skala = "Getaran kuat ⚠️";

  Serial.print(" | Skala: ");
  Serial.println(skala);

  // Buzzer aktif bila getaran melewati ambang batas
  if (avgMagnitude > threshold) {
    digitalWrite(buzzerPin, HIGH);
    delay(80);
    digitalWrite(buzzerPin, LOW);
  }

  delay(200);
}


