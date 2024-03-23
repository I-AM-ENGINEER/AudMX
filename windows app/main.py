from pycaw.pycaw import AudioUtilities, ISimpleAudioVolume
from PIL import Image
import os
import sys

from PySide6.QtSerialPort import QSerialPort, QSerialPortInfo
from PySide6.QtWidgets import QApplication, QMenu, QSystemTrayIcon
from PySide6.QtCore import QIODevice, QTimer, QObject
from PySide6.QtGui import QIcon
from serialLib import seriall

import ctype

import win32con
import win32api

import sys
from PySide6 import QtCore, QtGui, QtWidgets
#
class SystemTrayIcon(QtWidgets.QSystemTrayIcon):
    flag_warning = True

    def __init__(self, icon, parent=None):
        QtWidgets.QSystemTrayIcon.__init__(self, icon, parent)

        self.menu = QtWidgets.QMenu(parent)
        self.Action1 = self.menu.addAction("off warning")
        self.Action2 = self.menu.addAction("Action2")
        self.exitAction = self.menu.addAction("EXIT")

        self.setContextMenu(self.menu)

        self.exitAction.triggered.connect(self.exit)
        self.Action1.triggered.connect(self.action1)
        self.Action2.triggered.connect(self.action2)

    def exit(self):
        QtCore.QCoreApplication.exit()

    def action1(self):
        if self.flag_warning:
            self.Action1.setText("on warning")
            self.flag_warning = False
        else:
            self.Action1.setText("off warning")
            self.flag_warning = True


    def action2(self):
        if self.flag_warning:
            self.showMessage("hui", "sosi hui")

class MainClass(QtWidgets.QWidget):
    volLevelApp = []
    def __init__(self):
        super().__init__()
        self.timer = QTimer()
        self.timer.setInterval(2500)

        icon = QIcon("icon.png")
        self.trayIcon = SystemTrayIcon(icon, self)
        self.trayIcon.show()
        pid, vid = self.readINIfile()
        self.ser = seriall(vid, pid, 115200)
        self.timer.timeout.connect(self.ser.startSerialAutoConnect)
        self.ser.SignalSerialStartOk.connect(self.startMassege)
        self.ser.SignalReadButton.connect(lambda comand: self.keyPleerHandle(comand))
        self.ser.SignalReadVoluem.connect(lambda comand: self.levelVolHandle(comand))

        self.audioSessions = AudioUtilities.GetAllSessions()
        self.timer.start()

    def readINIfile(self):
        with open('ini.txt') as f:
            lines = f.readlines()
        if len(lines) > 1:
            if lines[0].find("PID=") != -1 and lines[1].find("VID=") != -1:
                return int(lines[0][4:-1]), int(lines[1][4:])
            else:
                self.trayIcon.showMessage("ERROR", "ERROR don't current PID/VID")
                return 99999, 99999
        else:
            self.trayIcon.showMessage("ERROR", "ERROR don't search ini.txt or current PID/VID")
            return 99999, 99999

    def keyPleerHandle(self, comand):
        comand = str(comand)
        if comand.find('play') != -1:
            win32api.keybd_event(win32con.VK_MEDIA_PLAY_PAUSE, 0, 0, 0)
            win32api.keybd_event(win32con.VK_MEDIA_PLAY_PAUSE, 0, win32con.KEYEVENTF_KEYUP, 0)
        elif comand.find('next') != -1:
            win32api.keybd_event(win32con.VK_MEDIA_NEXT_TRACK, 0, 0, 0)
            win32api.keybd_event(win32con.VK_MEDIA_NEXT_TRACK, 0, win32con.KEYEVENTF_KEYUP, 0)
        elif comand.find('prev') != -1:
            win32api.keybd_event(win32con.VK_MEDIA_PREV_TRACK, 0, 0, 0)
            win32api.keybd_event(win32con.VK_MEDIA_PREV_TRACK, 0, win32con.KEYEVENTF_KEYUP, 0)

    def levelVolHandle(self, comand):
        comand = str(comand)

        for session in self.audioSessions:
            volume = session._ctl.QueryInterface(ISimpleAudioVolume)
            if session.Process and session.Process.name() == self.volLevelApp[0] + ".exe":
                print("volume.GetMasterVolume(): %s" % volume.GetMasterVolume())
                volume.SetMasterVolume(float(self.volLevelApp[2:])/100, None)
            else:
                if self.trayIcon.flag_warning:
                    self.trayIcon.showMessage("ERROR", self.volLevelApp[0] + '.exe not found')

    def process_folder(self, folder_path):
        byte_arrays = []
        # Проходим по всем файлам в папке
        for filename in os.listdir(folder_path):
            # Проверяем, что файл имеет расширение .bmp
            if filename.endswith(".bmp"):

                self.volLevelApp.append(str(filename[3:-4]))
                print(filename[:-4])
                # Получаем полный путь к файлу
                file_path = os.path.join(folder_path, filename)
                # Преобразуем изображение в байтовый массив и добавляем его в список
                byte_array = self.bmp_to_byte_array(file_path)
                byte_arrays.append(byte_array)
        return byte_arrays


    def bmp_to_byte_array(self, image_path):
        # Открываем изображение с помощью Pillow
        img = Image.open(image_path)
        # Проверяем, является ли изображение монохромным
        if img.mode != '1':
            raise ValueError("Изображение не является монохромным")
        # Получаем данные изображения в виде байтов
        img_bytes = img.tobytes()
        return img_bytes

    def startMassege(self):
        self.timer.stop()
        for image_byte in range(self.process_folder(".\\icon")):
            self.ser.writeSerial("SET_ICON:" + str(image_byte))




# print("hui")
if __name__ == '__main__':
    # print("hui")
    app = QApplication(sys.argv)
    app.setQuitOnLastWindowClosed(False)
    window = MainClass()
    sys.exit(app.exec())

