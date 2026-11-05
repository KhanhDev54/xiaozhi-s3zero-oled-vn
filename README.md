# xiaozhi-s3zero-oled-vn

Firmware chatbot AI Xiaozhi **tiếng Việt** cho mạch tự lắp **ESP32-S3 Zero + OLED 0.96"**
(INMP441 + MAX98357A), theo sơ đồ *"ChatBot AI (Xiaozhi) mod by tandev"*.
Dựa trên bản gốc [78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) v2.5.3.

👉 **Trang nạp firmware:** https://khanhdev54.github.io/xiaozhi-s3zero-oled-vn/
(dùng chung với mạch 2.8" — trang liệt kê firmware của cả hai mạch)

---

## Mục lục

1. [Tính năng](#1-tính-năng)
2. [Phần cứng & sơ đồ chân](#2-phần-cứng--sơ-đồ-chân)
3. [Nạp firmware](#3-nạp-firmware)
4. [Cài Wi-Fi & kích hoạt](#4-cài-wi-fi--kích-hoạt)
5. [Trò chuyện](#5-trò-chuyện)
6. [Nút bấm & đèn](#6-nút-bấm--đèn)
7. [Báo thức, hẹn giờ, âm lịch](#7-báo-thức-hẹn-giờ-âm-lịch)
8. [Màn hình chờ, thời tiết, tin tức](#8-màn-hình-chờ-thời-tiết-tin-tức)
9. [Xử lý sự cố](#9-xử-lý-sự-cố)
10. [Dành cho người phát triển](#10-dành-cho-người-phát-triển)
11. [Nguồn gốc & giấy phép](#11-nguồn-gốc--giấy-phép)

---

## 1. Tính năng

| Tính năng | Ghi chú |
|---|---|
| 🗣️ Trò chuyện AI bằng giọng nói | Qua server xiaozhi.me, trả lời tiếng Việt |
| 🇻🇳 Giao diện & âm báo tiếng Việt | Ngôn ngữ firmware `vi-VN` |
| 👂 Từ đánh thức **"Jarvis"** | Nhận dạng offline trên chip (nói "Jarvis" hoặc "Hi Jarvis") |
| 🖥️ OLED 0.96" 128x64 | Trạng thái, **mặt "mochi" động** (chớp mắt, nhún, liếc khi nghe, mấp máy miệng khi nói, đổi biểu cảm theo cảm xúc), nội dung trò chuyện |
| ⏰ Báo thức, hẹn giờ | Ra lệnh bằng giọng, lưu khi tắt nguồn |
| 📅 Âm lịch | Hiện trên màn chờ; hỏi "hôm nay âm lịch ngày mấy", can chi, giờ hoàng đạo |
| 🕒 Màn hình chờ | Giờ to, biểu tượng thời tiết, thứ/ngày, âm lịch, nhiệt độ, AQI, "còn x ngày tới Rằm/mùng 1"; tự giảm sáng 22h–6h |
| 🌤️ Thời tiết | Open-Meteo (không cần key): hiện tại + 3 ngày tới, chất lượng không khí |
| 📰 Tin tức | Đọc tin mới nhất, thể thao, thế giới… từ RSS VnExpress |
| 📻 Radio VOV | VOV1, VOV2, VOV3, VOV5, VOV Giao thông HN/HCM, VOV Mekong |
| 🪙 Giá vàng SJC | Lấy từ website SJC (sjc.com.vn), đồng/lượng |
| 💱 Tỷ giá Vietcombank | USD, EUR, JPY, KRW, CNY… mua tiền mặt / chuyển khoản / bán |
| 💡 Đèn GPIO12 | Bật/tắt bằng giọng nói |

## 2. Phần cứng & sơ đồ chân

- **Waveshare ESP32-S3 Zero** (4MB Flash, 2MB PSRAM)
- OLED **SSD1306 0.96" 128x64** I2C (địa chỉ 0x3C)
- Micro **INMP441**, khuếch đại **MAX98357A** + loa
- 3 nút bấm, 1 LED, sạc pin TP4056 (tuỳ chọn)

| Thiết bị | Chân thiết bị | ESP32-S3 Zero |
|---|---|---|
| OLED | VCC / GND / SCL / SDA | 3V3 / GND / **GPIO9** / **GPIO8** |
| MAX98357A | DIN / BCLK / LRC | **GPIO5** / **GPIO6** / **GPIO7** |
| MAX98357A | VIN / GAIN / SD / GND | 5V / GND / 5V / GND |
| INMP441 | WS / SCK / SD | **GPIO2** / **GPIO3** / **GPIO4** |
| INMP441 | VDD / GND / L/R | 3V3 / GND / GND |
| Nút đánh thức | (nối GND khi nhấn) | **GPIO1** |
| Nút tăng âm lượng | (nối GND khi nhấn) | **GPIO10** |
| Nút giảm âm lượng | (nối GND khi nhấn) | **GPIO11** |
| LED / đèn ngoài | (qua điện trở 220–330Ω) | **GPIO12** |
| LED RGB trên bo | WS2812 | GPIO21 (trạng thái) |

## 3. Nạp firmware

1. Mở **https://khanhdev54.github.io/xiaozhi-s3zero-oled-vn/** bằng **Chrome/Edge** trên máy tính.
2. Cắm mạch Zero bằng cáp USB-C **có truyền dữ liệu**.
3. Bấm **Kết nối** → chọn cổng COM → **Connect**.
   Không thấy cổng: **giữ BOOT**, cắm lại cáp (hoặc bấm RESET), thả BOOT.
4. Trong danh sách, chọn firmware ở nhóm **"ESP32-S3 Zero + OLED 0.96" (4MB)"** (đã chọn sẵn).
5. Lần đầu nạp: tích **"Xoá toàn bộ flash trước khi nạp"**.
6. Bấm **Bắt đầu nạp**. Xong thì bấm RESET nếu màn hình chưa sáng.

> Trang sẽ **cảnh báo nếu chọn nhầm** firmware 16MB của mạch 2.8" cho mạch Zero 4MB.

Tải file về tự nạp: tab **Actions** → lần chạy mới nhất ✓ → **Artifacts** → `merged-binary.bin`
→ nạp ở địa chỉ `0x0`.

## 4. Cài Wi-Fi & kích hoạt

1. Lần đầu, màn hình hiện tên Wi-Fi mạch phát ra (bắt đầu bằng `KenDev`) và địa chỉ cài đặt.
2. Điện thoại kết nối Wi-Fi đó → chọn Wi-Fi nhà (**2.4GHz**) → nhập mật khẩu.
3. Mạch đọc **mã kích hoạt 6 số** → vào **https://xiaozhi.me** → Console → thêm thiết bị → nhập mã.
4. Trong cấu hình Agent trên xiaozhi.me, chọn ngôn ngữ / giọng đọc tiếng Việt.

Đổi Wi-Fi sau này: **bấm nhanh 3 lần** nút đánh thức (GPIO1) bất cứ lúc nào, hoặc bấm 1 lần lúc mạch đang khởi động. Mạch phát Wi-Fi **`KenDev-xxxx`**, vào `192.168.4.1` để cài.

### Trang cài đặt (192.168.4.1)

Trang cài đặt có 3 thẻ, giao diện tiếng Việt:

| Thẻ | Nội dung |
|---|---|
| **Wi-Fi** | Danh sách mạng xung quanh (có vạch sóng), Wi-Fi đã lưu (đặt ưu tiên / xóa), nhập mật khẩu |
| **Tùy chỉnh** | Thành phố cho thời tiết (để trống = tự đoán theo IP), âm lượng khi khởi động, giờ tự giảm sáng màn hình ban đêm |
| **Nâng cao** | Địa chỉ OTA, công suất phát Wi-Fi, ghi nhớ BSSID |

Mẹo: vào thẻ **Tùy chỉnh** và bấm **Lưu** *trước* khi bấm **Kết nối** ở thẻ Wi-Fi, vì sau khi kết nối
thành công mạch sẽ tắt trang cài đặt. Tùy chỉnh có hiệu lực từ lần khởi động sau.

> Giao diện xiaozhi.me có thể đổi theo thời gian; tên các mục chỉ mang tính tham khảo.

## 5. Trò chuyện

- Nói **"Jarvis"** (đọc *"Gia-vịt"*, nói "Hi Jarvis" cũng được), hoặc bấm **nút đánh thức** (GPIO1).
- Sau khi đánh thức, nói chuyện **bằng tiếng Việt** bình thường.
- Đang nói / đang nghe: bấm nút đánh thức để ngắt / kết thúc.

## 6. Nút bấm & đèn

| Nút | Bấm 1 lần | Giữ lâu |
|---|---|---|
| Đánh thức (GPIO1) | Bắt đầu / dừng trò chuyện | — |
| Tăng âm lượng (GPIO10) | +10 | Âm lượng tối đa |
| Giảm âm lượng (GPIO11) | −10 | Tắt tiếng |

Đèn GPIO12: nói *"Bật đèn"*, *"Tắt đèn"*.

## 7. Báo thức, hẹn giờ, âm lịch

| Câu lệnh mẫu | Kết quả |
|---|---|
| *"Đặt báo thức 6 giờ 30 sáng"* | Báo thức một lần |
| *"Báo thức 7 giờ mỗi ngày, nhãn đi làm"* | Báo thức lặp hằng ngày |
| *"Mình có những báo thức nào?"* | Liệt kê báo thức |
| *"Xoá hết báo thức"* | Xoá |
| *"Hẹn giờ 10 phút"* | Đếm ngược |
| *"Tắt báo thức"* | Tắt chuông đang kêu |
| *"Hôm nay âm lịch ngày mấy?"*, *"Giờ hoàng đạo hôm nay?"* | Trả lời bằng giọng |

- Chuông kêu tối đa **1 phút**; bấm nút đánh thức hoặc bắt đầu nói chuyện để tắt.
- Lưu tối đa 8 báo thức trong bộ nhớ mạch, **không mất khi tắt nguồn**.
- Cần mạch đã có giờ chuẩn (đã kết nối mạng sau khi bật).

## 8. Màn hình chờ, thời tiết, tin tức

### Màn hình chờ (OLED 0.96")

Khi rảnh khoảng 5 giây, OLED chuyển sang màn hình chờ:

```
 09:18          ☁        <- biểu tượng thời tiết vẽ bằng nét (nắng, trăng, mây, mưa, giông, sương mù)
 T5 05/11 · 26/9 ÂL      <- dòng dưới đổi mỗi 5 giây:
 23° Mưa rào · AQI 96       ngày + âm lịch -> thời tiết -> đếm ngày tới Rằm / mùng 1
 Còn 4 ngày tới mùng 1      (đang nghe radio thì có thêm tên kênh)
```

- Ban ngày OLED để độ sáng vừa phải (đỡ tốn điện); từ **22 giờ đến 6 giờ sáng** giảm sáng hẳn (đổi giờ trong trang cài đặt).
- Ẩn ngay khi trò chuyện, khi báo thức kêu, hoặc 4 giây sau khi bấm nút (để thấy thông báo âm lượng).
- Cần mạch đã có giờ chuẩn (đã kết nối mạng).

### Thời tiết

Hỏi: *"Thời tiết hôm nay thế nào?"*, *"Ngày mai có mưa không?"*, *"Không khí hôm nay có tốt không?"*.

- Dữ liệu thật từ [Open-Meteo](https://open-meteo.com) (miễn phí, không cần key), dự báo 3 ngày tới, AQI.
- Vị trí tự xác định theo địa chỉ IP; cập nhật 30 phút/lần.

### Giá vàng & tỷ giá

Hỏi: *"Giá vàng hôm nay bao nhiêu?"*, *"Giá vàng SJC ở Hà Nội"*, *"Tỷ giá đô hôm nay"*, *"Tỷ giá euro"*.

- **Giá vàng**: lấy từ website của **Công ty Vàng bạc Đá quý Sài Gòn (SJC)**, đơn vị đồng/lượng,
  mặc định khu vực Hồ Chí Minh.
- **Tỷ giá**: lấy từ bảng tỷ giá công khai của **Vietcombank** (mua tiền mặt, mua chuyển khoản, bán).
- Đây là dữ liệu trên website của SJC và Vietcombank, **không phải API chính thức có tài liệu**;
  nếu họ đổi website thì tính năng có thể ngừng chạy cho tới khi firmware được cập nhật.
- Chỉ để tham khảo, không dùng thay cho bảng giá tại quầy.

### Tin tức

Hỏi: *"Tin tức hôm nay có gì?"*, *"Có tin thể thao gì mới?"*, *"Đọc tin thế giới"*, *"Tin công nghệ"*.

- Lấy từ RSS của **VnExpress**: tin mới nhất, thời sự, thế giới, kinh doanh, thể thao, giải trí,
  khoa học công nghệ, sức khỏe, giáo dục, pháp luật (mặc định 5 tin).
- Trợ lý chỉ đọc **tiêu đề + tóm tắt** có trong RSS; không có tỉ số trực tiếp hay bảng xếp hạng.
- Dùng cho mục đích cá nhân.

### Radio VOV

Nói: *"Mở radio VOV1"*, *"Nghe VOV giao thông Hà Nội"*, *"Mở đài âm nhạc"*, *"Tắt radio"*.

- Kênh: VOV1 (thời sự), VOV2 (văn hoá), VOV3 (âm nhạc), VOV5, VOV Giao thông Hà Nội / TP.HCM, VOV Mekong.
- Radio **bắt đầu phát sau khi cuộc trò chuyện kết thúc**, và **tự dừng khi bạn gọi "Jarvis"**
  hoặc bấm nút đánh thức — muốn nghe tiếp thì bảo mở lại.
- Nút tăng/giảm âm lượng dùng được khi đang nghe radio; màn chờ hiện tên kênh.
- Luồng phát trực tiếp từ `stream.vovmedia.vn` (AAC). Cần mạng ổn định.

## 9. Xử lý sự cố

| Hiện tượng | Cách xử lý |
|---|---|
| Không thấy cổng COM | Cáp USB có dữ liệu; giữ BOOT khi cắm |
| Màn OLED không sáng | Kiểm tra SDA=GPIO8, SCL=GPIO9, nguồn 3V3; OLED địa chỉ 0x3C |
| Chữ OLED bị ngược | Đổi `DISPLAY_MIRROR_X/Y` trong `config.h` |
| Không nghe tiếng | MAX98357A: DIN=5, BCLK=6, LRC=7, VIN 5V |
| Không nhận giọng | INMP441: L/R nối GND, WS=2, SCK=3, SD=4 |
| Khởi động lại liên tục | Nạp lại với ô *Xoá toàn bộ flash* |
| Màn chờ không hiện | Mạch cần có giờ chuẩn (kết nối mạng); chờ ~5 giây sau khi rảnh |
| Không đọc được tin tức / thời tiết | Kiểm tra mạng ra Internet; thử lại sau ít phút |
| Radio không phát / bị ngắt | Chờ cuộc trò chuyện kết thúc; kiểm tra mạng; thử kênh khác |
| Cần gửi log cho người hỗ trợ | Trang nạp → mục **3. Xem log của mạch** → *Mở log* → thao tác lỗi → *Tải file .txt* |

## 10. Dành cho người phát triển

Mỗi lần push lên `main` (trừ khi chỉ sửa file `.md`), workflow
[`build-zero-oled.yml`](.github/workflows/build-zero-oled.yml) build bản `esp32s3-zero-oled-096`
bằng ESP-IDF 6.1 (`--language vi-VN --wake-word wn9_sophia_tts`) rồi đăng lên trang nạp.

Build trên máy (ESP-IDF 6.0.1+, khuyên dùng 6.1):

```
python scripts/build.py esp32s3-zero-oled --name esp32s3-zero-oled-096 --language vi-VN --wake-word wn9_sophia_tts
idf.py -p COM5 flash monitor
```

| File | Nội dung |
|---|---|
| `main/boards/esp32s3-zero-oled/config.h` | Sơ đồ chân, kiểu OLED, lật màn hình |
| `main/boards/esp32s3-zero-oled/config.json` | Bản build (0.91" / 0.96" / 1.3" SH1106), flash 4MB |
| `main/boards/esp32s3-zero-oled/alarm_manager.*` | Báo thức, hẹn giờ, lệnh âm lịch |
| `main/boards/esp32s3-zero-oled/lunar_calendar.*` | Âm lịch Việt Nam |
| `main/boards/esp32s3-zero-oled/idle_screen.*` | Màn hình chờ OLED |
| `main/boards/esp32s3-zero-oled/info_services.*` | Thời tiết Open-Meteo, tin tức RSS |
| `main/boards/esp32s3-zero-oled/rss_parser.*` | Đọc RSS (kiểm thử được trên máy tính) |
| `main/boards/esp32s3-zero-oled/radio_player.*` | Radio VOV (giải mã AAC, đổi tần số mẫu) |
| `main/audio/audio_service.*` | Thêm `KeepOutputAlive()` để radio giữ loa bật |
| `web/` | Trang nạp (dùng chung với repo mạch 2.8") |

Đổi từ đánh thức: sửa `--wake-word` trong workflow, ví dụ `wn9_alexa`, `wn9_jarvis_tts`, `wn9_hiesp`.

## 11. Nguồn gốc & giấy phép

- Dựa trên [78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) (commit `a73e9a7`, v2.5.3).
  README gốc: [README_upstream.md](README_upstream.md).
- Sơ đồ chân theo hình *"ChatBot AI (Xiaozhi) mod by tandev"*.
- Thuật toán âm lịch của Hồ Ngọc Đức.
- Trang nạp dùng [esptool-js](https://github.com/espressif/esptool-js) của Espressif.
- Giấy phép MIT — xem [LICENSE](LICENSE).
