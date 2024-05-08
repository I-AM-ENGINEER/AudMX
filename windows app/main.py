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
from PySide6.QtCore import QIODevice, QTimer, QObject, Signal
from PySide6.QtGui import QIcon
from serialLib import seriall
import setStyle_Black_Or_White

dict_monitor_global = {}
old_screen_skale = 1
theme = int
class SystemTrayIcon(QtWidgets.QSystemTrayIcon):           #класс приложения в трее
    flag_warning = True
    SignalLIght1 = Signal()
    SignalLIght2 = Signal()
    SignalLIght3 = Signal()
    def __init__(self, icon, parent=None):
        QtWidgets.QSystemTrayIcon.__init__(self, icon, parent)

        self.menu = QtWidgets.QMenu(parent)

        self.menu_light = self.menu.addMenu("light")
        self.Action1 = self.menu.addAction("off warning")
        self.Action2 = self.menu.addAction("Action2")
        self.exitAction = self.menu.addAction("EXIT")

        self.setContextMenu(self.menu)

        self.Action_light1 = self.menu_light.addAction("white")
        self.Action_light2 = self.menu_light.addAction("wave")
        self.Action_light3 = self.menu_light.addAction("level_value")
        self.Action_light1.triggered.connect(self.action_light1)
        self.Action_light2.triggered.connect(self.action_light2)
        self.Action_light2.triggered.connect(self.action_light3)
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
    def action_light1(self):
        self.SignalLIght1.emit()

    def action_light2(self):
        self.SignalLIght2.emit()

    def action_light3(self):
        self.SignalLIght3.emit()

