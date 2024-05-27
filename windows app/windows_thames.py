import winreg
import sys
import os
from PySide6.QtCore import QTimer
reg = winreg.ConnectRegistry(None, winreg.HKEY_CURRENT_USER)
reg_path = r'SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize'
class ThemesWindows():
    __reg = winreg.ConnectRegistry(None, winreg.HKEY_CURRENT_USER)
    __reg_path = r'SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize'

    @staticmethod
    def getStyle():

        try:
            reg_key = winreg.OpenKey(reg, reg_path)
        except FileNotFoundError:
            pass

        for i in range(1024):
            try:
                value_name, value, _ = winreg.EnumValue(reg_key, i)
                if value_name == 'SystemUsesLightTheme':

                    return value
            except:
                print('тема:' + str(value))
    @staticmethod
    def getCSSFile(theme: int):

        if theme == 0:
            bundle_dir = getattr(sys, '_MEIPASS', os.path.abspath(os.path.dirname(__file__)))
            path_to_help = os.path.join(bundle_dir, 'B_sylete')

            with open(path_to_help, "r+") as help_file:
                cssStyle = str(help_file.read())
                return cssStyle
        else:
            bundle_dir = getattr(sys, '_MEIPASS', os.path.abspath(os.path.dirname(__file__)))
            path_to_help = os.path.join(bundle_dir, 'W_sylete')

            with open(path_to_help, "r+") as help_file:
                return str(help_file.read())
    # def getTrue(self):
class AutoUpdateStile():
    __callback = None
    __old_theme = 0
    def __init__(self, callback, interval=3000):
        '''

        :param callback: metod update style
        :param interval:
        '''
        self.__timer = QTimer()
        self.__timer.timeout.connect(self.__upDateStyle())
        self.__timer.setInterval(interval)
        self.__callback = callback
        callback(ThemesWindows.getCSSFile(self.__old_theme))

    def __upDateStyle(self):
        if self.__changeThemes:
            self.__callback(ThemesWindows.getCSSFile(self.__old_theme))
    def __changeThemes(self):
        if self.__old_theme != ThemesWindows.getStyle():
            self.__old_theme = not self.__old_theme
            return True
        return False



def getStyleBW(lock_theme = ''):
    global reg
    global reg_path
    try:
        reg_key = winreg.OpenKey(reg, reg_path)
    except FileNotFoundError:
        pass

    for i in range(1024):
        try:
            value_name, value, _ = winreg.EnumValue(reg_key, i)

            if lock_theme == 'White':
                value = 1
            elif lock_theme == 'Black':
                value = 0
            if value_name == 'SystemUsesLightTheme':
                if value == 0:
                    bundle_dir = getattr(sys, '_MEIPASS', os.path.abspath(os.path.dirname(__file__)))
                    path_to_help = os.path.abspath(os.path.join(bundle_dir, 'B_sylete'))

                    with open(path_to_help, "r+") as help_file:
                        cssStyle = str(help_file.read())
                        return cssStyle, value
                else:
                    bundle_dir = getattr(sys, '_MEIPASS', os.path.abspath(os.path.dirname(__file__)))
                    path_to_help = os.path.abspath(os.path.join(bundle_dir, 'W_sylete'))

                    with open(path_to_help, "r+") as help_file:
                        cssStyle = str(help_file.read())
                        return cssStyle, value
        except:
            print('тема:' + str(value))



# class SettingsAp():
