# import time
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QIODevice, pyqtSignal, QObject


class seriall(QObject):
    productIdentifier = 0
    vendorIdentifier = 0
    BaudRate = 0
    SignalReadPort = pyqtSignal(list)
    SignalSerialRegOk = pyqtSignal()
    SignalSerialStartOk = pyqtSignal()
    SignalError = pyqtSignal()
    SignalInvalidComand = pyqtSignal()
    SignalReadFinish = pyqtSignal(list)
    SignalReadLine = pyqtSignal(str)

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
        inputSrt = self.serial.readAll()
        print(inputSrt)
        try:
            inputSrt = str(inputSrt, 'utf-8')
            for char in inputSrt:
                self.mas_ser.append(char)
                if '\n' in self.mas_ser:
                    out_str = ''.join(self.mas_ser[:-1])
                    print(out_str)
                    self.SignalReadLine.emit(out_str)
                    if len(self.mas_iner) > 40:
                        self.flag_read_data = False
                        self.mas_iner.clear()
                    if 'REG_OK' in out_str:
                        self.SignalSerialRegOk.emit()
                        if len(self.mas_iner) > 0:
                            self.flag_read_data = False
                            self.SignalReadFinish.emit(self.mas_iner)

                            self.mas_iner.clear()
                    else:
                        if self.flag_read_data:
                            self.mas_iner.append(out_str)
                    self.mas_ser.clear()
        except:
            pass


    def closeSerial(self):                 #закрываем serial и стираем все состояния кнопок в приложении
        self.serial.close()

    def writeSerial(self, iner):
        self.flag_read_data = True
        self.serial.write(str(iner).encode())


