import subprocess, signal
import socket
import threading
import time
import os
last_set_port = 13000
class SocketVolume():
    __pid = 0
    __dst_port = -1
    __call_back = None
    __count = 0.0
    __process = None
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


    def __init__(self, pid: int,num: int, call_back = None):
        '''
        :param call_back: adr on func call_back(float)
        :param pid: proccess id for volumpid.exe
        '''
        global last_set_port
        self.__dst_port = self.__setPort(last_set_port+1, 13400)
        if self.__dst_port == -1:
            return
        last_set_port = self.__dst_port
        print(self.__dst_port)

        self.__pid = pid
        self.__call_back = call_back

        self.__udp_thread = threading.Thread(target=self.__udp_client)
        self.__udp_thread.daemon = True
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

    def __is_port_free(self, port):

        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            # sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            try:
                sock.bind(("localhost", port))
                return True
            except OSError:
                return False


    def __setPort(self, start_port, stop_port):

        for port in range(start_port, stop_port):
            if self.__is_port_free(port):
                return port



        return -1
        # for i in range(start_port, stop_port):
        #     potoc = threading.Thread(target=self.__skanPort, args=(i, self.__dst_port))
        #     potoc.start()
        #     # self.__dst_port += 1
        #     if self.__dst_port != - 1:
        #         break

    def stop(self):
        # process = subprocess.Popen(command, shell=True)
        subprocess.Popen("TASKKILL /F /PID {pid} /T".format(pid=self.__process.pid))
        # self.__process.send_signal(signal.CTRL_C_EVENT)
        # p.kill()
    @staticmethod
    def killAllProcess():
        process = subprocess.Popen("TASKKILL /F /IM volumepid.exe", stdout=subprocess.PIPE)
        process.kill()
# class volumeVol():
#     def __int__(self):

# def dsd(str):
#     print(str)
#
# if __name__ == "__main__":
#     s = SocketVolume(420)
#     # while True:
#     print(s)
#     # s.stop()
#     # os.killpg(os.getpgid(pro.pid), signal.SIGTERM)
#     # time.sleep(1)
#     while True:
#         # time.sleep(30)
#         print(s)
#         if float(s) != 0.0:
#             s.stop()
#             break
