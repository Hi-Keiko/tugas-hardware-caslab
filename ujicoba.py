import sys
import numpy as np
import os
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QMessageBox
from PyQt5.uic import loadUi
from PyQt5.QtWidgets import QToolTip, QLabel, QInputDialog

# --- IMPOR BARU UNTUK GRAFIK DAN SERIAL ---
import pyqtgraph as pg
import serial
import time
import math # Import math untuk fungsi sin() pada simulasi
# ------------------------------------------

# Perhatikan impor ini. Jika 'Resource' adalah file .py yang dihasilkan dari pyrcc5,
# pastikan ia ada di direktori yang sama.
from Resource import rscseis

class Dashboard(QMainWindow):
    """
    Kelas utama untuk dashboard Seismograph.
    Ini memuat tata letak dari file 'seismograph.ui'.
    """
    def __init__(self):
        super().__init__()
        
        # Memuat file UI yang dibuat di Qt Designer.
        try:
            loadUi("seismograph.ui", self)
        except Exception as e:
            print(f"Error loading UI file 'seismograph.ui': {e}")

        # BARIS KODE UNTUK MEMENUHI LAYAR
        self.showMaximized()
        
        # --- KONFIGURASI SERIAL DAN GRAFIK REAL-TIME ---

        # 1. KONFIGURASI SERIAL (SESUAIKAN INI!)
        self.SERIAL_PORT = 'COM3'  # Ganti dengan port serial yang sesuai
        self.BAUD_RATE = 9600
        self.serial_connection = None
        
        # Kontrol Mode Simulasi
        self.is_mocking = False
        
        # Buffer Data
        self.MAX_POINTS = 500  # Maksimum titik yang ditampilkan pada grafik
        # time_data hanya digunakan sebagai indeks X
        self.time_data = list(range(self.MAX_POINTS)) 
        self.roll_data = [0] * self.MAX_POINTS 
        
        # Variabel untuk Simulasi Data
        self.mock_index = 0
        self.mock_amplitude = 50.0

        # 2. INISIALISASI GRAFIK PYQTGRAPH
        try:
            # Buat PlotWidget baru
            self.plot_widget = pg.PlotWidget(self.rollacc)
            # Atur layout di QWidget placeholder 'rollacc'
            layout = QVBoxLayout(self.rollacc)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.addWidget(self.plot_widget)
            
            self.plot_widget.setBackground('w')
            self.plot_widget.setTitle("Pembacaan Serial Real-time (Roll/Akselerasi)")
            self.plot_widget.setLabel('left', "Nilai Data")
            self.plot_widget.setLabel('bottom', "Waktu / Titik Data")
            # Inisialisasi kurva plot (warna biru)
            self.rollacc_curve = self.plot_widget.plot(self.time_data, self.roll_data, pen=pg.mkPen(color=(0, 0, 255), width=2))
        
        except Exception as e:
             print(f"Error saat menyiapkan grafik di 'rollacc': Pastikan 'rollacc' adalah QWidget yang dapat diakses. Error: {e}")
             
        # 3. SETUP KONEKSI SERIAL
        self.setup_serial()

        # 4. SETUP TIMER UNTUK PEMBARUAN GRAFIK
        self.timer = QtCore.QTimer()
        self.timer.setInterval(50)  # Update setiap 50 ms (20 kali per detik)
        self.timer.timeout.connect(self.update_plot)
        self.timer.start()

    def closeEvent(self, event):
        """Menutup koneksi serial secara bersih saat aplikasi ditutup."""
        if self.serial_connection and self.serial_connection.isOpen():
            print(f"Menutup koneksi serial di {self.SERIAL_PORT}...")
            self.serial_connection.close()
        event.accept()

    def setup_serial(self):
        """Mencoba membuat koneksi ke port serial yang ditentukan."""
        try:
            self.serial_connection = serial.Serial(
                self.SERIAL_PORT, 
                self.BAUD_RATE, 
                timeout=0.01 
            )
            print(f"Koneksi serial berhasil di {self.SERIAL_PORT} @ {self.BAUD_RATE} bps (Mode Hardware)")
            self.is_mocking = False # Pastikan mode simulasi mati jika koneksi berhasil
        except serial.SerialException as e:
            print(f"Gagal membuka port serial {self.SERIAL_PORT}: {e}")
            self.serial_connection = None
            self.is_mocking = True # Aktifkan mode simulasi
            print("Beralih ke Mode Simulasi Data.")

            # Tampilkan pesan error dan pemberitahuan mode simulasi
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Warning)
            msg.setText("Gagal Koneksi Serial")
            msg.setInformativeText(f"Gagal koneksi ke port '{self.SERIAL_PORT}'. Beralih ke mode SIMULASI data.")
            msg.setWindowTitle("Peringatan Serial")
            msg.exec_()

    def generate_mock_data(self):
        """Menghasilkan data simulasi (gelombang sinus dengan sedikit noise)."""
        # Data utama berupa gelombang sinus (simulasi osilasi)
        value = self.mock_amplitude * math.sin(self.mock_index * 0.1)
        # Tambahkan sedikit noise acak
        noise = np.random.normal(0, 1) 
        new_value = value + noise
        
        # Update indeks untuk pergerakan gelombang
        self.mock_index += 1
        
        return new_value

    def update_plot(self):
        """Membaca data serial atau menghasilkan data simulasi dan memperbarui grafik."""
        new_value = None

        if self.is_mocking:
            # Ambil data dari simulasi
            new_value = self.generate_mock_data()
        
        elif self.serial_connection and self.serial_connection.isOpen():
            # Ambil data dari serial
            try:
                if self.serial_connection.in_waiting > 0:
                    line = self.serial_connection.readline().decode('utf-8').strip()
                    
                    if line:
                        try:
                            # Asumsi data serial hanya mengirim satu nilai float per baris
                            new_value = float(line)
                            
                        except ValueError:
                            print(f"Data serial tidak valid (bukan angka) diterima: {line}")
                            new_value = None # Jangan perbarui jika data tidak valid
            
            except Exception as e:
                print(f"Error saat membaca/memproses serial: {e}")
                QMessageBox.critical(self, "Error Fatal", "Koneksi serial terputus. Harap restart aplikasi.")
                self.serial_connection.close()
                self.serial_connection = None
                self.is_mocking = True # Beralih ke mode simulasi jika terputus

        if new_value is not None:
            # Geser data lama ke kiri dan tambahkan data baru
            self.roll_data[:-1] = self.roll_data[1:]
            self.roll_data[-1] = new_value

            # Perbarui grafik
            self.rollacc_curve.setData(self.time_data, self.roll_data)
        
# Pengecekan entry point yang benar dalam Python adalah '__main__', 
# terlepas dari nama file skrip.
if __name__ == "__main__":
    pg.setConfigOption('background', 'w') 
    pg.setConfigOption('foreground', 'k') 

    app = QApplication(sys.argv)
    ui = Dashboard()
    ui.show()
    sys.exit(app.exec_())