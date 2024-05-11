
from PySide6.QtCore import QSettings
RUN_PATH = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"

def addAppToAvtoRun(name_app: str, path_to_app: str):
    settings = QSettings(RUN_PATH, QSettings.NativeFormat)
    settings.setValue(name_app, path_to_app)
def removeAppToAvtoRun(name_app: str):
    settings = QSettings(RUN_PATH, QSettings.NativeFormat)
    settings.remove(name_app)
def readAppToAvtoRun(name_app: str) -> bool:
    settings = QSettings(RUN_PATH, QSettings.NativeFormat)
    return settings.contains(name_app)