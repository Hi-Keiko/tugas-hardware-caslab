import sys
import numpy as np
import os
import math
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


        self.SERIAL_PORT = 'COM10'  
        self.BAUD_RATE = 115200
        self.serial_connection = None


        self.MAX_POINTS = 500  # Maksimum titik yang ditampilkan pada grafik
        self.time_data = list(range(self.MAX_POINTS))
        self.DT = 0.20 #Data dikirim tiap 0.2 detik

        # PEMBATASAN BUAT DATA

        # DATA 1: ROLL ACCEL (Ax)
        self.roll_data = [0] * self.MAX_POINTS
        self.rollacc_curve = None

        # DATA 2: PITCH ACCEL (Ay)
        self.pitch_data = [0] * self.MAX_POINTS
        self.pitchacc_curve = None

        # DATA 3: ROLL ANGLE (Sudut Roll)
        self.roll_angle_data = [0] * self.MAX_POINTS
        self.rollangle_curve = None
       
        # DATA 4: PITCH ANGLE (Sudut Pitch)
        self.pitch_angle_data = [0] * self.MAX_POINTS
        self.pitchangle_curve = None

        # Data 5: Yaw Acc
        self.yaw_data = [1.0] * self.MAX_POINTS #default 1.0 gravitasi
        self.yawacc_curve = None


        #Variabel buat hitung PGA, PGV, PGD
        self.peak_pga_window = 0.0        
        self.pga_window_size = 10          # pengambilan 10 data = update tiap 2 detik
        self.pga_counter = 0              

        # Variabel inisialisasi untuk integrasi (kecepatan dan perpindahan)
        self.current_vel = 0.0 
        self.current_disp = 0.0 
        self.peak_vel_g = 0.0              
        self.peak_disp_g = 0.0
        self.current_mmi = 0.0            


        self.pga_label = None
        self.pgv_label = None
        self.pgd_label = None
        self.mmi_label = None

        self.pga_unit_label = "g"  
        self.pgv_unit_label = "gs" 
        self.pgd_unit_label = "gs²" 



        self.STATIC_BIAS = 1.0 #kalibrasi Az


        # INISIALISASI GRAFIK

        #1. Ax
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



        #2. Ay

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



        #3. Roll Angle
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



        # 4. pitch angle

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
            print(f"Error saat menyiapkan grafik di 'pitchang'. Error: {e}")



        # 5. yawac
        try:

            self.yaw_plot_widget = pg.PlotWidget(self.yawac)
            layout_yaw = QVBoxLayout(self.yawac)
            layout_yaw.setContentsMargins(0, 0, 0, 0)
            layout_yaw.addWidget(self.yaw_plot_widget)

            self.yaw_plot_widget.setBackground('w')
            self.yaw_plot_widget.setTitle("Yaw Acceleration (Akselerasi Sumbu Z) - Getaran Murni")
            self.yaw_plot_widget.setLabel('left', "Nilai Data (g)")
            self.yaw_plot_widget.setLabel('bottom', "Waktu / Titik Data")
            self.yawacc_curve = self.yaw_plot_widget.plot(self.time_data, self.yaw_data, pen=pg.mkPen(color=(255, 165, 0), width=2))

       
        except Exception as e:

            print(f"Error saat menyiapkan grafik di 'yawac'. Error: {e}")


        def setup_value_display(container_name, title, initial_value, unit, decimal_places=4):
            #nyiapin QLabel dalam QWidget Container
            container = self.findChild(QWidget, container_name)
            if container:
                layout = QVBoxLayout(container)
                layout.setContentsMargins(10, 10, 10, 10)
            
                # Label Judul
                title_label = QLabel(title)
                title_label.setFont(QFont("Arial", 10))
                title_label.setStyleSheet("color: #333; font-weight: bold;")
              
                # Label Nilai
                value_label = QLabel(f"{initial_value:.4f} {unit}")
                value_label.setFont(QFont("Arial", 20, QFont.Bold))
                value_label.setStyleSheet("color: #7ed975;") # Warna biru
                value_label.setAlignment(QtCore.Qt.AlignCenter)


                layout.addWidget(title_label)
                layout.addWidget(value_label)

                return value_label

            else:
                print(f"Peringatan: QWidget kontainer '{container_name}' tidak ditemukan di UI.")
                return None



        self.pga_label = setup_value_display('pga', "PGA (Peak Acceleration)", self.peak_pga_window, self.pga_unit_label)
        self.pgv_label = setup_value_display('pgv', "PGV (Peak Velocity)", self.peak_vel_g, self.pgv_unit_label)
        self.pgd_label = setup_value_display('pgd', "PGD (Peak Displacement)", self.peak_disp_g, self.pgd_unit_label)
        self.mmi_label = setup_value_display('mmi', "MMI (Modified Mercalli Intensity)", self.current_mmi, "MMI", decimal_places=1)



        self.setup_serial()


        self.timer = QtCore.QTimer()
        self.timer.setInterval(50)  # Update setiap 50 ms (20 kali per detik)
        self.timer.timeout.connect(self.update_plot)
        self.timer.start()



    def setup_serial(self):

        #Koneksi port ke serial
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



    def calculate_mmi(self, pga_g):

        #Menghitung mmi

        GRAVITY_CMS2 = 980.665
        pga_cms2 = pga_g * GRAVITY_CMS2


        if pga_cms2 <= 0:
            return 0.0

        log_pga = math.log10(pga_cms2)

 
        if log_pga <= 1.6:
            # MMI = 2.27 + 1.647 * log(PGA)
            mmi = 2.27 + 1.647 * log_pga

        else:
            # MMI = -1.361 + 1.647 * log(PGA)
            mmi = -1.361 + 1.647 * log_pga

        return round(mmi, 1)

       

       

    def update_plot(self):
        #untuk updating plot
        if self.serial_connection and self.serial_connection.isOpen():
            try:
                 while self.serial_connection.in_waiting > 0:
                    line = self.serial_connection.readline().decode('utf-8').strip()

                    if line:
                        data_parts = line.split(',')


                        if len(data_parts) == 5:

                            try:
                                new_roll_accel = float(data_parts[0])    # Data 1: Ax
                                new_pitch_accel = float(data_parts[1])   # Data 2: Ay
                                new_roll_angle = float(data_parts[2])    # Data 3: Roll Angle
                                new_pitch_angle = float(data_parts[3])   # Data 4: Pitch Angle
                                new_yaw_accel = float(data_parts[4])   # Data 5: Az

                                detrended_yaw_accel = new_yaw_accel - self.STATIC_BIAS



                                # Update data


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

                                # 5. Yaw Accel
                                self.yaw_data[:-1] = self.yaw_data[1:]
                                self.yaw_data[-1] = detrended_yaw_accel



                                self.rollacc_curve.setData(self.time_data, self.roll_data)
                                self.pitchacc_curve.setData(self.time_data, self.pitch_data)
                                self.rollangle_curve.setData(self.time_data, self.roll_angle_data)
                                self.pitchangle_curve.setData(self.time_data, self.pitch_angle_data)
                                self.yawacc_curve.setData(self.time_data, self.yaw_data)




                                total_acc = math.sqrt(new_roll_accel**2 + new_pitch_accel**2 + new_yaw_accel**2)
                                ground_accel = abs(total_acc - 1.0)


                                self.peak_pga_window = max(self.peak_pga_window, ground_accel)

                                #integrasi PGV
                                self.current_vel += ground_accel * self.DT
                                self.peak_vel_g = max(self.peak_vel_g, abs(self.current_vel)) # Lacak puncak PGV

                                #integrasi PGD
                                self.current_disp += self.current_vel * self.DT
                                self.peak_disp_g = max(self.peak_disp_g, abs(self.current_disp)) # Lacak puncak PGD


                                self.pga_counter += 1


                                if self.pga_counter >= self.pga_window_size:

                                    self.current_mmi = self.calculate_mmi(self.peak_pga_window)
                                    if self.pga_label:


                                        self.pga_label.setText(f"{self.peak_pga_window:.4f} {self.pga_unit_label}")
                                    if self.pgv_label:
                                        self.pgv_label.setText(f"{self.peak_vel_g:.4f} {self.pgv_unit_label}")
                                    if self.pgd_label:
                                        self.pgd_label.setText(f"{self.peak_disp_g:.4f} {self.pgd_unit_label}")
                                    if self.mmi_label:
                                        self.mmi_label.setText(f"{self.current_mmi:.1f} MMI")

                                           
                                    # Reset Nilai Puncak dan Integrasi 

                                    self.peak_pga_window = 0.0
                                    self.peak_vel_g = 0.0
                                    self.peak_disp_g = 0.0

                                   
                                    # Penting: Reset juga integrasi untuk menghindari drift permanen 
                                    self.current_vel = 0.0
                                    self.current_disp = 0.0
                                    self.pga_counter = 0



                            except ValueError:
                                print(f"Data serial tidak valid (bukan angka) diterima: {line}")
                            except Exception as e:
                                print(f"Error saat memproses data: {e}")
                        else:
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
