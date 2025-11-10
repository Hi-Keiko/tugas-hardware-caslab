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
        # Memuat UI dari file .ui. 
        # Pastikan QWidget 'rollacc', 'pitchacc', 'rollang', dan 'pitchang' ada di dalamnya.
        loadUi("seismograph.ui", self)
        self.showMaximized()

        # --- KONFIGURASI SERIAL ---
        self.SERIAL_PORT = 'COM7'  # Sesuaikan dengan port ESP32 Anda
        self.BAUD_RATE = 115200
        self.serial_connection = None

        self.MAX_POINTS = 500  # Maksimum titik yang ditampilkan pada grafik
        self.time_data = list(range(self.MAX_POINTS))
        
        # --- DATA 1: ROLL ACCEL (Ax) ---
        self.roll_data = [0] * self.MAX_POINTS
        self.rollacc_curve = None
        
        # --- DATA 2: PITCH ACCEL (Ay) ---
        self.pitch_data = [0] * self.MAX_POINTS 
        self.pitchacc_curve = None

        # --- DATA 3: ROLL ANGLE (Sudut Roll) ---
        self.roll_angle_data = [0] * self.MAX_POINTS 
        self.rollangle_curve = None
        
        # --- DATA 4: PITCH ANGLE (Sudut Pitch) ---
        self.pitch_angle_data = [0] * self.MAX_POINTS 
        self.pitchangle_curve = None


        # =======================================================
        # 1. INISIALISASI GRAFIK ROLL ACCEL (rollacc) - Ax
        # =======================================================
        try:
            self.roll_plot_widget = pg.PlotWidget(self.rollacc)
            layout_roll = QVBoxLayout(self.rollacc)
            layout_roll.setContentsMargins(0, 0, 0, 0)
            layout_roll.addWidget(self.roll_plot_widget) 

            self.roll_plot_widget.setBackground('w')
            self.roll_plot_widget.setTitle("Roll Acceleration (Akselerasi Sumbu X)")
            self.roll_plot_widget.setLabel('left', "Nilai Data (g)")
            self.roll_plot_widget.setLabel('bottom', "Waktu / Titik Data")
            self.rollacc_curve = self.roll_plot_widget.plot(self.time_data, self.roll_data, pen=pg.mkPen(color=(0, 0, 255), width=2))
        
        except Exception as e:
            print(f"Error saat menyiapkan grafik di 'rollacc'. Error: {e}")

        # =======================================================
        # 2. INISIALISASI GRAFIK PITCH ACCEL (pitchacc) - Ay
        # =======================================================
        try:
            self.pitch_plot_widget = pg.PlotWidget(self.pitchacc)
            layout_pitch = QVBoxLayout(self.pitchacc)
            layout_pitch.setContentsMargins(0, 0, 0, 0)
            layout_pitch.addWidget(self.pitch_plot_widget)

            self.pitch_plot_widget.setBackground('w')
            self.pitch_plot_widget.setTitle("Pitch Acceleration (Akselerasi Sumbu Y)")
            self.pitch_plot_widget.setLabel('left', "Nilai Data (g)")
            self.pitch_plot_widget.setLabel('bottom', "Waktu / Titik Data")
            self.pitchacc_curve = self.pitch_plot_widget.plot(self.time_data, self.pitch_data, pen=pg.mkPen(color=(255, 0, 0), width=2))
        
        except Exception as e:
            print(f"Error saat menyiapkan grafik di 'pitchacc'. Error: {e}")

        # =======================================================
        # 3. INISIALISASI GRAFIK ROLL ANGLE (rollang) - Sudut Roll
        # =======================================================
        try:
            self.roll_angle_plot_widget = pg.PlotWidget(self.rollang)
            layout_roll_angle = QVBoxLayout(self.rollang)
            layout_roll_angle.setContentsMargins(0, 0, 0, 0)
            layout_roll_angle.addWidget(self.roll_angle_plot_widget) 

            self.roll_angle_plot_widget.setBackground('w')
            self.roll_angle_plot_widget.setTitle("Roll Angle (Sudut Roll)")
            self.roll_angle_plot_widget.setLabel('left', "Sudut (Derajat)")
            self.roll_angle_plot_widget.setLabel('bottom', "Waktu / Titik Data")
            self.rollangle_curve = self.roll_angle_plot_widget.plot(self.time_data, self.roll_angle_data, pen=pg.mkPen(color=(0, 150, 0), width=2))
        
        except Exception as e:
            print(f"Error saat menyiapkan grafik di 'rollang'. Error: {e}")

        # =======================================================
        # 4. INISIALISASI GRAFIK PITCH ANGLE (pitchang) - Sudut Pitch
        # =======================================================
        try:
            # PENTING: Jika error terjadi di sini, periksa file seismograph.ui.
            # Pastikan Anda memiliki QWidget (misalnya QFrame) dengan objectName 'pitchang'.
            self.pitch_angle_plot_widget = pg.PlotWidget(self.pitchang)
            layout_pitch_angle = QVBoxLayout(self.pitchang)
            layout_pitch_angle.setContentsMargins(0, 0, 0, 0)
            layout_pitch_angle.addWidget(self.pitch_angle_plot_widget)

            self.pitch_angle_plot_widget.setBackground('w')
            self.pitch_angle_plot_widget.setTitle("Pitch Angle (Sudut Pitch)")
            self.pitch_angle_plot_widget.setLabel('left', "Sudut (Derajat)")
            self.pitch_angle_plot_widget.setLabel('bottom', "Waktu / Titik Data")
            self.pitchangle_curve = self.pitch_angle_plot_widget.plot(self.time_data, self.pitch_angle_data, pen=pg.mkPen(color=(150, 0, 150), width=2))
        
        except Exception as e:
            # Pesan error yang lebih informatif
            print(f"ERROR: Gagal menyiapkan grafik di 'pitchang'.")
            print(f"Penyebab paling umum adalah QWidget 'pitchang' tidak ditemukan/salah nama di file seismograph.ui.")
            print(f"Detail Error: {e}")





        
        # DARK SYSTEM: INISIALISASI LABEL SUDUT (Untuk menampilkan nilai numerik) - INI GAADA COK, DARK SYSTEM
        try:
            # Asumsi Anda membuat QLabel di UI dengan nama objek 'rollang_value_label' dan 'pitchang_value_label'
            self.rollang_value_label = self.findChild(QLabel, 'rollang_value_label')
            self.pitchang_value_label = self.findChild(QLabel, 'pitchang_value_label')
            
            if self.rollang_value_label and self.pitchang_value_label:
                self.rollang_value_label.setText("Roll: 0.00 °")
                self.pitchang_value_label.setText("Pitch: 0.00 °")
            else:
                 print("Peringatan: QLabel untuk nilai sudut tidak ditemukan (rollang_value_label/pitchang_value_label). Nilai numerik tidak akan ditampilkan.")
        except AttributeError:
            pass 

        # DARK SYSTEM: INISIALISASI LABEL SUDUT (Untuk menampilkan nilai numerik) - INI GAADA COK, DARK SYSTEM




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
                timeout=0.1 # Non-blocking read (waktu tunggu sangat singkat)
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
        """Membaca data serial dan memperbarui grafik dan label."""
        if self.serial_connection and self.serial_connection.isOpen():
            try:
                # Cek apakah ada data yang tersedia untuk dibaca
                while self.serial_connection.in_waiting > 0:
                    # Baca satu baris data
                    line = self.serial_connection.readline().decode('utf-8').strip()
                    
                    if line:
                        data_parts = line.split(',')

                        # PERIKSA: Sekarang diharapkan ada 4 nilai (Ax, Ay, RollAngle, PitchAngle)
                        if len(data_parts) == 4:
                            try:
                                new_roll_accel = float(data_parts[0])    # Data 1: Ax
                                new_pitch_accel = float(data_parts[1])   # Data 2: Ay
                                new_roll_angle = float(data_parts[2])    # Data 3: Roll Angle
                                new_pitch_angle = float(data_parts[3])   # Data 4: Pitch Angle

                                # --- Update Data Grafis ---
                                
                                # 1. Roll Accel
                                self.roll_data[:-1] = self.roll_data[1:]
                                self.roll_data[-1] = new_roll_accel
                                
                                # 2. Pitch Accel
                                self.pitch_data[:-1] = self.pitch_data[1:]
                                self.pitch_data[-1] = new_pitch_accel

                                # 3. Roll Angle
                                self.roll_angle_data[:-1] = self.roll_angle_data[1:]
                                self.roll_angle_data[-1] = new_roll_angle

                                # 4. Pitch Angle
                                self.pitch_angle_data[:-1] = self.pitch_angle_data[1:]
                                self.pitch_angle_data[-1] = new_pitch_angle

                                # --- UPDATE NILAI NUMERIK (LABEL) ---
                                if hasattr(self, 'rollang_value_label') and self.rollang_value_label:
                                    self.rollang_value_label.setText(f"Roll: {new_roll_angle:.2f} °")
                                if hasattr(self, 'pitchang_value_label') and self.pitchang_value_label:
                                    self.pitchang_value_label.setText(f"Pitch: {new_pitch_angle:.2f} °")
                                
                                # Perbarui KEEMPAT kurva plot
                                self.rollacc_curve.setData(self.time_data, self.roll_data)
                                self.pitchacc_curve.setData(self.time_data, self.pitch_data)
                                self.rollangle_curve.setData(self.time_data, self.roll_angle_data)
                                self.pitchangle_curve.setData(self.time_data, self.pitch_angle_data)



                                # BERARTI TARO FUNGSI PENGHITUNG PGA, PGV, PGD DISINI WOI

                                # INI BAGIAN PGA

                                #Hitung Magnitude Acceleration Total









                                
                            except ValueError:
                                print(f"Data serial tidak valid (bukan angka) diterima: {line}")
                            except Exception as e:
                                print(f"Error saat memproses data: {e}")
                        else:
                            # Jika format data tidak sesuai (bukan 4 nilai dipisahkan koma)
                            print(f"Format data serial tidak valid: {line}. Diharapkan 4 nilai dipisahkan koma (Ax,Ay,RollAngle,PitchAngle).")
            except Exception as e:
                print(f"Error saat membaca/memproses serial: {e}")
                if self.serial_connection and self.serial_connection.isOpen():
                    self.serial_connection.close()
                self.serial_connection = None


if __name__ == "__main__":
    app = QApplication(sys.argv)
    ui = Dashboard()
    ui.show()
    sys.exit(app.exec_())
