import sys
import numpy as np
import os
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget, QMessageBox, QPushButton
from PyQt5.uic import loadUi
from PyQt5.QtWidgets import QToolTip, QLabel, QInputDialog
from PyQt5.QtGui import QFont, QPixmap
from PyQt5 import QtCore

import pyqtgraph as pg
import serial
import time

from Resource import rscseis

class Dashboard(QMainWindow):
    def __init__(self):
        super(Dashboard, self).__init__()
        loadUi("seismograph.ui", self)
        self.showMaximized()

        #UNTUK ROLL ACCELERATION=====
        self.SERIAL_PORT = 'COM3'  # Contoh: 'COM3' (Windows) atau '/dev/ttyACM0' (Linux/macOS)
        self.BAUD_RATE = 9600 # Sesuaikan
        self.serial_connection = None

        self.MAX_POINTS = 500  # Maksimum titik yang ditampilkan pada grafik
        self.time_data = list(range(self.MAX_POINTS))
        # Inisialisasi data Y dengan nol
        self.roll_data = [0] * self.MAX_POINTS

        #Inisialisasi Grafik PyQtGraph
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
             print(f"Error saat menyiapkan grafik di 'rollacc'. Error: {e}")

        #Setup Koneksi Serial
        self.setup_serial()

        #Setup Timer untuk pembaruan grafik
        self.timer = QtCore.QTimer()
        self.timer.setInterval(50)  # Update setiap 50 ms (20 kali per detik)
        self.timer.timeout.connect(self.update_plot)
        self.timer.start()

    def setup_serial(self):
        """Mencoba membuat koneksi ke port serial yang ditentukan."""
        try:
            self.serial_connection = serial.Serial(
                self.SERIAL_PORT, 
                self.BAUD_RATE, 
                timeout=0.01 # Non-blocking read (waktu tunggu sangat singkat)
            )
            print(f"Koneksi serial berhasil di {self.SERIAL_PORT} @ {self.BAUD_RATE} bps")
        except serial.SerialException as e:
            print(f"Gagal membuka port serial {self.SERIAL_PORT}: {e}")
            self.serial_connection = None
            # Tampilkan pesan error ke pengguna
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Critical)
            msg.setText("Gagal Koneksi Serial")
            msg.setInformativeText(f"Pastikan perangkat terhubung dan port '{self.SERIAL_PORT}' benar.\nError: {e}")
            msg.setWindowTitle("Error Serial")
            msg.exec_()
        
    def update_plot(self):
        """Membaca data serial dan memperbarui grafik."""
        if self.serial_connection and self.serial_connection.isOpen():
            try:
                # Cek apakah ada data yang tersedia untuk dibaca
                if self.serial_connection.in_waiting > 0:
                    # Baca satu baris data
                    line = self.serial_connection.readline().decode('utf-8').strip()
                    
                    if line:
                        # Coba konversi data menjadi float
                        try:
                            # Asumsi data serial hanya mengirim satu nilai per baris (misalnya: "123.45\n")
                            new_roll_value = float(line)
                            
                            # Geser data lama ke kiri dan tambahkan data baru
                            self.roll_data[:-1] = self.roll_data[1:]
                            self.roll_data[-1] = new_roll_value

                            # Perbarui grafik
                            self.rollacc_curve.setData(self.time_data, self.roll_data)
                            
                        except ValueError:
                            # Abaikan jika data yang diterima bukan angka
                            print(f"Data serial tidak valid (bukan angka) diterima: {line}")

            except Exception as e:
                # Tangani error IO atau decoding (misalnya, jika perangkat dicabut)
                print(f"Error saat membaca/memproses serial: {e}")
                # Matikan koneksi untuk menghindari crash berulang
                self.serial_connection.close()
                self.serial_connection = None
             

if __name__ == "__main__":
    app = QApplication(sys.argv)
    ui = Dashboard()
    ui.show()
    sys.exit(app.exec_())

