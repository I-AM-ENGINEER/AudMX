# import time
from PySide6.QtSerialPort import QSerialPort, QSerialPortInfo
from PySide6.QtCore import QIODevice, Signal, QObject

# PC->ESP
# SET_ICON:posision:icon     -> num=(int[0:4]) icon=(bite arrey)


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
        global vendorIdentifier
        global productIdentifier
        global BaudRate
        BaudRate = BaudRate_
        vendorIdentifier = vendorIdentifier_
        productIdentifier = productIdentifier_
        self.serial = QSerialPort()
        # self.circular_array = CircularArray.CircularArray(400)
        self.mas_ser = []
        self.mas_iner = []
        self.flag_read_data = False

    def readPort(self):                            #запускаем сериал и счтываем порты(записываем в comboBox)
        ports = QSerialPortInfo().availablePorts()
        # ports = [port.portName() for port in ports]
        self.SignalReadPort.emit(ports)
        return ports
    def startSerialAutoConnect(self):                            #запускаем сериал и авто коннект
        portList = self.readPort()
        for port in portList:
            if port.productIdentifier() == productIdentifier and port.vendorIdentifier() == vendorIdentifier:
                self.openSerialAndStartMessage(port)

    def openSerialAndStartMessage(self, currertPort, BaudRate):                   #начинаем общение с сериалом
        print(currertPort, BaudRate)
        self.serial.setBaudRate(BaudRate)
        self.serial.setPortName(currertPort)
        self.serial.open(QIODevice.ReadWrite)
        self.serial.readyRead.connect(self.readInpurAndOutput)
        self.SignalSerialStartOk.emit()

    def readInpurAndOutput(self):                        #оброботчик всех приходящих пакетов
        inputSrt = self.serial.readLine()
        print(inputSrt)
        inputSrt = str(inputSrt, 'utf-8')
        if inputSrt.find("SET_BUTTON:") != -1:
            self.SignalReadButton.emit(inputSrt[12])
        elif inputSrt.find("SET_VALUE") != -1:
            self.SignalReadVoluem.emit(inputSrt[12])

    def closeSerial(self):                 #закрываем serial и стираем все состояния кнопок в приложении
        self.serial.close()

    def writeSerial(self, iner):
        self.flag_read_data = True
        self.serial.write(str(iner).encode())
