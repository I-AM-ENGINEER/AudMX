# from __future__ import print_function
from pycaw.pycaw import AudioUtilities, ISimpleAudioVolume


from PIL import Image
import os

import threading
# def main():
#     sessions = AudioUtilities.GetAllSessions()
#     for session in sessions:
#         volume = session._ctl.QueryInterface(ISimpleAudioVolume)
#         if session.Process and session.Process.name() == "vlc.exe":
#             print("volume.GetMasterVolume(): %s" % volume.GetMasterVolume())
#             volume.SetMasterVolume(0.3, None)

from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PIL import Image
import os

import sys
from PyQt5.QtWidgets import QApplication










class MainClass():
    def __init__(self):
        pass







def bmp_to_byte_array(image_path):
    # Открываем изображение с помощью Pillow
    img = Image.open(image_path)

    # Проверяем, является ли изображение монохромным
    if img.mode != '1':
        raise ValueError("Изображение не является монохромным")

    # Получаем данные изображения в виде байтов
    img_bytes = img.tobytes()

    return img_bytes


def process_folder(folder_path):
    byte_arrays = []
    # Проходим по всем файлам в папке
    for filename in os.listdir(folder_path):
        # Проверяем, что файл имеет расширение .bmp
        if filename.endswith(".bmp"):
            # Получаем полный путь к файлу
            file_path = os.path.join(folder_path, filename)
            # Преобразуем изображение в байтовый массив и добавляем его в список
            byte_array = bmp_to_byte_array(file_path)
            byte_arrays.append(byte_array)
    return byte_arrays


# Путь к папке с изображениями
folder_path = ".\\icon"
# Обработка папки и получение байтовых массивов изображений
image_byte_arrays = process_folder(folder_path)

# print(image_byte_arrays[0])
#
# for i in image_byte_arrays[0]:
#     print(hex(i))




if __name__ == '__main__':
    app = QApplication(sys.argv)

    window = MainClass()
    # window.show()
    sys.exit(app.exec_())
