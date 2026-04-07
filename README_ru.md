# 📌 PinOverlay

Расширение для проводника Windows, которое добавляет:
- Пункт **«📌 Закрепить»** в контекстное меню файлов и папок
- Значок скрепки поверх иконки закреплённых элементов
- Пункт **«📌 Открепить»** для уже закреплённых элементов

## Как это работает

Расширение состоит из двух частей:

**PinExtension.dll** (C#/SharpShell) — контекстное меню:
- Добавляет пункт «Закрепить» / «Открепить» при правом клике
- Добавляет префикс `📌` к имени файла или папки — они сортируются выше остальных
- Сохраняет пути закреплённых элементов в `%APPDATA%\PinExtension\pinned.txt`

**PinOverlay.dll** (C++) — иконка-оверлей:
- Читает тот же `pinned.txt`
- Рисует значок скрепки в правом верхнем углу иконки закреплённых элементов

## Требования

- Windows 10/11 x64
- [.NET Framework 4.8](https://dotnet.microsoft.com/download/dotnet-framework/net48)
- [.NET SDK](https://dotnet.microsoft.com/download) — для сборки PinExtension
- [Visual Studio 2019/2022](https://visualstudio.microsoft.com/) с компонентом **«Desktop development with C++»** — для сборки PinOverlay
- Права Администратора для установки

## Установка

### Шаг 1 — Сборка PinExtension (C#)

```bat
cd PinExtension
```
```
build.bat
```

Готовая DLL: `PinExtension\bin\Release\PinExtension.dll`

### Шаг 2 — Сборка PinOverlay (C++)

Открой обычный `cmd` (не PowerShell) и выполни:

```bat
cd PinOverlay
```
```
build.bat
```

Готовая DLL: `PinOverlay\PinOverlay.dll`

### Шаг 3 — Установка PinExtension

Скопируй в одну папку:
```
📁 install_folder\
  ├── PinExtension.dll
  ├── SharpShell.dll
  ├── Apex.WinForms.dll
  ├── ServerManager.exe
  └── install.ps1
```

`ServerManager.exe` скачать с [релизов SharpShell](https://github.com/dwmkerr/sharpshell/releases).

Запусти PowerShell **от имени Администратора**:
```powershell
Set-ExecutionPolicy -Scope Process Bypass
```
```
.\install.ps1
```

Либо вручную через реестр — смотри `install.ps1`.

### Шаг 4 — Установка PinOverlay

PowerShell **от имени Администратора** из папки `PinOverlay\`:
```powershell
Set-ExecutionPolicy -Scope Process Bypass
```
```
.\install.ps1
```

Или вручную:
```bat
regsvr32 PinOverlay.dll
```

Проводник перезапустится автоматически.

## Удаление

**PinExtension:**
```powershell
.\install.ps1 -Uninstall
```

**PinOverlay:**
```powershell
.\install.ps1 -Uninstall
```

Или вручную:
```bat
regsvr32 /u PinOverlay.dll
```

## Структура проекта

```
📁 PinExtension\
  ├── PinContextMenu.cs     — логика контекстного меню
  ├── PinExtension.csproj   — файл проекта
  ├── build.bat             — сборка
  └── install.ps1           — установка / удаление

📁 PinOverlay\
  ├── PinOverlay.h          — заголовочный файл
  ├── PinOverlay.cpp        — реализация оверлея
  ├── PinOverlay.rc         — ресурс иконки
  ├── PinOverlay.def        — экспорты DLL
  ├── pin_overlay.ico       — иконка скрепки
  ├── build.bat             — сборка
  └── install.ps1           — установка / удаление
```

## Примечания

- На Windows 11 пункт контекстного меню появляется в **«Показать дополнительные параметры»**. Чтобы старое меню открывалось сразу:
  ```
  reg add "HKCU\Software\Classes\CLSID\{86ca1aa0-34aa-4e8b-a509-50c905bae2a2}\InprocServer32" /f /ve
  ```
- Windows ограничивает количество оверлей-иконок до **15**. Если значок не появляется — проверь сколько слотов занято:
  ```powershell
  Get-ChildItem "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers"
  ```
- Если иконка не появилась сразу — очисти кэш иконок:
  ```bat
  ie4uinit.exe -show
  ```
- Данные хранятся в `%APPDATA%\PinExtension\pinned.txt`
