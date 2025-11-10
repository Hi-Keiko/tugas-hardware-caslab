import sys
import numpy as np
import os
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget, QMessageBox, QLabel, QLCDNumber
from PyQt5.uic import loadUi

import pyqtgraph as pg
# import serial # Dinonaktifkan untuk simulasi
import time
from Resource import rscseis # Asumsi file ini ada
# Asumsi: File UI 'seismograph.ui' ada dan memiliki QWidget bernama 'yawac' dan QLabel 'azacc_value_label'.

class SimulatedDashboard(QMainWindow):
    def __init__(self):
        super(SimulatedDashboard, self).__init__()
        # Memuat UI dari file .ui. 
        try:
            loadUi("seismograph.ui", self)
        except FileNotFoundError:
            print("PERINGATAN: File 'seismograph.ui' tidak ditemukan. Pastikan Anda memiliki file UI yang benar.")
            pass

        self.showMaximized()

        # --- KONFIGURASI SIMULASI ---
        self.MAX_POINTS = 500  # Maksimum titik yang ditampilkan pada grafik
        self.time_data = list(range(self.MAX_POINTS))
        self.SIMULATION_RUNNING = True
        
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

        # --- DATA 5: AZ ACCEL (Az) ---
        self.az_data = [1.0] * self.MAX_POINTS # Disimpan dan akan diplot di yawac
        self.yawacc_curve = None # Kurva untuk plot Az di widget 'yawac'

        self.setup_graphs()
        
        # INISIALISASI LABEL SUDUT (Untuk menampilkan nilai numerik)
        self.setup_labels()
        
        #Setup Timer untuk pembaruan grafik
        self.timer = QtCore.QTimer()
        self.timer.setInterval(50)  # Update setiap 50 ms (20 kali per detik)
        
        # PENTING: Koneksikan ke fungsi simulasi
        self.timer.timeout.connect(self.simulate_and_update_plot)
        self.timer.start()


    def setup_graphs(self):
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
            print(f"ERROR: Gagal menyiapkan grafik di 'pitchang'. Pastikan QWidget 'pitchang' ada di file seismograph.ui. Detail Error: {e}")
        
        # =======================================================
        # 5. INISIALISASI GRAFIK YAW ACCEL (yawac) - PLOT AZ
        # =======================================================
        try:
            self.yaw_accel_plot_widget = pg.PlotWidget(self.yawac)
            layout_yaw_accel = QVBoxLayout(self.yawac)
            layout_yaw_accel.setContentsMargins(0, 0, 0, 0)
            layout_yaw_accel.addWidget(self.yaw_accel_plot_widget)
            self.yaw_accel_plot_widget.setBackground('w')
            # Gunakan judul yang sesuai untuk Az di sini
            self.yaw_accel_plot_widget.setTitle("Yaw/Vertical Acceleration (Akselerasi Sumbu Z)") 
            self.yaw_accel_plot_widget.setLabel('left', "Nilai Data (g)")
            self.yaw_accel_plot_widget.setLabel('bottom', "Waktu / Titik Data")
            # Memplot data Az (self.az_data) di widget 'yawac'
            self.yawacc_curve = self.yaw_accel_plot_widget.plot(self.time_data, self.az_data, pen=pg.mkPen(color=(255, 165, 0), width=2)) # Oranye
        except Exception as e:
            print(f"ERROR: Gagal menyiapkan grafik di 'yawac'. Pastikan QWidget 'yawac' ada di file seismograph.ui. Detail Error: {e}")


    def setup_labels(self):
        # INISIALISASI LABEL SUDUT (Untuk menampilkan nilai numerik)
        self.rollang_value_label = self.findChild(QLabel, 'rollang_value_label')
        self.pitchang_value_label = self.findChild(QLabel, 'pitchang_value_label')
        
        # LABEL BARU: Asumsi ada QLabel untuk nilai Az Accel
        self.azacc_value_label = self.findChild(QLabel, 'azacc_value_label')
        
        if self.rollang_value_label and self.pitchang_value_label:
            self.rollang_value_label.setText("Roll: 0.00 °")
            self.pitchang_value_label.setText("Pitch: 0.00 °")
        
        if self.azacc_value_label:
             self.azacc_value_label.setText("Az: 1.00 g") # Nilai awal untuk Az
        else:
            print("Peringatan: QLabel untuk nilai Az ('azacc_value_label') tidak ditemukan. Nilai numerik Az tidak akan ditampilkan.")

        
    def generate_simulated_data(self):
        """Menghasilkan 5 data acak yang menyerupai output MPU yang stabil."""
        # Fluktuasi kecil (noise)
        noise = 0.01 
        angle_noise = 0.5

        # Ax dan Ay: sekitar 0g
        Ax = np.random.normal(loc=0.0, scale=noise)
        Ay = np.random.normal(loc=0.0, scale=noise)
        # Az: sekitar +1.0g (gravitasi)
        Az = np.random.normal(loc=1.0, scale=noise)

        # Roll dan Pitch: sekitar 0 derajat (sensor stabil)
        RollAngle = np.random.normal(loc=0.0, scale=angle_noise)
        PitchAngle = np.random.normal(loc=0.0, scale=angle_noise)
        
        # Untuk tujuan simulasi PGA/PGV/PGD, kita tambahkan spike sesekali
        if np.random.rand() < 0.01: # 1% kemungkinan simulasi 'gempa' kecil
            Ax += np.random.uniform(-0.5, 0.5)
            Ay += np.random.uniform(-0.5, 0.5)
            Az += np.random.uniform(-0.5, 0.5)

        # Output 5 data dalam format float: Ax, Ay, RollAngle, PitchAngle, Az
        return Ax, Ay, RollAngle, PitchAngle, Az

    
    def simulate_and_update_plot(self):
        """Menghasilkan data simulasi dan memperbarui grafik dan label."""
        if self.SIMULATION_RUNNING:
            try:
                # Dapatkan 5 nilai dari generator simulasi
                Ax, Ay, RollAngle, PitchAngle, Az = self.generate_simulated_data()

                # --- Update Data Grafis ---
                
                # 1. Roll Accel (Ax)
                self.roll_data[:-1] = self.roll_data[1:]
                self.roll_data[-1] = Ax
                
                # 2. Pitch Accel (Ay)
                self.pitch_data[:-1] = self.pitch_data[1:]
                self.pitch_data[-1] = Ay

                # 3. Roll Angle
                self.roll_angle_data[:-1] = self.roll_angle_data[1:]
                self.roll_angle_data[-1] = RollAngle

                # 4. Pitch Angle
                self.pitch_angle_data[:-1] = self.pitch_angle_data[1:]
                self.pitch_angle_data[-1] = PitchAngle

                # 5. Az Accel (Diplot di yawac)
                self.az_data[:-1] = self.az_data[1:]
                self.az_data[-1] = Az

                # --- UPDATE NILAI NUMERIK (LABEL) ---
                if hasattr(self, 'rollang_value_label') and self.rollang_value_label:
                    self.rollang_value_label.setText(f"Roll: {RollAngle:.2f} °")
                if hasattr(self, 'pitchang_value_label') and self.pitchang_value_label:
                    self.pitchang_value_label.setText(f"Pitch: {PitchAngle:.2f} °")
                # Update label Az
                if hasattr(self, 'azacc_value_label') and self.azacc_value_label:
                    self.azacc_value_label.setText(f"Az: {Az:.2f} g")
                
                # Perbarui SEMUA kurva plot
                self.rollacc_curve.setData(self.time_data, self.roll_data)
                self.pitchacc_curve.setData(self.time_data, self.pitch_data)
                self.rollangle_curve.setData(self.time_data, self.roll_angle_data)
                self.pitchangle_curve.setData(self.time_data, self.pitch_angle_data)
                # Kurva Az yang baru di widget 'yawac'
                if self.yawacc_curve:
                    self.yawacc_curve.setData(self.time_data, self.az_data)
                
            except Exception as e:
                print(f"Error saat memproses data simulasi: {e}")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    # Gunakan SimulatedDashboard
    ui = SimulatedDashboard() 
    ui.show()
    sys.exit(app.exec_())