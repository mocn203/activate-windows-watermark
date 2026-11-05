# 🪟 Dự Án 1: Activate Windows Watermark (Troll Edition) 🪟

> **"Máy mua bản quyền tiền triệu nhưng vẫn nhớ cảm giác dùng Windows lậu!"** 😂  
> Công cụ hiển thị dòng chữ **"Activate Windows / Go to Settings to activate Windows."** đè lên màn hình cho các máy đã kích hoạt bản quyền xịn. Dùng để troll bạn bè, đồng nghiệp, streamer hoặc sống lại cảm giác hoài niệm năm xưa!

---

## ✨ Điểm nổi bật & Tính năng "Cực Hài"

1. **Chuẩn 100% Watermark Thật của Microsoft**:
   - Sử dụng đúng font chuẩn **Segoe UI** của hệ thống Windows.
   - Kích thước tỉ lệ chuẩn và độ mờ bán trong suốt (alpha blending per-pixel) chân thật 1:1.
   - Tự động nhận diện DPI scaling (100%, 125%, 150%, 200%) giữ chữ luôn sắc nét từng pixel.

2. **Xuyên thấu tuyệt đối (Click-Through)**:
   - Sử dụng cờ Win32 `WS_EX_TRANSPARENT | WS_EX_LAYERED`.
   - Chuột bấm, click, kéo thả, cuộn trang hay chơi game bắn súng đều **xuyên qua chữ 100%**, hoàn toàn không bị vướng víu!

3. **Tàng hình chuyên nghiệp**:
   - Không xuất hiện trong `Alt + Tab`.
   - Không hiện biểu tượng dưới Taskbar (chạy nền nhẹ nhàng qua System Tray).
   - Luôn nổi trên cùng (`TopMost`) kể cả khi mở app khác.

4. **Phím tắt cứu nguy khẩn cấp (`Panic Hotkey`)**:
   - Bấm **`Ctrl + Alt + W`**: Ẩn/Hiện watermark tức thì trong 0.01 giây khi đồng nghiệp hay bạn gái lại gần!

5. **Bộ 7 Presets Troll Đỉnh Cao**:
   - 🔹 **1. Activate Windows (Mặc định)**: *"Activate Windows / Go to Settings to activate Windows."*
   - 🔹 **2. Windows 11 Enterprise**: *"Windows 11 Enterprise / Evaluation copy. Build 26100.ge_release"*
   - 🔹 **3. AI Copilot**: *"Activate Copilot / Go to Settings to buy Copilot Pro."*
   - 🔹 **4. Linux Troll**: *"Activate Linux / Go to Settings to install Arch Linux btw."*
   - 🔹 **5. Touch Grass**: *"Activate Real Life / Go outside and touch some grass."*
   - 🔹 **6. Tiếng Việt**: *"Kích hoạt Windows / Vào Cài đặt để kích hoạt Windows."*
   - 🔹 **7. Custom Text...**: Mở bảng nhập tự do bất kỳ chữ gì bạn muốn troll!

6. **Điều khiển từ Khay hệ thống (System Tray)**:
   - Click phải vào icon Windows ở góc khay đồng hồ để chỉnh:
     - Bật/Tắt Watermark.
     - Đổi độ mờ (**30%**, **45%**, **60%**, **80%**, **100%**).
     - Đổi vị trí (**Bottom-Right**, **Bottom-Left**, **Top-Right**, **Top-Left**).
     - Hỗ trợ nhiều màn hình (**Primary Monitor Only** hoặc **All Connected Monitors**).
     - Thoát ứng dụng.

7. **Siêu nhẹ**:
   - File thực thi native C++ `activate-windows.exe` chỉ vỏn vẹn **~44 KB**!
   - Không cần cài .NET runtime, không cần Python hay cài đặt phức tạp.

---

## 🚀 Cách sử dụng nhanh

### Cách 1: Chạy trực tiếp (Khuyên dùng)
- Chạy file:
  ```cmd
  run.bat
  ```
  hoặc nhấp đúp trực tiếp vào file **`activate-windows.exe`**.
