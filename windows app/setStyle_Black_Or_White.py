import winreg
import sys
import os
reg = winreg.ConnectRegistry(None, winreg.HKEY_CURRENT_USER)
reg_path = r'SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize'

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