class MainClass(QtWidgets.QWidget):
    volLevelApp = []
    num_load_icon = 0
    teat_perer = 0
    # dictVolumeApp = {}
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
        self.ser = seriall(vid, pid, 1000000)

        self.ser.SignalSerialStartOk.connect(self.startMassege)
        self.ser.SignalReadButton.connect(lambda comand: self.keyPleerHandle(comand))
        self.ser.SignalReadVoluem.connect(lambda comand: self.levelVolHandle(comand))
        self.ser.SignalSetIcon.connect(lambda ans: self.loadIconOnESP(ans))


        self.audioSessions = AudioUtilities.GetAllSessions()

        self.timer_2 = QTimer()
        self.timer_2.timeout.connect(self.updateStyleUI)
        self.timer_2.setInterval(30000)
        self.timer_2.start()
        self.updateStyleUI()
        # self.process_folder(".\\icon")
        # for image_byte in range(self.process_folder(".\\icon")):
        #     pass
        self.timer_ser_con = QTimer()
        self.timer_ser_con.timeout.connect(self.ser.startSerialAutoConnect)
        self.timer_ser_con.setInterval(1500)
        self.timer_ser_con.start()

        self.timer_test = QTimer()
        self.timer_test.timeout.connect(self.testFunc)
        self.timer_test.setInterval(100)


        self.trayIcon.SignalLIght1.connect(self.handleSignalLIght1)
        self.trayIcon.SignalLIght2.connect(self.handleSignalLIght2)
        self.trayIcon.SignalLIght3.connect(self.handleSignalLIght3)

        self.mas_icon = [[icon, num] for num, icon in enumerate(self.process_folder(".\\icon"))]


    def updateStyleUI(self):
        """
        #функция вызываеммая таймером и обновляющяя стиль приложения
        :return: None
        """
        global theme
        cssStyle, themeBW = setStyle_Black_Or_White.getStyleBW()
        if theme != themeBW:
            self.setStyleSheet(cssStyle)

    def readINIfile(self):
        """
        #читает ини файл с некоторыми наситройками
        :return: None
        """
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

    def keyPleerHandle(self, comand: str) -> None:
        """
        #обработчик команд из сериал порта и иниирующий нажатия кнопок плеера
        :param comand: строка с командой типа ''
        :return: NONE
        """
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

    def levelVolHandle(self, comand: str) -> None:
        """
        #обработчик команд из сериал порта и выставляющий нужый уровень громкости
        :param comand: строка с командой типа ''
        :return: NONE
        """
        comand = str(comand).split("|")
        if (len(comand) != 5):
            return
        for num_app, volume_level in enumerate(comand):

            for session in self.audioSessions:
                volume = session._ctl.QueryInterface(ISimpleAudioVolume)

                if session.Process and session.Process.name() == self.volLevelApp[num_app][0] + ".exe":
                    # print("volume.GetMasterVolume(): %s" % volume.GetMasterVolume())
                    try:
                        volume_level = int(volume_level)
                    except:
                        return
                    volume.SetMasterVolume(float(volume_level)/1024, None)
                    self.volLevelApp[num_app][1] = True
                    break

            # if self.trayIcon.flag_warning:
            #     if (self.volLevelApp[num_app][1] == True):
            #         self.trayIcon.showMessage("ERROR", self.volLevelApp[num_app][0] + '.exe not found')
            #         self.volLevelApp[num_app][1] = False
    def handleSignalLIght1(self):
        self.ser.writeSerial("SET_LIGHT:white")
    def handleSignalLIght2(self):
        self.ser.writeSerial("SET_LIGHT:wave")
    def handleSignalLIght3(self):
        self.ser.writeSerial("SET_LIGHT:level_value")

    def process_folder(self, folder_path: str) ->  list[bytes]:
        """
        #читает иконки из папки и прогоняет их через преобразование
        :param folder_path: - относительный путь к папку и иконками
        :return: массив байтовых строк
        """
        byte_arrays = []
        # Проходим по всем файлам в папке
        for filename in os.listdir(folder_path):
            # Проверяем, что файл имеет расширение .bmp
            # i = 0
            if filename.endswith(".bmp"):
                if (len(self.volLevelApp) > 4):
                    return byte_arrays
                self.volLevelApp.append([str(filename[8:-4]), True])
                # self.dictVolumeApp[i] = filename[8:-4]
                # i += 1
                # print(filename[2:-4])
                # Получаем полный путь к файлу
                file_path = os.path.join(folder_path, filename)
                # Преобразуем изображение в байтовый массив и добавляем его в список
                byte_array = self.bmp_to_byte_array(file_path)

                byte_arrays.append(byte_array)

        return byte_arrays


    def bmp_to_byte_array(self, image_path: str) ->  bytes:
        """
        #преобразование картинок в байт массив
        :param image_path: - путь к иконками
        :return: байт массив
        """
        # Открываем изображение с помощью Pillow
        img = Image.open(image_path)
        # Проверяем, является ли изображение монохромным
        if img.mode != '1':
            raise ValueError("Изображение не является монохромным")
        # Получаем данные изображения в виде байтов
        img_bytes = img.tobytes()
        print("bmp_to_byte_array: ", img_bytes)
        return img_bytes

    def startMassege(self):
        """
        #вызываеться послесигнала о подключении и запускает перврначальную настройку(записывает картинки в микщер)
        :return:
        """

        self.timer_ser_con.stop()
        self.loadIconOnESP()

    def loadIconOnESP(self, ans = 0):
        # if (len(self.mas_icon[self.num_load_icon][0]) < self.num_load_icon + 1):
        #     return
        # self.num_load_icon += ans
        print(len(self.mas_icon[self.num_load_icon][0]))
        # self.ser.writeSerial("SET_ICON 1\r\n")
        self.ser.writeSerial("SET_ICON " + str(self.mas_icon[self.num_load_icon][1]) + "\n")
        self.timer_test.start()

        # for i in range(len(self.mas_icon[self.num_load_icon][0])):
        #     self.ser.writeByteSerial(self.mas_icon[self.num_load_icon][0][i])





    def testFunc(self):
        self.teat_perer += 1
        print(len(self.mas_icon[self.num_load_icon][0][(self.teat_perer - 1) * 64:self.teat_perer * 64]) , self.teat_perer, self.num_load_icon)
        self.ser.writeByteSerial(self.mas_icon[self.num_load_icon][0][(self.teat_perer - 1) * 64:self.teat_perer * 64])
        if (self.teat_perer == 6):
            self.timer_test.stop()
            self.teat_perer = 0
            self.num_load_icon += 1



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