- Dòng watermark sẽ lập tức xuất hiện ở góc dưới bên phải màn hình!
- Bấm **`Ctrl + Alt + W`** để bật / tắt nhanh.
- Nhấp phải icon Windows ở khay hệ thống (System Tray gần đồng hồ) để chỉnh Preset hoặc tuỳ biến.

### Cách 2: Tắt ứng dụng
- Chọn **Exit** trong menu chuột phải icon khay hệ thống, hoặc chạy:
  ```cmd
  stop.bat
  ```

---

## 🛠️ Biên dịch từ mã nguồn (Build from Source)

Nếu bạn muốn chỉnh sửa mã nguồn C++ trong `main.cpp` và tự build lại:
```cmd
build.bat
```
Lệnh tương đương (sử dụng MinGW-w64 GCC):
```cmd
g++ -O2 -s -mwindows -municode main.cpp -o activate-windows.exe -lgdiplus -lgdi32 -luser32 -lshell32 -lcomctl32 -lole32
```

---

## 🐍 Phiên bản Python (Dành cho ai thích Python)

Dự án cung cấp sẵn file `watermark.py` thuần Standard Library (không cần cài thêm pip thư viện ngoài):
```cmd
python watermark.py
```
Hoặc tuỳ chỉnh chữ:
```cmd
python watermark.py "Kích hoạt Windows" "Mua key dạo đi em ơi"
```

---

## ⌨️ Tham số dòng lệnh (CLI Parameters)

Bạn có thể chạy `activate-windows.exe` kèm các flag để tự động hóa hoặc troll ẩn danh:

```cmd
:: Chạy chế độ tàng hình (không hiện icon tray)
activate-windows.exe --silent

:: Chạy trên tất cả các màn hình
activate-windows.exe --all-monitors

:: Chọn preset (1 đến 5)
activate-windows.exe --preset 4

:: Đặt chữ tùy biến qua lệnh
activate-windows.exe --title "Tài khoản Hết Hạn" --subtitle "Vui lòng nộp 500k để tiếp tục sử dụng máy"

:: Đặt độ đậm chữ 80%
activate-windows.exe --opacity 80
```

---

*Chúc anh em troll vui vẻ và có những tràng cười sảng khoái!* 🎉


---

## 🏛️ Cách 3: Bật Watermark GỐC 100% Của Chính Windows (Native DWM)

Nếu anh muốn **Watermark GỐC 100%** do chính nhân đồ họa DWM (`dwmcore.dll`) của Microsoft vẽ (hàng auth chính hiệu, không qua app overlay):

### Cơ chế hoạt động:
- Windows chỉ vẽ dòng watermark gốc khi phát hiện hệ thống ở trạng thái **Notification Mode (Chưa kích hoạt)**.
- Microsoft không cung cấp nút bấm hay key Registry riêng để bật riêng watermark mà không đưa máy về trạng thái chưa kích hoạt.
- **Yên tâm 100% về bản quyền**: Em đã kiểm tra máy của anh có sẵn **Key OEM vĩnh viễn lưu trong BIOS bo mạch chủ (UEFI MSDM)**:  
  `KHDHN-4DGHP-HWVJ2-VQ8W8-6MVCK`  
  Nên anh có thể gỡ key ra để trải nghiệm và nạp lại bất cứ lúc nào!

### Hướng dẫn sử dụng:
1. **Để bật Watermark gốc**:  
   Click chuột phải vào **`native_enable.bat`** -> Chọn **Run as administrator**.  
   *(Windows Explorer sẽ khởi động lại và dòng watermark gốc của Windows sẽ hiện lên).*
2. **Để khôi phục lại bản quyền gốc**:  
   Click chuột phải vào **`native_restore.bat`** -> Chọn **Run as administrator**.  
   *(Script sẽ tự động lấy key OEM từ BIOS nạp lại và kích hoạt bản quyền xanh mướt trở lại!)*
