# Activate Windows Watermark Overlay

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010%20%7C%2011-0078D6.svg)]()
[![Binary Size](https://img.shields.io/badge/Binary%20Size-44%20KB-brightgreen.svg)]()
[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)]()
[![DPI Awareness](https://img.shields.io/badge/DPI-Per--Monitor%20V2-orange.svg)]()

A lightweight, zero-dependency, pixel-perfect native Win32/GDI+ desktop overlay that restores the authentic Windows activation watermark on genuine, activated Windows installations.

---

## Overview

Modern Windows machines often come pre-activated with OEM digital licenses. For developers, streamers, and enthusiasts who find genuine desktops oddly unfamiliar, **Activate Windows Watermark Overlay** accurately renders the native desktop activation watermark with mathematical precision and zero system impact.

The application operates entirely in user space without modifying system files, altering licensing registries, or degrading Windows personalization capabilities.

---

## Key Features

- **Pixel-Perfect Authenticity**: Uses the native `Segoe UI` system font family with 1:1 typographic sizing, letter-spacing, and anti-aliased subpixel rendering matching genuine Desktop Window Manager (DWM) output.
- **Zero Input Interruption (Click-Through)**: Leverages `WS_EX_TRANSPARENT` and `WS_EX_LAYERED` window styles. All mouse clicks, gestures, drag-and-drop operations, and full-screen games pass through without latency or input drops.
- **Per-Monitor V2 DPI Scaling**: Dynamically adjusts font metrics and layout margins across mixed-DPI multi-monitor environments (100%, 125%, 150%, 200%).
- **Multi-Monitor Architecture**: Supports single primary monitor anchoring or simultaneous rendering across all active displays.
- **Stealth Integration**: Bypasses the Windows Taskbar and Alt+Tab application switcher (`WS_EX_TOOLWINDOW`).
- **Emergency Panic Hotkey**: Press `Ctrl + Alt + W` to toggle visibility instantly.
- **Native Footprint**: Written in pure Win32 C++ with zero external dependencies. The compiled binary is under 45 KB and consumes less than 3 MB of RAM.

---

## Technical Architecture

The watermark is drawn using an off-screen 32-bit Device Independent Bitmap (DIB) with pre-multiplied alpha values submitted directly to the Desktop Window Manager via `UpdateLayeredWindow`:

```
+--------------------------------------------------------------------+
|                         Application Core                           |
+--------------------------------------------------------------------+
                                |
                                v
               +----------------------------------+
               |  Per-Monitor V2 DPI Scaling      |
               +----------------------------------+
                                |
                                v
               +----------------------------------+
               |  GDI+ Text Measurement & Layout  |
               +----------------------------------+
                                |
                                v
               +----------------------------------+
               |  32-bpp ARGB Memory DIB Section  |
               +----------------------------------+
                                |
                                v
               +----------------------------------+
               |  In-Place Alpha Pre-multiplication|
               +----------------------------------+
                                |
                                v
+--------------------------------------------------------------------+
|            Win32 DWM Pipeline (UpdateLayeredWindow)                |
|      [ WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST ]         |
+--------------------------------------------------------------------+
```

### Pre-multiplied Alpha Calculation

To prevent dark fringe artifacts during desktop composition, RGB channels are normalized against per-pixel alpha:

$$R_{\text{out}} = \left\lfloor \frac{R_{\text{in}} \times A}{255} \right\rfloor, \quad G_{\text{out}} = \left\lfloor \frac{G_{\text{in}} \times A}{255} \right\rfloor, \quad B_{\text{out}} = \left\lfloor \frac{B_{\text{in}} \times A}{255} \right\rfloor$$

---

## Installation

### Pre-built Binaries

Download the latest standalone executable from the [Releases](https://github.com/mocn203/activate-windows-watermark/releases) section:

1. Download `activate-windows-v1.0.0-win-x64.zip`.
2. Extract the archive.
3. Run `activate-windows.exe`.

No installation or administrative privileges required.

---

## Usage

### Graphical Controls

- **Toggle Visibility**: Double-click the notification tray icon or press `Ctrl + Alt + W`.
- **Context Menu**: Right-click the system tray icon to access:
  - Watermark Presets
  - Opacity levels (30%, 45%, 60%, 80%, 100%)
  - Screen positioning (Bottom-Right, Bottom-Left, Top-Right, Top-Left)
  - Display routing (Primary monitor only / All displays)
  - Custom text configuration

### Command-Line Interface (CLI)

The application supports automated startup via command-line arguments:

```cmd
activate-windows.exe [OPTIONS]
```

#### Options

| Option | Argument | Description |
| :--- | :--- | :--- |
| `--preset` | `<1-5>` | Select predefined text template |
| `--title` | `"<string>"` | Set custom primary text |
| `--subtitle` | `"<string>"` | Set custom secondary text |
| `--opacity` | `<1-100>` | Set watermark opacity percentage |
| `--all-monitors` | | Enable rendering across all detected displays |
| `--silent` | | Run in headless mode without a system tray icon |

#### Examples

```cmd
:: Launch with default activation watermark
activate-windows.exe

:: Run silently on all monitors
activate-windows.exe --all-monitors --silent

:: Set custom text and high opacity
activate-windows.exe --title "Evaluation Copy" --subtitle "Build 26100.ge_release" --opacity 80
```

---

## Presets

The application includes several built-in text profiles:

1. **Standard Activation (Default)**
   - Line 1: `Activate Windows`
   - Line 2: `Go to Settings to activate Windows.`
2. **Windows 11 Enterprise Evaluation**
   - Line 1: `Windows 11 Enterprise`
   - Line 2: `Evaluation copy. Build 26100.ge_release`
3. **Copilot Subscription**
   - Line 1: `Activate Copilot`
   - Line 2: `Go to Settings to buy Copilot Pro.`
4. **Linux Migration**
   - Line 1: `Activate Linux`
   - Line 2: `Go to Settings to install Arch Linux btw.`
5. **Wellness Reminder**
   - Line 1: `Activate Real Life`
   - Line 2: `Go outside and touch some grass.`
6. **Custom Definition**
   - User-defined primary and secondary strings via UI or CLI.

---

## Building from Source

### Prerequisites

- A standard C++17 compiler (MinGW-w64 GCC or MSVC)
- Windows SDK (GDI+, User32, Gdi32, Shell32, Ole32)

### Building with MinGW-w64 (GCC)

```cmd
g++ -O2 -s -mwindows -municode main.cpp -o activate-windows.exe -lgdiplus -lgdi32 -luser32 -lshell32 -lcomctl32 -lole32
```

Alternatively, run the provided build script:

```cmd
build.bat
```

### Building with Microsoft Visual C++ (MSVC)

```cmd
cl /O2 /EHsc /DUNICODE /D_UNICODE main.cpp /link /SUBSYSTEM:WINDOWS gdiplus.lib gdi32.lib user32.lib shell32.lib comctl32.lib ole32.lib /OUT:activate-windows.exe
```

---

## Performance & Resource Utilization

| Metric | Measured Value |
| :--- | :--- |
| **Binary Size** | ~44 KB (stripped) |
| **RAM Consumption** | 2.8 MB - 3.2 MB |
| **CPU Usage** | 0.0% idle |
| **GDI Handles** | 6 handles |
| **Startup Time** | < 15 ms |
| **Dependencies** | None (Static CRT / Native OS libraries) |

---

## Contributing

Contributions, bug reports, and suggestions are welcome. Please open an issue or submit a pull request on GitHub.

---

## License

This project is licensed under the terms of the [MIT License](LICENSE).
