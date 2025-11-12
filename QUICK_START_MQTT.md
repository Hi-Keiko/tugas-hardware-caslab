# Quick Start Guide - MQTT Setup

## 🚀 Setup Cepat (5 Menit)

### 1️⃣ Install Library Python
```bash
pip install paho-mqtt
```

### 2️⃣ Install Library Arduino
Di Arduino IDE:
- Tools → Manage Libraries
- Search: **PubSubClient**
- Install

### 3️⃣ Edit main.cpp
Ganti baris 12-13:
```cpp
const char* ssid = "NAMA_WIFI_ANDA";          // ⚠️ GANTI
const char* password = "PASSWORD_WIFI_ANDA";  // ⚠️ GANTI
```

### 4️⃣ Upload ke ESP32
- Pilih Board: ESP32 Dev Module
- Pilih Port
- Upload

### 5️⃣ Jalankan Python
```bash
python main.py
```

## ✅ Checklist

- [ ] Library `paho-mqtt` terinstall di Python
- [ ] Library `PubSubClient` terinstall di Arduino IDE
- [ ] WiFi SSID dan password sudah diganti di main.cpp
- [ ] ESP32 berhasil upload tanpa error
- [ ] ESP32 terhubung ke WiFi (cek Serial Monitor)
- [ ] Python aplikasi running tanpa error
- [ ] Grafik mulai update
- [ ] Tombol buzzer berfungsi

## 🆘 Troubleshooting Cepat

**ESP32 tidak connect WiFi?**
→ Pastikan WiFi 2.4GHz (bukan 5GHz)

**Python error "ModuleNotFoundError: paho"?**
→ Jalankan: `pip install paho-mqtt`

**Grafik tidak update?**
→ Restart ESP32 dan Python aplikasi

---

Untuk dokumentasi lengkap, baca: **README_MQTT.md**
