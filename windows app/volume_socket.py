import subprocess
import socket

import threading
class SocketVolume():
    __pid = 0
    __dst_port = 13200
    __call_back = None
    __count = 0.0
    def __float__(self):
        if (self.__call_back == None):
            return self.__count
        else:
            return -1.0
    def __str__(self):
        if (self.__call_back == None):
            return str(self.__count)
        else:
            return "set callback"


    def __init__(self, pid: int, call_back = None):
        '''
        :param call_back: adr on func call_back(float)
        :param pid: proccess id for volumpid.exe
        '''
        self.__setPort(13000, 13400)

        self.__pid = pid
        self.__call_back = call_back

        self.__udp_thread = threading.Thread(target=self.__udp_client)
        self.__udp_thread.daemon = True  # Делаем поток демоном, чтобы он завершался при завершении основного потока
        self.__udp_thread.start()

    def __udp_client(self):
        command = [".\\volumepid.exe", str(self.__pid), str(self.__dst_port)]
        self.__process = subprocess.Popen(command, stdout=subprocess.PIPE)
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        # Привязываем сокет к указанному хосту и порту
        server_address = ("localhost", self.__dst_port)
        sock.bind(server_address)

        try:
            if (self.__call_back == None):
                while True:
                    # Принимаем данные
                    data, address = sock.recvfrom(4096)
                    self.__count = float(data.decode())
            else:
                while True:
                    # Принимаем данные
                    data, address = sock.recvfrom(4096)
                    self.__call_back(float(data.decode()))
                # print(f"Received message from {address}: {data.decode()}")
        except KeyboardInterrupt:
            print("Client stopped by user")
        finally:
            # Закрываем сокет
            sock.close()
    def __skanPort(self, port, _dst_port):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(0.5)
        try:
            connect = sock.connect(("localhost", port))
            _dst_port = port
            sock.close()
        except:


            pass
        # self.potoc.join()

    def __setPort(self, start_port, stop_port):
        for i in range(start_port, stop_port):
            potoc = threading.Thread(target=self.__skanPort, args=(i, self.__dst_port))
            potoc.start()
            # self.__dst_port += 1
            if self.__dst_port != - 1:
                break

    def stop(self):
        self.__process.kill()
    @staticmethod
    def killAllProcess():
        process = subprocess.Popen("taskkill /F /IM volumepid.exe", stdout=subprocess.PIPE)
        process.kill()

def dsd(str):
    print(str)

if __name__ == "__main__":
    s = SocketVolume(13200)

    while True:
        print(s)
