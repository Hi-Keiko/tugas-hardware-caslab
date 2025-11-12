# 📘 Panduan Setup Bluetooth untuk ESP32 Seismograph

## 📋 Daftar Isi
1. [Requirements](#requirements)
2. [Setup ESP32](#setup-esp32)
3. [Setup Komputer (macOS)](#setup-macos)
4. [Setup Komputer (Windows)](#setup-windows)
5. [Setup Komputer (Linux)](#setup-linux)
6. [Menjalankan Aplikasi](#menjalankan-aplikasi)
7. [Troubleshooting](#troubleshooting)

---

## 🔧 Requirements

### Hardware:
- ESP32 (dengan Bluetooth Classic support)
- MPU6050 sensor
- LCD 16x2
- 4 Buzzer (1 untuk threshold alarm, 3 untuk kontrol manual)
- Kabel jumper dan power supply

### Software:
- Arduino IDE atau PlatformIO
- Python 3.7+
- Library Python: `pyserial`, `PyQt5`, `pyqtgraph`, `numpy`

---

## 🚀 Setup ESP32

### Langkah 1: Upload Kode ke ESP32

1. **Buka Arduino IDE** atau PlatformIO
2. **Install ESP32 Board** (jika belum):
   - Arduino IDE: File → Preferences → Additional Board Manager URLs
   - Tambahkan: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Tools → Board → Boards Manager → Cari "ESP32" → Install

3. **Pilih Board ESP32**:
   - Tools → Board → ESP32 Arduino → ESP32 Dev Module (atau sesuai board Anda)

4. **Pastikan Bluetooth Enabled**:
   - Tools → Partition Scheme → "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"
   - Atau pilih yang support Bluetooth

5. **Upload main.cpp**:
   - Hubungkan ESP32 via USB
   - Pilih Port yang benar (Tools → Port)
   - Klik Upload

6. **Verifikasi Upload**:
   - Buka Serial Monitor (115200 baud)
   - Anda akan melihat: "Bluetooth Device is ready to pair"
   - LCD akan menampilkan: "BT: ESP32_Seis" dan "Ready to pair"

### Langkah 2: Cek Nama Bluetooth

- Nama Bluetooth ESP32: **`ESP32_Seismograph`**
- Anda bisa mengubahnya di `main.cpp` pada baris:
  ```cpp
  SerialBT.begin("ESP32_Seismograph"); // Ganti nama di sini
  ```

---

## 🍎 Setup macOS

### Langkah 1: Pair ESP32 dengan macOS

1. **Buka System Preferences**:
   - Klik  → System Preferences → Bluetooth

2. **Aktifkan Bluetooth** di Mac Anda

3. **Cari ESP32**:
   - Tunggu beberapa detik, device bernama **"ESP32_Seismograph"** akan muncul
   - Klik **"Connect"** atau **"Pair"**

4. **Verifikasi Pairing**:
   - Device harus menunjukkan status "Connected"

### Langkah 2: Cari Port Bluetooth

1. **Buka Terminal** (⌘ + Space → ketik "Terminal")

2. **List semua port**:
   ```bash
   ls /dev/tty.*
   ```

3. **Cari port ESP32**:
   ```bash
   ls /dev/tty.ESP32*
   ```
   
   Anda akan melihat sesuatu seperti:
   ```
   /dev/tty.ESP32_Seismograph
   ```

4. **Catat nama port** ini untuk digunakan di aplikasi Python

### Langkah 3: Update main.py

Edit file `main.py`, pada bagian `__init__()`:

```python
self.USE_BLUETOOTH = True
self.SERIAL_PORT = '/dev/tty.ESP32_Seismograph'  # Ganti sesuai port Anda
```

---

## 🪟 Setup Windows

### Langkah 1: Pair ESP32 dengan Windows

1. **Buka Settings**:
   - Klik Start → Settings → Devices → Bluetooth & other devices

2. **Aktifkan Bluetooth**

3. **Add Bluetooth Device**:
   - Klik **"Add Bluetooth or other device"**
   - Pilih **"Bluetooth"**
   - Tunggu, pilih **"ESP32_Seismograph"**
   - Klik **"Pair"** atau **"Connect"**

4. **Verifikasi Pairing**:
   - ESP32_Seismograph harus muncul di daftar "Paired devices"

### Langkah 2: Cari COM Port Bluetooth

1. **Buka Device Manager**:
   - Klik kanan Start → Device Manager
   - Atau: Win + X → Device Manager

2. **Expand "Ports (COM & LPT)"**:
   - Cari port dengan nama yang mengandung "Bluetooth" atau "ESP32"
   - Misalnya: "Standard Serial over Bluetooth link (COM10)"

3. **Catat nomor COM Port** (contoh: COM10, COM5, dll)

### Langkah 3: Update main.py

Edit file `main.py`, pada bagian `__init__()`:

```python
self.USE_BLUETOOTH = True
self.SERIAL_PORT = 'COM10'  # Ganti dengan COM port Bluetooth Anda
```

---

## 🐧 Setup Linux (Ubuntu/Debian)

### Langkah 1: Install BlueZ Tools

```bash
sudo apt-get update
sudo apt-get install bluez bluez-tools
```

### Langkah 2: Pair ESP32

1. **Scan untuk device Bluetooth**:
   ```bash
   hcitool scan
   ```
   
   Output contoh:
   ```
   Scanning ...
       AA:BB:CC:DD:EE:FF   ESP32_Seismograph
   ```

2. **Catat MAC Address** ESP32 (contoh: AA:BB:CC:DD:EE:FF)

3. **Pair device**:
   ```bash
   bluetoothctl
   ```
   
   Di dalam bluetoothctl:
   ```
   power on
   agent on
   default-agent
   scan on
   ```
   
   Tunggu sampai ESP32_Seismograph muncul, lalu:
   ```
   pair AA:BB:CC:DD:EE:FF
   trust AA:BB:CC:DD:EE:FF
   connect AA:BB:CC:DD:EE:FF
   ```

### Langkah 3: Buat Serial Port Binding

```bash
sudo rfcomm bind /dev/rfcomm0 AA:BB:CC:DD:EE:FF 1
```

### Langkah 4: Set Permissions

```bash
sudo chmod 666 /dev/rfcomm0
```

### Langkah 5: Update main.py

Edit file `main.py`, pada bagian `__init__()`:

```python
self.USE_BLUETOOTH = True
self.SERIAL_PORT = '/dev/rfcomm0'
```

---

## ▶️ Menjalankan Aplikasi

### Langkah 1: Install Python Dependencies

```bash
pip install pyserial PyQt5 pyqtgraph numpy
```

### Langkah 2: Jalankan Aplikasi

```bash
python main.py
```

### Langkah 3: Verifikasi Koneksi

Jika berhasil, Anda akan melihat:
- Console: "Koneksi Bluetooth berhasil di [port] @ 115200 bps"
- GUI menampilkan data real-time dari sensor
- Grafik mulai bergerak

### Langkah 4: Test Kontrol Buzzer

1. Klik tombol **"ON"** pada salah satu buzzer di panel "BUZZER CONTROL"
2. Buzzer di ESP32 harus menyala
3. Klik tombol **"OFF"** untuk mematikan

---

## 🔍 Troubleshooting

### ❌ "Bluetooth is not enabled" Error saat Compile ESP32

**Solusi**:
- Arduino IDE: Tools → Partition Scheme → Pilih yang bukan "No OTA"
- Pastikan board yang dipilih adalah ESP32 (bukan ESP8266)
- Coba board definition lain: ESP32 Dev Module, ESP32 WROVER Module, dll

### ❌ ESP32 tidak muncul di daftar Bluetooth

**Solusi**:
1. Reset ESP32 (tekan tombol RESET)
2. Pastikan kode sudah di-upload dengan benar
3. Cek Serial Monitor - harus ada pesan "Bluetooth Device is ready to pair"
4. Coba scan ulang Bluetooth di komputer
5. Pastikan jarak ESP32 tidak terlalu jauh (< 10 meter)

### ❌ "Permission denied" di Linux

**Solusi**:
```bash
sudo chmod 666 /dev/rfcomm0
# Atau tambahkan user ke dialout group:
sudo usermod -a -G dialout $USER
# Logout dan login kembali
```

### ❌ Port tidak ditemukan di macOS

**Solusi**:
1. Unpair dan pair ulang ESP32
2. Cek dengan:
   ```bash
   ls -la /dev/tty.* | grep -i esp
   ```
3. Restart Bluetooth:
   ```bash
   sudo killall -HUP blued
   ```

### ❌ "Resource busy" atau "Port already in use"

**Solusi**:
1. Tutup aplikasi lain yang menggunakan port tersebut
2. Di Terminal/CMD:
   - Windows: Cek di Device Manager, disable lalu enable port
   - macOS/Linux: 
     ```bash
     sudo lsof | grep tty.ESP32
     # Kill process yang menggunakan port
     ```

### ❌ Data tidak muncul di GUI

**Solusi**:
1. Pastikan koneksi Bluetooth berhasil (cek console Python)
2. Cek Serial Monitor ESP32 - data harus terkirim
3. Pastikan format data benar (5 nilai dipisah koma)
4. Reset ESP32 dan restart aplikasi Python
5. Cek `self.SERIAL_PORT` di main.py sudah benar

### ❌ Koneksi terputus-putus

**Solusi**:
1. Pastikan power supply ESP32 stabil
2. Kurangi jarak antara ESP32 dan komputer
3. Hindari interferensi WiFi/Bluetooth lain
4. Coba turunkan baud rate ke 57600 atau 38400

---

## 📊 Format Data yang Dikirim

ESP32 mengirim data via Bluetooth dalam format CSV:
```
Ax,Ay,RollAngle,PitchAngle,Az\n
```

Contoh:
```
0.123,-0.045,12.34,-5.67,0.987
```

**Keterangan**:
- `Ax`: Akselerasi sumbu X (g)
- `Ay`: Akselerasi sumbu Y (g)
- `RollAngle`: Sudut Roll (derajat)
- `PitchAngle`: Sudut Pitch (derajat)
- `Az`: Akselerasi sumbu Z (g)

---

## 🎛️ Kontrol Buzzer via Bluetooth

Format perintah dari Python ke ESP32:
```
BUZ<nomor>,<state>\n
```

Contoh:
- `BUZ1,1\n` - Nyalakan Buzzer 1
- `BUZ1,0\n` - Matikan Buzzer 1
- `BUZ2,1\n` - Nyalakan Buzzer 2
- `BUZ3,0\n` - Matikan Buzzer 3

---

## 💡 Tips & Best Practices

1. **Range Bluetooth**: Jarak optimal < 10 meter tanpa penghalang
2. **Power Supply**: Gunakan power supply yang stabil (5V 1A minimum)
3. **Battery Operation**: ESP32 + Bluetooth konsumsi ~240mA, gunakan battery 2000mAh+ untuk operasi portable
4. **Debug**: Gunakan Serial Monitor USB sambil test Bluetooth untuk debug
5. **Reconnection**: Jika koneksi putus, aplikasi Python harus di-restart

---

## 🔄 Switching antara Bluetooth dan USB

Di file `main.py`, ubah variable `USE_BLUETOOTH`:

**Untuk Bluetooth**:
```python
self.USE_BLUETOOTH = True
self.SERIAL_PORT = '/dev/tty.ESP32_Seismograph'  # atau COM port di Windows
```

**Untuk USB Serial**:
```python
self.USE_BLUETOOTH = False
self.SERIAL_PORT = 'COM10'  # atau /dev/ttyUSB0 di Linux
```

---

## 📞 Support

Jika masih ada masalah:
1. Cek koneksi hardware
2. Pastikan semua library ter-install
3. Cek Serial Monitor untuk error messages
4. Verify ESP32 board definition di Arduino IDE

---

**Selamat menggunakan ESP32 Seismograph dengan koneksi Bluetooth! 🎉**
