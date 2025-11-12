# 📡 Panduan Setup Komunikasi MQTT - ESP32 Seismograph

Dokumentasi lengkap untuk menggunakan sistem Seismograph dengan komunikasi wireless menggunakan protokol MQTT (Message Queuing Telemetry Transport).

---

## 📋 Daftar Isi
1. [Pengenalan MQTT](#pengenalan-mqtt)
2. [Persiapan Hardware](#persiapan-hardware)
3. [Instalasi Library](#instalasi-library)
4. [Konfigurasi ESP32](#konfigurasi-esp32)
5. [Konfigurasi Python](#konfigurasi-python)
6. [Cara Menggunakan](#cara-menggunakan)
7. [Troubleshooting](#troubleshooting)
8. [Topologi Komunikasi](#topologi-komunikasi)

---

## 🌐 Pengenalan MQTT

### Apa itu MQTT?
MQTT (Message Queuing Telemetry Transport) adalah protokol komunikasi lightweight yang dirancang untuk IoT (Internet of Things). MQTT menggunakan model publish-subscribe untuk komunikasi.

### Keuntungan MQTT vs Serial:
- ✅ **Wireless**: Tidak perlu kabel USB
- ✅ **Jarak Jauh**: Bisa akses dari mana saja (selama terhubung internet)
- ✅ **Multiple Clients**: Banyak device bisa subscribe ke topic yang sama
- ✅ **Reliable**: Mendukung QoS (Quality of Service)
- ✅ **Bandwidth Rendah**: Hemat data

### Broker MQTT yang Digunakan:
Kami menggunakan **EMQX Public Broker**:
- **Host**: `broker.emqx.io`
- **Port**: `1883` (non-SSL) atau `8883` (SSL)
- **Gratis** dan **tidak perlu registrasi**

---

## 🔧 Persiapan Hardware

### Komponen yang Dibutuhkan:
1. **ESP32** (dengan WiFi built-in)
2. **MPU6050** (Accelerometer & Gyroscope)
3. **LCD 16x2** (Display)
4. **4 Buzzer** (1 utama + 3 kontrol)
5. **Koneksi WiFi** yang stabil

### Pin Configuration:
```
LCD:
- RS  -> GPIO14
- E   -> GPIO27
- D4  -> GPIO26
- D5  -> GPIO25
- D6  -> GPIO33
- D7  -> GPIO32

MPU6050 (I2C):
- SDA -> GPIO21
- SCL -> GPIO22

Buzzer:
- Buzzer Utama -> GPIO4
- Buzzer 1      -> GPIO15
- Buzzer 2      -> GPIO2
- Buzzer 3      -> GPIO16
```

---

## 📦 Instalasi Library

### A. Library Arduino (ESP32)

**1. PubSubClient (Library MQTT)**
```
1. Buka Arduino IDE
2. Menu: Sketch → Include Library → Manage Libraries
3. Search: "PubSubClient" by Nick O'Leary
4. Klik Install
```

**2. Library Lainnya** (jika belum ada):
- `WiFi.h` (sudah built-in di ESP32)
- `Wire.h` (sudah built-in)
- `MPU6050.h` → Search di Library Manager
- `LiquidCrystal.h` (sudah built-in)

### B. Library Python

**Install menggunakan pip:**
```bash
pip install paho-mqtt
```

Atau jika menggunakan pip3:
```bash
pip3 install paho-mqtt
```

**Library lain yang dibutuhkan** (seharusnya sudah terinstall):
```bash
pip install PyQt5 pyqtgraph numpy
```

---

## ⚙️ Konfigurasi ESP32

### 1. Edit File `main.cpp`

**Ganti kredensial WiFi Anda** di bagian ini (baris 12-13):
```cpp
// ===== KONFIGURASI WiFi =====
const char* ssid = "NAMA_WIFI_ANDA";          // ⚠️ GANTI INI
const char* password = "PASSWORD_WIFI_ANDA";  // ⚠️ GANTI INI
```

**Contoh:**
```cpp
const char* ssid = "MyHome-WiFi";
const char* password = "password123";
```

### 2. Topic MQTT (Opsional)

Jika ingin mengubah nama topic, edit baris 16-17:
```cpp
const char* mqtt_topic_data = "seismograph/data";       // Data dari sensor
const char* mqtt_topic_buzzer = "seismograph/buzzer";   // Perintah buzzer
```

**💡 Tips**: Gunakan prefix unik jika ada banyak device seismograph:
```cpp
const char* mqtt_topic_data = "seismo/device1/data";
const char* mqtt_topic_buzzer = "seismo/device1/buzzer";
```

### 3. Upload ke ESP32

1. Hubungkan ESP32 ke komputer via USB
2. Pilih Board: `ESP32 Dev Module`
3. Pilih Port yang sesuai
4. Klik Upload

### 4. Monitor Serial

Buka Serial Monitor (115200 baud) untuk melihat status koneksi:
```
Connecting to WiFi: MyHome-WiFi
.....
WiFi connected!
IP address: 192.168.1.100
Attempting MQTT connection...connected
Subscribed to: seismograph/buzzer
ESP32 Seismograph - MQTT Mode
Client ID: ESP32-Seismograph-A3F2
```

---

## 🐍 Konfigurasi Python

### 1. Edit File `main.py`

**Tidak perlu mengubah apapun jika menggunakan default settings.**

Tapi jika Anda mengubah topic di ESP32, ubah juga di Python (baris 26-29):
```python
self.MQTT_BROKER = "broker.emqx.io"
self.MQTT_PORT = 1883
self.MQTT_TOPIC_DATA = "seismograph/data"       # Harus sama dengan ESP32
self.MQTT_TOPIC_BUZZER = "seismograph/buzzer"   # Harus sama dengan ESP32
```

### 2. Jalankan Aplikasi

```bash
python main.py
```

Atau:
```bash
python3 main.py
```

---

## 🚀 Cara Menggunakan

### Langkah-langkah:

1. **Nyalakan ESP32**
   - ESP32 akan otomatis connect ke WiFi
   - Kemudian connect ke MQTT broker
   - LCD akan menampilkan status koneksi

2. **Jalankan Aplikasi Python**
   ```bash
   python main.py
   ```

3. **Tunggu Koneksi MQTT**
   - Cek console untuk pesan:
     ```
     Connecting to MQTT broker: broker.emqx.io:1883
     Connected to MQTT Broker!
     Subscribed to topic: seismograph/data
     ```

4. **Monitor Data Real-time**
   - Grafik akan mulai update secara otomatis
   - Data dikirim setiap 200ms dari ESP32

5. **Kontrol Buzzer**
   - Klik tombol ON/OFF pada panel "BUZZER CONTROL"
   - Perintah akan dikirim via MQTT ke ESP32
   - Buzzer akan merespon dalam <100ms

### Indikator Status:

**ESP32 LCD Display:**
- `Connecting WiFi` → Sedang konek ke WiFi
- `WiFi Connected` + IP → WiFi berhasil
- `MQTT Connecting` → Sedang konek ke broker
- `MQTT Connected` → Siap mengirim/menerima data
- `Getaran: X.XX` → Nilai magnitude getaran

**Python Console:**
- `Connected to MQTT Broker!` → Koneksi berhasil
- `Subscribed to topic: ...` → Listening untuk data
- `Perintah dikirim via MQTT: ...` → Buzzer command sent

---

## 🔍 Troubleshooting

### Problem 1: ESP32 Tidak Bisa Connect ke WiFi

**Gejala:**
```
Connecting to WiFi: .....................
WiFi connection failed!
```

**Solusi:**
- ✅ Cek SSID dan password sudah benar
- ✅ Pastikan WiFi menggunakan 2.4GHz (ESP32 tidak support 5GHz)
- ✅ Coba restart router WiFi
- ✅ Dekatkan ESP32 ke router

---

### Problem 2: MQTT Connection Failed

**Gejala:**
```
Attempting MQTT connection...failed, rc=-2 try again in 5 seconds
```

**Return Code & Artinya:**
- `-4` atau `4`: Connection timeout
- `-2` atau `2`: Network failure
- `-1` atau `5`: Connection refused

**Solusi:**
- ✅ Pastikan koneksi internet aktif
- ✅ Coba ping `broker.emqx.io`:
  ```bash
  ping broker.emqx.io
  ```
- ✅ Cek firewall tidak memblok port 1883
- ✅ Gunakan broker alternatif jika EMQX down:
  ```cpp
  const char* mqtt_broker = "test.mosquitto.org";
  ```

---

### Problem 3: Python Tidak Terima Data

**Gejala:**
- Grafik tidak update
- Console Python tidak menampilkan data

**Solusi:**
1. **Cek Topic Match:**
   ```python
   # Di Python
   self.MQTT_TOPIC_DATA = "seismograph/data"
   ```
   ```cpp
   // Di ESP32
   const char* mqtt_topic_data = "seismograph/data";
   ```
   **Harus sama persis!**

2. **Restart Kedua Program:**
   - Restart ESP32 (tekan tombol reset)
   - Restart Python aplikasi

3. **Cek Console ESP32:**
   - Buka Serial Monitor
   - Pastikan data terkirim (muncul print `Ax,Ay,Roll,Pitch,Az`)

---

### Problem 4: Library Import Error

**Gejala:**
```python
ModuleNotFoundError: No module named 'paho'
```

**Solusi:**
```bash
pip install paho-mqtt --upgrade
```

Atau cek Python version:
```bash
python --version
pip --version
```

Jika menggunakan Python 3:
```bash
pip3 install paho-mqtt
python3 main.py
```

---

### Problem 5: Buzzer Tidak Merespon

**Gejala:**
- Klik tombol ON/OFF tapi buzzer tidak bunyi

**Solusi:**
1. **Cek Console Python:**
   ```
   Perintah dikirim via MQTT: Buzzer 1 -> ON
   ```

2. **Cek Serial Monitor ESP32:**
   Seharusnya muncul:
   ```
   Message arrived [seismograph/buzzer] BUZ1,1
   Buzzer 1 ON
   ```

3. **Jika tidak muncul di ESP32:**
   - ESP32 belum subscribe ke topic buzzer
   - Restart ESP32 dan cek log MQTT connection

4. **Jika muncul tapi buzzer tidak bunyi:**
   - Cek koneksi hardware buzzer
   - Test dengan mengubah kode:
     ```cpp
     digitalWrite(buzzer1Pin, HIGH);
     delay(1000);
     digitalWrite(buzzer1Pin, LOW);
     ```

---

## 📊 Topologi Komunikasi

```
┌──────────────┐         WiFi          ┌─────────────────┐
│              │  ────────────────────▶ │                 │
│    ESP32     │                        │  WiFi Router    │
│  (Publisher/ │  ◀────────────────────│   (Access       │
│  Subscriber) │                        │    Point)       │
└──────────────┘                        └─────────────────┘
      ▲                                          │
      │                                          │
      │ MQTT Protocol                  Internet  │
      │ (Port 1883)                              │
      │                                          ▼
      │                                 ┌─────────────────┐
      │                                 │                 │
      └─────────────────────────────────│  EMQX Broker    │
                                        │  (broker.emqx   │
                                        │     .io)        │
                                        └─────────────────┘
                                                 ▲
                                                 │
                                                 │ MQTT
                                                 │
                                        ┌────────┴────────┐
                                        │                 │
                                        │  Python Client  │
                                        │  (Subscriber/   │
                                        │   Publisher)    │
                                        └─────────────────┘
```

### Data Flow:

**1. Sensor Data (ESP32 → Python):**
```
ESP32 Sensor → Publish to "seismograph/data"
                    ↓
              EMQX Broker
                    ↓
Python Subscribe "seismograph/data" → Update GUI
```

**2. Buzzer Control (Python → ESP32):**
```
Python GUI (Button Click) → Publish to "seismograph/buzzer"
                                  ↓
                            EMQX Broker
                                  ↓
ESP32 Subscribe "seismograph/buzzer" → Control GPIO
```

---

## 🔐 Security Notes

### ⚠️ Peringatan untuk Broker Publik:

1. **Jangan kirim data sensitif** - Broker publik bisa diakses siapa saja
2. **Gunakan topic unik** - Hindari konflik dengan pengguna lain
3. **Untuk production**, gunakan:
   - Broker private (self-hosted)
   - MQTT dengan SSL/TLS (port 8883)
   - Username & password authentication

### Upgrade ke SSL/TLS (Opsional):

**ESP32:**
```cpp
#include <WiFiClientSecure.h>

WiFiClientSecure espClient;
const int mqtt_port = 8883;  // SSL port
```

**Python:**
```python
import ssl

mqtt_client.tls_set(
    ca_certs=None,
    certfile=None,
    keyfile=None,
    cert_reqs=ssl.CERT_REQUIRED,
    tls_version=ssl.PROTOCOL_TLS,
    ciphers=None
)
```

---

## 📈 Performance Tips

1. **Reduce Publish Rate** jika bandwidth terbatas:
   ```cpp
   const long mqttPublishInterval = 500; // 500ms instead of 200ms
   ```

2. **Use QoS Level 0** untuk data sensor (sudah default):
   ```python
   mqtt_client.publish(topic, payload, qos=0)
   ```

3. **Compress Data** jika perlu:
   ```cpp
   // Instead of: "0.123,0.456,12.34,23.45,0.987"
   // Use shorter format: "0.12,0.46,12.3,23.5,0.99"
   ```

---

## 📞 Support

Jika masih ada masalah:
1. Cek dokumentasi EMQX: https://www.emqx.io/docs/en/latest/
2. Cek Serial Monitor ESP32 untuk error messages
3. Cek Python console untuk error stack trace

---

## 📝 License

Project ini untuk keperluan edukasi dan penelitian.

---

**Selamat mencoba! 🎉**

Dibuat dengan ❤️ untuk proyek Seismograph IoT
