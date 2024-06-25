# import time
from PySide6.QtSerialPort import QSerialPort, QSerialPortInfo
from PySide6.QtCore import QIODevice, Signal, QObject, QTimer

class seriall(QObject):
    SignalReadPort = Signal(list)
    SignalSerialRegOk = Signal()
    SignalSerialStartOk = Signal()
    SignalError = Signal(str)
    SignalInvalidComand = Signal()
    SignalReadFinish = Signal(list)
    SignalReadLine = Signal(str)
    SignalReadButton = Signal(str)
    SignalReadVoluem = Signal(str)
    SignalSetIcon = Signal(int)
    SignalGetIcon = Signal()
    __productIdentifier = 0
    __vendorIdentifier = 0
    __BaudRate = 0
    __comand_Buff = ""
    __flag_do_read = 0
    __inputSrt = ""
    __flag_reconnect = False
    __ser_work = False
    __handleRead = None

    def __init__(self):
        super().__init__()

        self.__serial = QSerialPort()
        self.__serial.errorOccurred.connect(self.__handleError)
        self.__flag_read_data = False
        self.__serial.readyRead.connect(self.readInpurAndOutput)
    @property
    def doesSerWork(self):
        return self.__ser_work
    def setHanglerRead(self, hang: object):
        self.__handleRead = hang

    def autoConnect(self,  vendorIdentifier: int, productIdentifier: int, baudRate: int, reconnect=False):
        self.__BaudRate = baudRate
        self.__vendorIdentifier = vendorIdentifier
        self.__productIdentifier = productIdentifier
        self.__timer_avto_connect = QTimer()
        self.__timer_avto_connect.timeout.connect(self.__startSerialAutoConnect)
        self.__timer_avto_connect.setInterval(1500)
        self.__timer_avto_connect.start()
        self.__flag_reconnect = reconnect
    def __handleError(self, error):
        if error == QSerialPort.NoError:
            return
        if error == QSerialPort.ResourceError:
            self.SignalError.emit('disconnected')
            print("Serial port disconnected!")
            self.closeSerial()
            __ser_work = False
            if (self.__flag_reconnect == True):
                self.__timer_avto_connect.start()


    def readPort(self):
        """
        #
        :return: ports список всех работующих сериал портов
        """
        ports = QSerialPortInfo().availablePorts()
        ports_name = [port.portName() for port in ports]
        self.SignalReadPort.emit(ports_name)
        return ports

    def __startSerialAutoConnect(self):
        """
        #авто запуск сериал порта
        берет список всех пртов и если находить устройство с правильным PID/VID запускает подключение
        :return: None
        """
        portList = self.readPort()
        for port in portList:
            if port.productIdentifier() == self.__productIdentifier and port.vendorIdentifier() == self.__vendorIdentifier:
                self.openSerialAndStartMessage(port.portName(), self.__BaudRate)

    def openSerialAndStartMessage(self, currertPort: str, BaudRate: int):
        """
        #инициализирует сериал порт и подключает обработчик входящих команд
        по завершению вызывает сигнал SignalSerialStartOk
        :return: None
        """
        print(currertPort, BaudRate)
        self.__serial.setBaudRate(BaudRate)
        self.__serial.setPortName(currertPort)
        self.__serial.open(QIODevice.ReadWrite)
        self.__timer_avto_connect.stop()
        self.__ser_work = True

        self.SignalSerialStartOk.emit()



    def readInpurAndOutput(self):
        """
        #оброботчик всех приходящих пакетов
        при командах вызывает соответствующие сигналы
        :return: None
        """
        # inputSrt = self.serial.readLine()
        inputSrtB = self.__serial.readAll()


        if (self.__flag_do_read == 1):
            self.__inputSrt += str(inputSrtB, 'utf-8')
        else:
            self.__inputSrt = str(inputSrtB, 'utf-8')

        if (self.__inputSrt.find("\n") == -1):
            print("------------------------", len(self.__inputSrt), self.__inputSrt.find("\n"))
            self.__flag_do_read = 1
            return
        else:
            self.__flag_do_read = 0
        self.__inputSrt = self.__inputSrt[:self.__inputSrt.find("\n")]
        print("ser read: ", self.__inputSrt)

        # temp = inputSrt[0:self.__inputSrt.find("\n")]
        # inputSrt = temp
        try:
            self.__handleRead(self.__inputSrt)
        except:
            print("NO SET HANDLER!!!!!!!!")

        # if self.__inputSrt.find("BUTTON:") != -1:
        #     self.SignalReadButton.emit(self.__inputSrt)
        # elif self.__inputSrt.find("|") != -1:
        #     if (self.__inputSrt != self.__comand_Buff):
        #         self.__comand_Buff = self.__inputSrt
        #         self.SignalReadVoluem.emit(self.__inputSrt)
        # elif self.__inputSrt.find("OK") != -1:
        #     self.SignalSetIcon.emit(0)
        # elif self.__inputSrt.find("ERROR: -1") != -1:
        #     self.SignalSetIcon.emit(-1)
        # elif self.__inputSrt.find("Send 352 bytes") != -1:
        #     self.SignalGetIcon.emit()

    def closeSerial(self):
        self.__serial.close()

    def writeSerial(self, iner: str):
        """
        функция для записи команд в сериал порт
        :param iner: команда для отправки в сериал порт
        :return: None
        """
        self.__flag_read_data = True
        print("ser write:", iner)
        self.__serial.write(str(iner).encode())

    def writeByteSerial(self, iner):
        self.__flag_read_data = True
        print("ser write Byte: ", iner)
        self.__serial.write(bytes(iner))


