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
from PySide6.QtWidgets import QApplication, QMenu, QSystemTrayIcon, QMessageBox
from PySide6.QtCore import QIODevice, QTimer, QObject, Signal
from PySide6.QtGui import QIcon
from serialLib import seriall, SerCDC
import setStyle_Black_Or_White
from comtypes import CLSCTX_ALL
from avto_run_settings import AvtoRun
from volume_socket import SocketVolume
from icon_manager import IcomReader, VaveLight



theme = int
class SystemTrayIcon(QtWidgets.QSystemTrayIcon):           #класс приложения в трее
    flag_warning = True
    __flag_auto_boot = 0
    SignalLIght1 = Signal()
    SignalLIght2 = Signal()
    SignalLIght3 = Signal()

    def __init__(self, icon, auto_boot_flag=0, parent=None):
        QtWidgets.QSystemTrayIcon.__init__(self, icon, parent)
        self.__flag_auto_boot = auto_boot_flag

        self.menu = QtWidgets.QMenu(parent)

        self.menu_light = self.menu.addMenu("light")
        if (self.__flag_auto_boot == 0):
            self.avto_boot_action = self.menu.addAction("ON auto boot")
        else:
            self.avto_boot_action = self.menu.addAction("OFF auto boot")
        self.Action1 = self.menu.addAction("off warning")
        self.Action2 = self.menu.addAction("Action2")
        self.exitAction = self.menu.addAction("EXIT")


        self.Action_light1 = self.menu_light.addAction("white")
        self.Action_light2 = self.menu_light.addAction("wave")
        self.Action_light3 = self.menu_light.addAction("volume_level")

        self.setContextMenu(self.menu)
        self.Action_light1.triggered.connect(self.action_light1)
        self.Action_light2.triggered.connect(self.action_light2)
        self.Action_light3.triggered.connect(self.action_light3)
        self.avto_boot_action.triggered.connect(self.avtoBootAction)
        self.exitAction.triggered.connect(self.exit)
        self.Action1.triggered.connect(self.action1)
        self.Action2.triggered.connect(self.action2)
    def setFont(self, font):
        self.exitAction.setFont(font)
        self.Action1.setFont(font)
        self.Action2.setFont(font)

    def exit(self):
        QtCore.QCoreApplication.exit()
    def avtoBootAction(self):
        self.__flag_auto_boot = not self.__flag_auto_boot
        if (self.__flag_auto_boot == 0):
            self.avto_boot_action.setText("ON auto boot")
            AvtoRun.removeAppToAvtoRun("AudMX")
        else:
            self.avto_boot_action.setText("OFF auto boot")
            AvtoRun.addAppToAvtoRun("AudMX", sys.argv[0])

        
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

    last_process_list = []
    num_load_icon = 0
    teat_perer = 0
    open_process_list = []
    __comand_Buff = ''
    # ser_work = 0
    # mas_icon = []

    __count_presed_button = 0
    dictVolumeDBtoProsent = [-65.25,
                             -64.49741,
                             -58.173828125,
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


        icon = QIcon(os.path.abspath(os.path.join(getattr(sys, '_MEIPASS', os.path.abspath(os.path.dirname(__file__))), 'iconTrayB.png')))
        #icon = QIcon(sys.argv[0][:sys.argv[0].rindex("\\")] + "\\icon.png")
        self.trayIcon = SystemTrayIcon(icon, AvtoRun.readAppToAvtoRun("AudMX"), self)
        self.trayIcon.show()
        pid, vid = self.readINIfile()
        # self.ser = seriall()
        self.ser = SerCDC(True)
        self.ser.setHanglerRead(self.hanglerReadSer)
        # self.audioSessions = AudioUtilities.GetAllSessions()

        self.timer_2 = QTimer()
        self.timer_2.timeout.connect(self.updateStyleUI)
        self.timer_2.setInterval(5000)
        self.timer_2.start()
        self.updateStyleUI()
        self.upDateListOpenProcces()
        self.icon_mass = []

        # self.upDateListMasIcon()

        # self.timer_loadByte = QTimer()
        # self.timer_loadByte.timeout.connect(self.loadByteMasToESP)
        # self.timer_loadByte.setInterval(10)

        self.trayIcon.SignalLIght1.connect(self.handleSignalLIght1)
        self.trayIcon.SignalLIght2.connect(self.handleSignalLIght2)
        self.trayIcon.SignalLIght3.connect(self.handleSignalLIght3)

        self.ser.SignalSerialStartOk.connect(self.startMassege)
        # self.ser.SignalReadButton.connect(lambda comand: self.keyPleerHandle(comand))
        # self.ser.SignalReadVoluem.connect(lambda comand: self.levelVolHandle(comand))
        # self.ser.SignalSetIcon.connect(lambda ans: self.loadIconOnESP(ans))
        # # self.ser.SignalError.connect(lambda ans: self.handleSerError(ans))
        # self.ser.SignalGetIcon.connect(self.handleGetIcon)
        self.ser.autoConnect(vid, pid, 1000000, True)

        # self.timer_light = QTimer()
        # self.timer_light.timeout.connect(self.updateLight)
        # self.timer_light.setInterval(33)
    def hanglerReadSer(self, iner: str):
        if iner.find("BUTTON:") != -1:
            self.keyPleerHandle(iner)
        elif iner.find("|") != -1:
            if (iner != self.__comand_Buff):
                self.__comand_Buff = iner
                self.levelVolHandle(iner)
        elif iner.find("OK") != -1:
            self.loadIconOnESP(1)
        elif iner.find("ERROR: -1") != -1:
            self.loadIconOnESP(0)
        elif iner.find("Send 352 bytes") != -1:
            self.handleGetIcon()

    def upDateListOpenProcces(self):
        # a = AudioUtilities.GetAllSessions()
        self.open_process_list = [[session.Process.name(), session.Process.pid] for session in AudioUtilities.GetAllSessions() if session.Process] + [["master.exe", -1], ["system.exe", -1]]
    # def upDateListMasIcon(self):
    #     self.icon_mass = IcomReader.loadIcons(sys.argv[0][:sys.argv[0].rindex("\\")] + ".\\icon", self.open_process_list, 5)


    def updateStyleUI(self):
        """
        #функция вызываеммая таймером и обновляющяя стиль приложения
        :return: None
        """
        global theme
        cssStyle, themeBW = setStyle_Black_Or_White.getStyleBW()
        if theme != themeBW:
            self.setStyleSheet(cssStyle)
            if themeBW == 0:
                icon = QIcon(os.path.abspath(
                    os.path.join(getattr(sys, '_MEIPASS', os.path.abspath(os.path.dirname(__file__))), 'iconTrayB.png')))
            else:
                icon = QIcon(os.path.abspath(
                    os.path.join(getattr(sys, '_MEIPASS', os.path.abspath(os.path.dirname(__file__))), 'iconTrayW.png')))
            self.trayIcon.setIcon(icon)

        if (self.ser.doesSerWork == 1):
            self.upDateListOpenProcces()
            if (self.last_process_list != self.open_process_list):
                # self.__tmp_icon_mass = []
                IcomReader.setLastLevel(self.icon_mass, 0)
                # if (self.__tmp_icon_mass != self.icon_mass):
                # self.icon_mass = self.__tmp_icon_mass
                self.volLevelApp = []
                self.last_process_list = self.open_process_list
                tempp = IcomReader.loadIcons(sys.argv[0][:sys.argv[0].rindex("\\")] + ".\\icon", self.open_process_list, 5)
                if ([i.name for i in tempp] != [i.name for i in self.icon_mass]):
                    self.icon_mass = tempp
                # self.upDateListMasIcon()
                    self.loadIconOnESP()

    def readINIfile(self):
        """
        #читает ини файл с некоторыми наситройками
        :return: None
        """
        #with open(sys.argv[0][:sys.argv[0].rindex("\\")] + '\\ini.txt') as f:
        with open(os.path.abspath(os.path.join(getattr(sys, '_MEIPASS', os.path.abspath(os.path.dirname(__file__))), 'ini.txt'))) as f:
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
        comand = str(comand).split("|")
        comand[1] = int(comand[1])
        if (comand[0] == "realised"):
            if (self.__count_presed_button > 0):
                self.__count_presed_button = 0
                return
            if comand[1] == 1:
                win32api.keybd_event(win32con.VK_MEDIA_PLAY_PAUSE, 0, 0, 0)
                win32api.keybd_event(win32con.VK_MEDIA_PLAY_PAUSE, 0, win32con.KEYEVENTF_KEYUP, 0)
            elif comand[1] == 0:
                win32api.keybd_event(win32con.VK_MEDIA_NEXT_TRACK, 0, 0, 0)
                win32api.keybd_event(win32con.VK_MEDIA_NEXT_TRACK, 0, win32con.KEYEVENTF_KEYUP, 0)
            elif comand[1] == 2:
                win32api.keybd_event(win32con.VK_MEDIA_PREV_TRACK, 0, 0, 0)
                win32api.keybd_event(win32con.VK_MEDIA_PREV_TRACK, 0, win32con.KEYEVENTF_KEYUP, 0)

        else:
            self.__count_presed_button += 1
            if (self.__count_presed_button > 10):
                if comand[1] == 1:
                    pass
                elif comand[1] == 0:
                    pass
                elif comand[1] == 2:
                    pass

    def levelVolHandle(self, comand: str) -> None:
        """
        #обработчик команд из сериал порта и выставляющий нужый уровень громкости
        :param comand: строка с командой типа ''
        :return: NONE
        """
        comand = str(comand).split("|")
        if (len(comand) != 5):
            return
        self.audioSessions = AudioUtilities.GetAllSessions()
        for i in range(5):
            self.icon_mass[i].volume_level = int(int(comand[i])/10.24)
            if self.icon_mass[i].name == "":
                return
            if self.icon_mass[i].last_volume_level != self.icon_mass[i].volume_level:
                self.icon_mass[i].last_volume_level = self.icon_mass[i].volume_level
                if self.icon_mass[i].name == "master.exe":
                    devices = AudioUtilities.GetSpeakers()
                    interface = devices.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
                    volume = interface.QueryInterface(IAudioEndpointVolume)
                    volume.SetMasterVolumeLevel(self.dictVolumeDBtoProsent[self.icon_mass[i].volume_level], None)
                    continue
                for session in self.audioSessions:
                    if session.Process and session.Process.name() == self.icon_mass[i].name:
                        volume = session._ctl.QueryInterface(ISimpleAudioVolume)
                        volume.SetMasterVolume(float(self.icon_mass[i].volume_level) / 100, None)
                        # self.volLevelApp[num_app][1] = True
                        # break
                    elif (self.icon_mass[i].name == "system" and str(session)[
                        -5] == 'DisplayName: @%SystemRoot%\System32\AudioSrv.Dll'):
                        volume = session._ctl.QueryInterface(ISimpleAudioVolume)
                        volume.SetMasterVolume(float(self.icon_mass[i].volume_level) / 100, None)
                        # self.volLevelApp[num_app][1] = True

    def handleSignalLIght1(self):
        # self.timer_light.stop()
        # IcomReader.stopSocketVol(self.icon_mass)
        self.ser.writeSerial("SET_LIGHT:white")
    def handleSignalLIght2(self):
        # self.timer_light.stop()
        # IcomReader.stopSocketVol(self.icon_mass)
        self.ser.writeSerial("SET_LIGHT:wave")
    def handleSignalLIght3(self):
        # self.timer_light.start()
        self.ser.writeSerial("SET_LIGHT:level_value")
        self.valve_light = VaveLight(self.icon_mass, self.ser.writeSerial)
        self.valve_light.avtoUpdateStart()
        # IcomReader.startSocketVol(self.icon_mass)

    def startMassege(self):
        """
        #вызываеться послесигнала о подключении и запускает перврначальную настройку(записывает картинки в микщер)
        :return:
        """
        # self.ser_work = 1
        # self.timer_ser_con.stop()
        self.last_process_list = []
        # self.updateStyleUI()
        # self.loadIconOnESP()

    def loadIconOnESP(self, ans=0):
        self.num_load_icon = self.num_load_icon + ans
        self.valve_light.avtoUpdateStop()
        if (self.num_load_icon == 5):
            self.num_load_icon = 0
            self.valve_light.avtoUpdateStart()
            # self.mas_icon.clear()
            return
        self.ser.writeSerial("SET_ICON " + str(self.icon_mass[self.num_load_icon].num) + "\n")
        #self.timer_loadByte.start()


    def handleGetIcon(self):
        self.ser.writeByteSerial(self.icon_mass[self.num_load_icon].icon)
        # self.timer_loadByte.start()
    # def loadByteMasToESP(self):
    #     self.teat_perer += 1
    #     #/print(len(self.mas_icon[self.num_load_icon][0][(self.teat_perer - 1) * 64:self.teat_perer * 64]) , self.teat_perer, self.num_load_icon)
    #     self.ser.writeByteSerial(self.icon_mass[self.num_load_icon].icon[(self.teat_perer - 1) * 64:self.teat_perer * 64])
    #     if (self.teat_perer == 6):
    #         self.timer_loadByte.stop()
    #         self.teat_perer = 0
    #         self.num_load_icon += 1



if __name__ == '__main__':
    app = QApplication(sys.argv)
    # run_only_one_instance(app)
    app.setQuitOnLastWindowClosed(False)
    window = MainClass()
    sys.exit(app.exec())
