# Đồ án 1 — ESP32 + Client–Server qua LAN (REST & MQTT)

## 1) Thành phần
- **Server (Python FastAPI)**: nhận dữ liệu từ client (ESP32/PC) qua LAN.
- **ESP32**:
  - `rest_demo`: gửi POST JSON đến server (`/sensor`).
  - `mqtt_demo`: publish/subscribe qua MQTT broker.
- **PC (tùy chọn)**: script MQTT để mô phỏng client.

## 2) Chuẩn bị
- Cài **Python 3.10+** và **pip**.
- Cài **VS Code** (khuyến nghị).
- **Arduino IDE** (hoặc PlatformIO). Thêm board ESP32:
  - Mở *File → Preferences* → *Additional Boards Manager URLs*,
    dán: `https://dl.espressif.com/dl/package_esp32_index.json`
  - Vào *Tools → Board → Boards Manager*, tìm **esp32** (Espressif) và **Install**.
- (MQTT) Cài **Mosquitto** broker, mở cổng **1883**.

## 3) Chạy server (REST)
```bash
cd server
python -m venv .venv
# Windows: .venv\Scripts\activate      macOS/Linux: source .venv/bin/activate
pip install -r requirements.txt
uvicorn server:app --host 0.0.0.0 --port 8000
```
- Ghi lại IP LAN của máy chạy server (ví dụ `192.168.1.10`).
- Test: `http://<IP>:8000/ping`

> Bảo mật đơn giản: đặt API_KEY trong `server.py` và thêm header `X-API-Key` ở client.

## 4) Nạp code ESP32
- Mở `esp32/rest_demo/esp32_rest.ino` hoặc `esp32/mqtt_demo/esp32_mqtt.ino` trong Arduino IDE.
- Chọn board: **ESP32 Dev Module** (hoặc đúng model của bạn).
- Sửa `WIFI_SSID`, `WIFI_PASS`, `SERVER_IP` (REST) hoặc `MQTT_BROKER` (MQTT) cho đúng LAN.
- **Upload** và mở *Serial Monitor* (115200 baud).

## 5) MQTT nhanh (tùy chọn)
- Sub: `mosquitto_sub -h <IP broker> -t "lab/sensor/#"`
- Pub test: `mosquitto_pub -h <IP broker> -t "lab/sensor/esp32" -m '{"id":"esp32","value":23.5}'`

## 6) Khắc phục lỗi
- Không truy cập được: kiểm tra cùng subnet, firewall, port 8000/1883, IP tĩnh.
- ESP32 không vào Wi‑Fi: dùng băng 2.4GHz, đúng SSID/Pass.
- MQTT không hiện message: sai topic, chưa chạy broker, chưa gọi `loop()`.

## 7) Gợi ý báo cáo
- Kiến trúc, ảnh chụp log server/Serial, test bằng Postman/MQTT Explorer, bảo mật, mở rộng.
