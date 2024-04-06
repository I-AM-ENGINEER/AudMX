# import time
from PySide6.QtSerialPort import QSerialPort, QSerialPortInfo
from PySide6.QtCore import QIODevice, Signal, QObject

# PC->ESP
# SET_ICON:posision:icon     -> num=(int[0:4]) icon=(bite arrey)
# SET_LIGHT:name_mode     -> name_mode=(str(white/wave/level_value))

# PC<-ESP
# SET_VALUE:num:value   -> num=(int[0:4]) value=(int [0:100])
# SET_BUTTON:str   -> str=(prev/next/play)


class seriall(QObject):
    productIdentifier = 0
    vendorIdentifier = 0
    BaudRate = 0
    SignalReadPort = Signal(list)
    SignalSerialRegOk = Signal()
    SignalSerialStartOk = Signal()
    SignalError = Signal()
    SignalInvalidComand = Signal()
    SignalReadFinish = Signal(list)
    SignalReadLine = Signal(str)
    SignalReadButton = Signal(str)
    SignalReadVoluem = Signal(str)

    def __init__(self, vendorIdentifier_ = 0, productIdentifier_ = 0, BaudRate_ = 0):
        super().__init__()
        self.BaudRate = BaudRate_
        self.vendorIdentifier = vendorIdentifier_
        self.productIdentifier = productIdentifier_
        self.serial = QSerialPort()
        # self.circular_array = CircularArray.CircularArray(400)
        self.mas_ser = []
        self.mas_iner = []
        self.flag_read_data = False

    def readPort(self):
        """
        #запускаем сериал и счтываем порты(записываем в comboBox)
        :return: ports список всех работующих сериал портов
        """
        ports = QSerialPortInfo().availablePorts()
        # ports = [port.portName() for port in ports]
        self.SignalReadPort.emit(ports)
        return ports
    def startSerialAutoConnect(self):
        """
        #авто запуск сериал порта
        берет список всех пртов и если находить устройство с правильным PID/VID запускает подключение
        :return: None
        """
        portList = self.readPort()
        for port in portList:
            if port.productIdentifier() == self.productIdentifier and port.vendorIdentifier() == self.vendorIdentifier:
                self.openSerialAndStartMessage(port, self.BaudRate)

    def openSerialAndStartMessage(self, currertPort: str, BaudRate: int):                   #начинаем общение с сериалом
        """
        #инициализирует сериал порт и подключает обработчик входящих команд
        по завершению вызывает сигнал SignalSerialStartOk
        :return: None
        """
        print(currertPort, BaudRate)
        self.serial.setBaudRate(BaudRate)
        self.serial.setPortName(currertPort)
        self.serial.open(QIODevice.ReadWrite)
        self.serial.readyRead.connect(self.readInpurAndOutput)
        self.SignalSerialStartOk.emit()

    def readInpurAndOutput(self):
        """
        #оброботчик всех приходящих пакетов
        при командах вызывает соответствующие сигналы
        :return: None
        """
        inputSrt = self.serial.readLine()
        print(inputSrt)
        inputSrt = str(inputSrt, 'utf-8')
        if inputSrt.find("SET_BUTTON:") != -1:
            self.SignalReadButton.emit(inputSrt[12])
        elif inputSrt.find("SET_VALUE") != -1:
            self.SignalReadVoluem.emit(inputSrt[12])

    def closeSerial(self):                 #закрываем serial и стираем все состояния кнопок в приложении
        self.serial.close()

    def writeSerial(self, iner: str):
        """
        функция для записи команд в сериал порт
        :param iner: команда для отправки в сериал порт
        :return: None
        """
        self.flag_read_data = True
        self.serial.write(str(iner).encode())
