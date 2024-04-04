from pycaw.pycaw import AudioUtilities, ISimpleAudioVolume
from PIL import Image
import os
import sys
import ctype
import win32con
import win32api
from PySide6.QtGui import QWindow
from PySide6 import QtCore, QtGui, QtWidgets
from PySide6.QtSerialPort import QSerialPort, QSerialPortInfo
from PySide6.QtWidgets import QApplication, QMenu, QSystemTrayIcon
from PySide6.QtCore import QIODevice, QTimer, QObject
from PySide6.QtGui import QIcon
from serialLib import seriall
import setStyle_Black_Or_White

dict_monitor_global = {}
old_screen_skale = 1
theme = int
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
    def setFont(self, font):
        self.exitAction.setFont(font)
        self.Action1.setFont(font)
        self.Action2.setFont(font)

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
    def __init__(self, dict_monitor):
        super(MainClass, self).__init__()
        global dict_monitor_global
        dict_monitor_global = dict_monitor
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
        self.timer_2 = QTimer()
        self.timer_2.timeout.connect(self.updateStyleUI)
        self.timer_2.setInterval(10000)
        self.timer_2.start()

        # self.show()


    def updateStyleUI(self):

        global theme
        cssStyle, themeBW = setStyle_Black_Or_White.getStyleBW()
        if theme != themeBW:
            self.setStyleSheet(cssStyle)

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

    def keyPleerHandle(self, comand: str):
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

    def levelVolHandle(self, comand: str):
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
                print(filename[3:-4])
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

    # def editSize(self, set_screen_name='', **kwargs):  # отрисовка окна с учетем размера экрана
    #     global old_screen_skale
    #     if kwargs.get('constrctorTable', False):
    #         scale = old_screen_skale
    #     else:
    #         global dict_monitor_global
    #         scale = dict_monitor_global[set_screen_name]
    #         old_screen_skale = scale
    #
    #     font = QtGui.QFont()
    #     font.setFamily("Yu Gothic UI Semibold")
    #     font.setPointSize(12)
    #     self.trayIcon.setFont(font)
    #     # self.show()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    app.setQuitOnLastWindowClosed(False)
    # window = MainClass()


    dict_monitor = {}
    for screen in app.screens():  # проверяем все мориторы
        window1 = QWindow()
        window1.setScreen(screen)
        dpi = screen.logicalDotsPerInch()
        scale_factor = dpi / 96.0  # assuming default DPI is 96.0
        dict_monitor[screen.name()] = scale_factor

    window = MainClass(dict_monitor)  # создаем мейн окно
    # window.editSize(window.windowHandle().screen().name())
    # # # print(window.windowHandle().screen().name())
    # window.windowHandle().screenChanged.connect(lambda screen: window.editSize(screen.name()))  # подключаем тригер сманы эрана
    # window.windowHandle().windowStateChanged.connect((lambda: print('windowStateChanged')))
    # window.windowHandle().raise_()
    # window.show()
    sys.exit(app.exec())
