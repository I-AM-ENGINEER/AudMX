# -*- mode: python ; coding: utf-8 -*-


a = Analysis(
    ['AudMX.py'],
    pathex=[],
    binaries=[],
    datas=[('B_sylete', '.'), ('W_sylete', '.'), ('icon.png', '.'), ('ini.txt', '.')],
    hiddenimports=['pycaw', 'Pillow', 'ctype', 'pywin32', 'PySide6'],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='AudMX',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon=['C:\\Users\\savva\\Documents\\github\\AudMix\\icon.ico'],
)
