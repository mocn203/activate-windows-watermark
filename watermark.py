"""
Activate Windows Watermark - Python Edition (Zero Dependencies)
A troll watermark overlay that runs using Python standard library (tkinter + ctypes).
"""

import sys
import ctypes
from ctypes import wintypes
import tkinter as tk

# Enable Per-Monitor DPI awareness
try:
    ctypes.windll.shcore.SetProcessDpiAwareness(2)
except Exception:
    try:
        ctypes.windll.user32.SetProcessDPIAware()
    except Exception:
        pass

GWL_EXSTYLE = -20
WS_EX_LAYERED = 0x00080000
WS_EX_TRANSPARENT = 0x00000020
WS_EX_TOOLWINDOW = 0x00000080
WS_EX_NOACTIVATE = 0x08000000

class WatermarkApp:
    def __init__(self, title="Activate Windows", subtitle="Go to Settings to activate Windows."):
        self.root = tk.Tk()
        self.root.overrideredirect(True)
        self.root.wm_attributes("-topmost", True)

        # Transparent background color key
        self.bg_color = "#010203"
        self.root.configure(bg=self.bg_color)
        self.root.wm_attributes("-transparentcolor", self.bg_color)
        self.root.wm_attributes("-alpha", 0.65)

        # Calculate positioning
        screen_w = self.root.winfo_screenwidth()
        screen_h = self.root.winfo_screenheight()

        # Watermark Frame
        frame = tk.Frame(self.root, bg=self.bg_color)
        frame.pack(padx=10, pady=10)

        # Segoe UI font (Standard Windows font)
        lbl_title = tk.Label(
            frame,
            text=title,
            font=("Segoe UI", 15, "normal"),
            fg="#FFFFFF",
            bg=self.bg_color,
            anchor="w"
        )
        lbl_title.pack(anchor="w")

        lbl_sub = tk.Label(
            frame,
            text=subtitle,
            font=("Segoe UI", 10, "normal"),
            fg="#EDEDED",
            bg=self.bg_color,
            anchor="w"
        )
        lbl_sub.pack(anchor="w", pady=(2, 0))

        self.root.update_idletasks()
        win_w = frame.winfo_reqwidth() + 20
        win_h = frame.winfo_reqheight() + 20

        # Bottom right placement (above taskbar)
        margin_x = 40
        margin_y = 60
        pos_x = screen_w - win_w - margin_x
        pos_y = screen_h - win_h - margin_y

        self.root.geometry(f"{win_w}x{win_h}+{pos_x}+{pos_y}")

        # Set click-through extended window styles
        hwnd = self.root.winfo_id()
        user32 = ctypes.windll.user32
        
        # Get top-level window handle
        hwnd = user32.GetParent(hwnd) or hwnd
        
        ex_style = user32.GetWindowLongW(hwnd, GWL_EXSTYLE)
        ex_style |= WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE
        user32.SetWindowLongW(hwnd, GWL_EXSTYLE, ex_style)

        # Periodically ensure topmost
        self.keep_topmost()

    def keep_topmost(self):
        self.root.lift()
        self.root.wm_attributes("-topmost", True)
        self.root.after(2000, self.keep_topmost)

    def run(self):
        self.root.mainloop()

if __name__ == "__main__":
    title = sys.argv[1] if len(sys.argv) > 1 else "Activate Windows"
    subtitle = sys.argv[2] if len(sys.argv) > 2 else "Go to Settings to activate Windows."
    print("Running Python Watermark... Press Ctrl+C in terminal to stop.")
    app = WatermarkApp(title=title, subtitle=subtitle)
    app.run()
