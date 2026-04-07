# 📌 PinOverlay

A Windows Explorer extension that adds:
- A **"📌 Pin"** item to the right-click context menu for files and folders
- A paperclip overlay icon on pinned items
- A **"📌 Unpin"** item for already pinned items

## How It Works

The extension consists of two parts:

**PinExtension.dll** (C#/SharpShell) — context menu:
- Adds "Pin" / "Unpin" to the right-click menu
- Prepends `📌` to the file or folder name — pinned items sort above others
- Stores pinned paths in `%APPDATA%\PinExtension\pinned.txt`

**PinOverlay.dll** (C++) — icon overlay:
- Reads the same `pinned.txt`
- Draws a paperclip badge in the top-right corner of pinned items' icons

## Requirements

- Windows 10/11 x64
- [.NET Framework 4.8](https://dotnet.microsoft.com/download/dotnet-framework/net48)
- [.NET SDK](https://dotnet.microsoft.com/download) — to build PinExtension
- [Visual Studio 2019/2022](https://visualstudio.microsoft.com/) with the **"Desktop development with C++"** workload — to build PinOverlay
- Administrator rights for installation

## Installation

### Step 1 — Build PinExtension (C#)

```bat
cd PinExtension
build.bat
```

Output: `PinExtension\bin\Release\PinExtension.dll`

### Step 2 — Build PinOverlay (C++)

Open a regular `cmd` window (not PowerShell) and run:

```bat
cd PinOverlay
build.bat
```

Output: `PinOverlay\PinOverlay.dll`

### Step 3 — Install PinExtension

Copy these files into a single folder:
```
📁 install_folder\
  ├── PinExtension.dll
  ├── SharpShell.dll
  ├── Apex.WinForms.dll
  ├── ServerManager.exe
  └── install.ps1
```

Download `ServerManager.exe` from the [SharpShell releases page](https://github.com/dwmkerr/sharpshell/releases).

Run PowerShell **as Administrator**:
```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\install.ps1
```

### Step 4 — Install PinOverlay

Run PowerShell **as Administrator** from the `PinOverlay\` folder:
```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\install.ps1
```

Or manually:
```bat
regsvr32 PinOverlay.dll
```

Explorer will restart automatically.

## Uninstalling

**PinExtension:**
```powershell
.\install.ps1 -Uninstall
```

**PinOverlay:**
```powershell
.\install.ps1 -Uninstall
```

Or manually:
```bat
regsvr32 /u PinOverlay.dll
```

## Project Structure

```
📁 PinExtension\
  ├── PinContextMenu.cs     — context menu logic
  ├── PinExtension.csproj   — project file
  ├── build.bat             — build script
  └── install.ps1           — install / uninstall

📁 PinOverlay\
  ├── PinOverlay.h          — header file
  ├── PinOverlay.cpp        — overlay implementation
  ├── PinOverlay.rc         — icon resource
  ├── PinOverlay.def        — DLL exports
  ├── pin_overlay.ico       — paperclip icon
  ├── build.bat             — build script
  └── install.ps1           — install / uninstall
```

## Notes

- On Windows 11 the context menu item appears under **"Show more options"**. To make the classic menu open by default:
  ```
  reg add "HKCU\Software\Classes\CLSID\{86ca1aa0-34aa-4e8b-a509-50c905bae2a2}\InprocServer32" /f /ve
  ```
- Windows supports a maximum of **15 overlay icons** system-wide. If the paperclip doesn't appear, check how many slots are taken:
  ```powershell
  Get-ChildItem "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers"
  ```
- If the overlay icon doesn't appear immediately, clear the icon cache:
  ```bat
  ie4uinit.exe -show
  ```
- Pinned paths are stored in `%APPDATA%\PinExtension\pinned.txt`