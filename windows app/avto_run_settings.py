import os
import sys
from PySide6.QtCore import QSettings
class AvtoRun():
    __RUN_PATH = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"

    @staticmethod
    def addAppToAvtoRun(name_app: str, path_to_app: str):
        settings = QSettings(AvtoRun.__RUN_PATH, QSettings.NativeFormat)
        settings.setValue(name_app, path_to_app)

    @staticmethod
    def removeAppToAvtoRun(name_app: str):
        settings = QSettings(AvtoRun.__RUN_PATH, QSettings.NativeFormat)
        settings.remove(name_app)

    @staticmethod
    def readAppToAvtoRun(name_app: str) -> bool:
        settings = QSettings(AvtoRun.__RUN_PATH, QSettings.NativeFormat)
        return settings.contains(name_app)

# class SettingsMyINI():
#     @staticmethod
#     def loadSett(key: list, irq_ms) -> dict:
#         with open(os.path.abspath(
#                 os.path.join(getattr(sys, '_MEIPASS', os.path.abspath(os.path.dirname(__file__))), 'ini.txt'))) as f:
#             lines = f.readlines()
#         for line in lines:
#
#         if len(lines) > 1:
#             if lines[0].find("PID=") != -1 and lines[1].find("VID=") != -1:
#                 return int(lines[0][4:-1]), int(lines[1][4:])
#             else:
#                 irq_ms("ERROR", "ERROR don't current PID/VID")
#                 # self.trayIcon.showMessage("ERROR", "ERROR don't current PID/VID")
#                 return 99999, 99999
#         else:
#             irq_ms("ERROR", "ERROR don't search ini.txt or current PID/VID")
#             # self.trayIcon.showMessage("ERROR", "ERROR don't search ini.txt or current PID/VID")
#             return 99999, 99999