# AudMix GUI
Приложение для работы с микшером грамкости. Данное приложение запускаеться в трее

## Содержание
- [Использование](#использование)
- [Работа_с_исходным_кодом](#Работа_с_исходным_кодом)
- [файлы](#файлы)

## Использование
Для использования данных програм вам 
## Работа_с_исходным_кодом
Для работы с исходным кодом нужен нужен интерпретатор с библиотеками **PySide6**, **pywin32**, **Pillow**, **ctype**, **pycaw**. 
Для сборки вам потребуется **PyInstaller** и **Python** со всеми установленными библиотеками через **PIP**

 `pyinstaller --hidden-import pycaw  --hidden-import Pillow  --hidden-import ctype  --hidden-import pywin32 --hidden-import PySide6 --onefile -w --add-data="B_sylete;." --add-data="W_sylete;." main.py`

## файлы
- B_sylete/W_sylete - файлы с темами для приложение в формате css 
- [serialLib.py](https://github.com/AOProton/test_stand_30_01_24/blob/main/serialLib.py) - библиотека для работы с сериал портом
- [main.py](https://github.com/AOProton/test_stand_30_01_24/blob/main/main.py) - файл который связывает все калбеки и инициализирует все, а также содежит фунция для работы с скейлингом монитора 
- [ui.py](https://github.com/AOProton/test_stand_30_01_24/blob/main/ui.py) - файл знимающийся отрисовкой интерфейса
- [pyinstaller.txt](https://github.com/AOProton/test_stand_30_01_24/blob/main/pyinstaller.txt) - файл с командой для сборки проекта
 
## Команда проекта
 - ушла :/