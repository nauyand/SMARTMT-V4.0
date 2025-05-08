# SmartMT v4.0 - New Design 2025 🚛📡

**SmartMT (Smart Moda Transportasi)** adalah sistem pemantauan dan kontrol kendaraan cerdas berbasis dua CPU (2x Arduino Mega). Dirancang untuk memantau tekanan ban, suhu, tegangan, serta mengontrol sistem pneumatik secara lokal dan jarak jauh melalui internet.

## 🧠 Arsitektur Sistem

+-----------------------------+ SoftwareSerial +------------------------------+
| MCU 1 | <---------------------> | MCU 2 |
| (Main Controller) | | (Sub Controller) |
|----------------------------| |------------------------------|
| - 4 Sensor Suhu (MAX6675) | | - 2x TPMS tambahan (serial) |
| - Sensor Tekanan Ban (TPMS)| | - 4 Sensor Suhu (MAX6675) |
| - Sensor Tegangan Baterai | | - Sensor Opsional |
| - Sensor Tekanan Udara | | - Kontrol Motor Pneumatik |
| - RTC (Timestamp) |
| - SD Card (Logging lokal) |
+------------------------------+
| - ESP8266 (HTTP ke Web/HP) |
+----------------------------+

### Fitur Utama

#### MCU 1 (CPU Utama)
- Membaca sensor:
  - Baca 4 sensor suhu
  - Tekanan ban (TPMS) via rs232
  - humidity
  - Tegangan
  - Tekanan udara
- Waktu dari RTC untuk penanda log
- Simpan data ke SD card
- Kirim/Terima data :
  - Web server via ESP8266 (Via local HTTP)
  - Website protocol GPS teltonika
  - Aplikasi mobile (Via HTTP)
  - MCU 2 (via `rs485`)

#### MCU 2 (CPU Tambahan)
- Baca 4 sensor suhu tambahan
- Kontrol sistem pneumatik (via motor driver)
- Kirim umpan balik ke MCU 1 lewat komunikasi rs485

## 🌐 Koneksi Internet (ESP8266)

ESP8266 digunakan oleh MCU 1 untuk mengirim data ke:
- Server Web (PHP / Node.js / Google Sheets)
- Aplikasi Mobile (via HTTP)
- Format data: JSON atau Query String
- Interval pengiriman: 10–30 detik


