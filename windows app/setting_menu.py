import sys
from PySide6.QtWidgets import QCheckBox, QLabel, QPushButton, QComboBox, QDialog, QGridLayout
from PySide6.QtCore import QSettings

class MenuSettings(QDialog):
    __set_tray = ""
    __app_name = ""

    def __init__(self, set_tray: str, app_name: str, parent=None):
        super().__init__(parent)
        self.__set_tray = set_tray
        self.__app_name = app_name

        self.__check_box_1 = QCheckBox()
        self.__check_box_1_lb = QLabel("Авто старт")
        self.__set_theme_1 = QComboBox()
        self.__set_theme_1.addItems(["системная", "светлая", "темная"])
        self.__set_theme_1_lb = QLabel("тема")

        self.__button = QPushButton("Сохранить")
        self.__layout = QGridLayout()
        self.__layout.addWidget(self.__check_box_1, 1, 0, 1, 1)
        self.__layout.addWidget(self.__check_box_1_lb, 2, 0, 1, 1)
        self.__layout.addWidget(self.__set_theme_1, 2, 0, 1, 1)
        self.__layout.addWidget(self.__set_theme_1_lb, 2, 1, 1, 1)
        self.__layout.addWidget(self.__button, 3, 0, 1, 2)
        self.setLayout(self.__layout)

        self.__set_theme_1.setCurrentText(QSettings().value(self.__set_tray + "/theme", False, type=str))
        self.__check_box_1.setChecked(AvtoRunStatic.readAppToAvtoRun(self.__app_name))

        if self.exec_() == QDialog.DialogCode.Accepted:
            self.__safeSettings()
        self.deleteLater()

    def __safeSettingsApp(self):
        settings = QSettings()
        settings.setValue(self.__set_tray + "/theme", self.set_theme_1.currentText())
        settings.sync()
        if (self.__check_box_1.isChecked() == 0):
            AvtoRunStatic.removeAppToAvtoRun(self.__app_name)
        else:
            AvtoRunStatic.addAppToAvtoRun(self.__app_name, sys.argv[0])

    def readThemeMode(self) -> str:
        return QSettings().settings.value(self.__set_tray + "/theme", False, type=str)


class AvtoRunStatic():
    __RUN_PATH = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"

    @staticmethod
    def addAppToAvtoRun(name_app: str, path_to_app: str):
        settings = QSettings(AvtoRunStatic.__RUN_PATH, QSettings.NativeFormat)
        settings.setValue(name_app, path_to_app)

    @staticmethod
    def removeAppToAvtoRun(name_app: str):
        settings = QSettings(AvtoRunStatic.__RUN_PATH, QSettings.NativeFormat)
        settings.remove(name_app)

    @staticmethod
    def readAppToAvtoRun(name_app: str) -> bool:
        settings = QSettings(AvtoRunStatic.__RUN_PATH, QSettings.NativeFormat)
        return settings.contains(name_app)


from PyQt5.QtGui import QWindow
from PyQt5.QtWidgets import QApplication

class Monitor():
    @staticmethod
    def getDictMonitor() -> dict:
        app = QApplication.instance()  # Получаем экземпляр приложения
        if app is None:
            app = QApplication([])  # Создаем приложение, если его нет

        dict_monitor = {}

        for screen in app.screens():
            dpi = screen.logicalDotsPerInch()
            scale_factor = dpi / 96.0
            dict_monitor[screen.name()] = scale_factor
        return dict_monitor
    def temp(self, parent):
        parent.windowHandle().screenChanged.connect(lambda screen: parent.editSize(screen.name()))

