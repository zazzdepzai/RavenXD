# RavenXD Launcher (C++/Qt6)

Launcher Minecraft dựng bằng **C++17 + Qt6 Widgets**, giao diện phỏng theo bản
thiết kế (sidebar, hero banner, chọn phiên bản/RAM/mod loader, panel tin tức +
thông tin hệ thống bên phải), kèm module **tự động cập nhật mod**.

## Yêu cầu

- CMake >= 3.16
- Qt6 (Widgets + Network) — cài qua Qt Online Installer hoặc:
  - Windows: `winget install Qt.Qt` hoặc MSYS2 `pacman -S mingw-w64-x86_64-qt6-base`
  - macOS: `brew install qt`
  - Linux: `sudo apt install qt6-base-dev`
- Trình biên dịch hỗ trợ C++17 (MSVC, MinGW, GCC, Clang)

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="<đường-dẫn-tới-Qt6>"
cmake --build . --config Release
```

Chạy file thực thi `RavenXDLauncher` (hoặc `.exe` trên Windows) sinh ra trong
thư mục `build`.

## Cách auto-update mod hoạt động (`src/Updater.h/.cpp`)

1. Khi bấm **Download / Update**, launcher tải file JSON tại `manifestUrl`
   (xem mẫu ở `manifest.example.json`) — mỗi mod gồm `name`, `fileName`,
   `url`, `version`, `sha256`.
2. So sánh với phiên bản đã cài, lưu trong
   `<thư mục dữ liệu ứng dụng>/instances/<version>/mods/.installed_versions.ini`.
   Mod bị coi là cần cập nhật nếu: file chưa tồn tại **hoặc** version trong
   manifest khác version đã ghi nhận cục bộ.
3. Tải tuần tự từng mod cần cập nhật về file `.part`, sau đó:
   - Nếu manifest có `sha256`, tính hash SHA-256 của file tải về và so khớp —
     sai thì huỷ, không ghi đè bản cũ.
   - Đúng thì đổi tên `.part` → tên thật, ghi lại version mới vào
     `.installed_versions.ini`.
4. Tiến trình cập nhật (đang tải mod nào, %, số mod đã xong/lỗi) được phát ra
   qua signal và hiển thị ở dòng trạng thái + progress bar phía dưới nút PLAY.

### Tự triển khai server phía bạn

Bạn cần tự host file `manifest.json` (S3, GitHub Releases, CDN riêng...) và
cập nhật nó mỗi khi phát hành mod mới — launcher không tự sinh ra file này.
Sửa URL thật trong `MainWindow.cpp`:

```cpp
m_updater->setManifestUrl("https://updates.example.com/ravenxd/manifest.json");
```

## Cấu trúc thư mục

```
RavenXDLauncher/
├── CMakeLists.txt
├── manifest.example.json     # mẫu file manifest cho auto-update
├── README.md
└── src/
    ├── main.cpp
    ├── MainWindow.h / .cpp    # toàn bộ giao diện
    └── Updater.h / .cpp       # logic kiểm tra & tải cập nhật mod
```

## Việc còn cần bạn tự làm thêm (chưa nằm trong bản này)

- Logic thật để **launch Minecraft** (tải version manifest của Mojang, tải
  Forge/Fabric installer, build đúng classpath & JVM args rồi `QProcess::start`).
- Đăng nhập tài khoản Microsoft/Mojang (OAuth device code flow).
- Tải & giải nén version 1.20.1/1.21+ client, tài nguyên (assets/libraries).

Phần **UI + auto-update mod** trong bản này đã chạy được độc lập để bạn cắm
thêm các phần trên vào.