class SerCDC(seriall):
    __flag_cdc = False
    __quwewe_write = []
    __last_write = None
    __count_packeg = 0
    __num_packeg = 0
    def __init__(self, On_CDC: bool):
        super().__init__()
        self.__flag_cdc = On_CDC
        self.__timer_cdc = QTimer()
        self.__timer_cdc.timeout.connect(self.__load64Byte)
        self.__timer_cdc.setInterval(10)
    @property
    def mod_CDC(self):
        return self.__flag_cdc
    @mod_CDC.setter
    def mod_CDC(self, mod: bool):
        self.__flag_cdc = mod

    def writeSerial(self, iner: str):
        if self.__flag_cdc == False:
            super().writeSerial(iner)
        else:

            self.__quwewe_write.append([iner , True])
            # self.__last_write = iner
            # self.__count_packeg = (len(iner) / 64) + 1
            # self.__load64Byte()
            self.__timer_cdc.start()
    def writeByteSerial(self, iner: bytes):
        if self.__flag_cdc == False:
            super().writeByteSerial(iner)
        self.__quwewe_write.append([iner, False])
        # self.__last_write = iner
        # self.__count_packeg = (len(iner) / 64) + 1
        # self.__load64Byte()
        self.__timer_cdc.start()

        # self.__serial.write(bytes(iner))
    def clearnQuwewe(self):
        self.__quwewe_write.clear()

    def clearnSend(self):
        self.__num_packeg = 99
        self.__last_write = 0



    def __load64Byte(self):
        if self.__num_packeg < self.__count_packeg:
            if self.__last_write[1] == True:
                super().writeSerial(self.__last_write[0][self.__num_packeg * 64:(self.__num_packeg + 1) * 64])
            else:
                super().writeByteSerial(self.__last_write[0][self.__num_packeg * 64:(self.__num_packeg + 1) * 64])
            self.__num_packeg += 1
        else:
            self.__num_packeg = 0
            self.__count_packeg = 0
            self.__last_write = 0
            if len(self.__quwewe_write) != 0:
                self.__last_write = self.__quwewe_write.pop()
                self.__count_packeg = int((len(self.__last_write[0]) / 64)) + 1
                if self.__count_packeg == 1:
                    self.__load64Byte()
                else:
                    self.__timer_cdc.start()
            else:
                self.__timer_cdc.stop()


