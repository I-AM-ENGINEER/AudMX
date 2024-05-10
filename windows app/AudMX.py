from pycaw.pycaw import AudioUtilities, ISimpleAudioVolume, IAudioEndpointVolume
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
from comtypes import CLSCTX_ALL


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
    def masegeIconWarning(self, file_name: str):
        if self.flag_warning:
            self.showMessage("ERROR ICON", "icon: '" + file_name + "' don't have size 60x44px")
    def action_light1(self):
        self.SignalLIght1.emit()

    def action_light2(self):
        self.SignalLIght2.emit()

    def action_light3(self):
        self.SignalLIght3.emit()

class MainClass(QtWidgets.QWidget):
    volLevelApp = []
    last_vol_level = [0,0,0,0,0]
    num_load_icon = 0
    teat_perer = 0
    dictVolumeDBtoProsent = [-65.25,
    -60.49741,
    -54.173828125,
    -50.437278747558594,
    -47.282318115234375,
    -46.02272033691406,
    -42.34019088745117,
    -40.06081008911133,
    -38.07908630371094,
    -36.32617950439453,
    -34.75468063354492,
    -33.33053970336914,
    -32.02846908569336,
    -30.829191207885742,
    -29.7176513671875,
    -28.681884765625,
    -27.71221923828125,
    -26.800716400146484,
    -25.940793991088867,
    -25.126928329467773,
    -24.35443115234375,
    -23.61930274963379,
    -22.918092727661133,
    -22.2478084564209,
    -21.605838775634766,
    -20.989887237548828,
    -20.397926330566406,
    -19.828153610229492,
    -19.278972625732422,
    -18.748943328857422,
    -18.236774444580078,
    -17.741300582885742,
    -17.261470794677734,
    -16.796323776245117,
    -16.344989776611328,
    -15.906672477722168,
    -15.480639457702637,
    -15.06622314453125,
    -14.662806510925293,
    -14.269820213317871,
    -13.886737823486328,
    -13.513073921203613,
    -13.148375511169434,
    -12.792222023010254,
    -12.444223403930664,
    -12.10401439666748,
    -11.771252632141113,
    -11.445619583129883,
    -11.12681770324707,
    -10.814563751220703,
    -10.508596420288086,
    -10.20866584777832,
    -9.914539337158203,
    -9.625996589660645,
    -9.342827796936035,
    -9.064839363098145,
    -8.791844367980957,
    -8.523664474487305,
    -8.260135650634766,
    -8.001096725463867,
    -7.746397495269775,
    -7.49589729309082,
    -7.249458312988281,
    -7.006951332092285,
    -6.768252372741699,
    -6.5332441329956055,
    -6.301812648773193,
    -6.073853492736816,
    -5.849262237548828,
    -5.627941608428955,
    -5.409796714782715,
    -5.194738864898682,
    -4.982679843902588,
    -4.7735395431518555,
    -4.567237854003906,
    -4.363698959350586,
    -4.162849426269531,
    -3.9646193981170654,
    -3.7689411640167236,
    -3.5757486820220947,
    -3.384982109069824,
    -3.196580171585083,
    -3.0104846954345703,
    -2.8266398906707764,
    -2.6449923515319824,
    -2.4654886722564697,
    -2.288081407546997,
    -1.7679541110992432,
    -1.5984597206115723,
    -1.4308334589004517,
    -1.2650364637374878,
    -1.101028561592102,
    -0.9387713074684143,
    -0.7782278060913086,
    -0.6193622946739197,
    -0.4621390104293823,
    -0.3065262734889984,
    -0.15249048173427582,
    0.0,
    0.0]
    def __init__(self):
        super(MainClass, self).__init__()

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
        self.timer_test.setInterval(10)


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
            volume_level = int(volume_level)
            if (self.last_vol_level[num_app] == volume_level):
                continue
            if (self.volLevelApp[num_app][0] == "master"):

                devices = AudioUtilities.GetSpeakers()
                interface = devices.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
                volume = interface.QueryInterface(IAudioEndpointVolume)
                #/print(volume.GetMasterVolumeLevel())
                volume.SetMasterVolumeLevel(self.dictVolumeDBtoProsent[int(volume_level / 10.24)], None)
                continue

            self.audioSessions = AudioUtilities.GetAllSessions()
            for session in self.audioSessions:
                volume = session._ctl.QueryInterface(ISimpleAudioVolume)

                if session.Process and session.Process.name() == self.volLevelApp[num_app][0] + ".exe":
                    # #/print("volume.GetMasterVolume(): %s" % volume.GetMasterVolume())

                    volume.SetMasterVolume(float(volume_level)/1024, None)
                    self.volLevelApp[num_app][1] = True
                    break

    def handleSignalLIght1(self):
        self.ser.writeSerial("SET_LIGHT:white")
    def handleSignalLIght2(self):
        self.ser.writeSerial("SET_LIGHT:wave")
    def handleSignalLIght3(self):
        self.ser.writeSerial("SET_LIGHT:level_value")

    def process_folder(self, folder_path: str) -> list[bytes]:
        """
        #читает иконки из папки и прогоняет их через преобразование
        :param folder_path: - относительный путь к папку и иконками
        :return: массив байтовых строк
        """
        byte_arrays = []
        # Проходим по всем файлам в папке
        for filename in os.listdir(folder_path):
            # Проверяем, что файл имеет расширение .bmp
            if filename.endswith(".bmp"):
                if (len(self.volLevelApp) > 4):
                    return byte_arrays

                file_path = os.path.join(folder_path, filename)
                # Преобразуем изображение в байтовый массив и добавляем его в список
                byte_array = self.bmp_to_byte_array(file_path)
                if (len(byte_array) == 352):
                    self.volLevelApp.append([str(filename[8:-4]), True])
                    byte_arrays.append(byte_array)
                else:
                    self.trayIcon.masegeIconWarning(str(filename[8:-4]))




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
        #/print("bmp_to_byte_array: ", img_bytes)
        return img_bytes

    def startMassege(self):
        """
        #вызываеться послесигнала о подключении и запускает перврначальную настройку(записывает картинки в микщер)
        :return:
        """

        self.timer_ser_con.stop()
        self.loadIconOnESP()

    def loadIconOnESP(self, ans = 0):
        if (self.num_load_icon == 5):
            return
        self.ser.writeSerial("SET_ICON " + str(self.mas_icon[self.num_load_icon][1]) + "\n")
        self.timer_test.start()


    def testFunc(self):
        self.teat_perer += 1
        #/print(len(self.mas_icon[self.num_load_icon][0][(self.teat_perer - 1) * 64:self.teat_perer * 64]) , self.teat_perer, self.num_load_icon)
        self.ser.writeByteSerial(self.mas_icon[self.num_load_icon][0][(self.teat_perer - 1) * 64:self.teat_perer * 64])
        if (self.teat_perer == 6):
            self.timer_test.stop()
            self.teat_perer = 0
            self.num_load_icon += 1



if __name__ == '__main__':
    app = QApplication(sys.argv)
    app.setQuitOnLastWindowClosed(False)
    window = MainClass()  # создаем мейн окно
    sys.exit(app.exec())
