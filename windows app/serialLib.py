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

    def __init__(self):
        super().__init__()

        self.__serial = QSerialPort()
        self.__serial.errorOccurred.connect(self.__handleError)
        self.__flag_read_data = False
        self.__serial.readyRead.connect(self.readInpurAndOutput)
    @property
    def doesSerWork(self):
        return self.__ser_work

    def autoConnect(self,  vendorIdentifier: int, productIdentifier: int, baudRate: int, reconnect = False):
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
        print("ser read: ", self.__inputSrt[:self.__inputSrt.find("\n")])

        # temp = inputSrt[0:self.__inputSrt.find("\n")]
        # inputSrt = temp

        if self.__inputSrt.find("SET_BUTTON:") != -1:
            self.SignalReadButton.emit(self.__inputSrt[12])
        elif self.__inputSrt.find("|") != -1:
            if (self.__inputSrt != self.__comand_Buff):
                self.__comand_Buff = self.__inputSrt
                self.SignalReadVoluem.emit(self.__inputSrt)
        elif self.__inputSrt.find("OK") != -1:
            self.SignalSetIcon.emit(0)
        elif self.__inputSrt.find("ERROR: -1") != -1:
            self.SignalSetIcon.emit(-1)
        elif self.__inputSrt.find("Send 352 bytes") != -1:
            self.SignalGetIcon.emit()

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