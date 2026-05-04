# Bài Tập Lớn - Đồ Họa Máy Tính (HK II, 2025-2026)

## Mô Hình Thiết Bị Cơ Khí Đơn Giản

Bài tập lớn môn **Đồ Họa Máy Tính**, xây dựng mô hình 3D thiết bị cơ khí có thể tương tác được bằng OpenGL.

---

## Mục Tiêu

- Mô hình hóa các đối tượng đơn giản và kết hợp thành đối tượng phức hợp
- Thực hành các phép biến đổi hình học (quay, dịch chuyển)
- Điều khiển camera trong không gian 3D
- Tô màu và ánh sáng (Lighting & Shading)

---

## Cấu Trúc Mô Hình Thiết Bị

Thiết bị gồm 6 bộ phận chính, lồng ghép theo thứ bậc:

| STT | Bộ phận | Mô tả |
|-----|---------|-------|
| 1 | **Đế** (Base) | 2 hình trụ gắn cứng; quay tự do quanh trục của nó, toàn bộ thiết bị quay theo |
| 2 | **Khung** (Frame) | 1 hình xuyến + 2 hình trụ (màu tím); gắn cứng vào đế |
| 3 | **Khớp vạn năng 1** (Gimbal 1) | 1 hình xuyến + 2 hình trụ (màu xanh lá); quay quanh trục do 2 hình trụ của khung tạo ra |
| 4 | **Khớp vạn năng 2** (Gimbal 2) | 1 hình xuyến (màu đỏ); quay quanh trục do 2 hình trụ của Gimbal 1 tạo ra |
| 5 | **Trục** (Axis) | Hình trụ (màu đỏ); gắn cố định vào Gimbal 2 |
| 6 | **Đĩa quay** (Rotor) | Logo trường Bách Khoa; quay quanh trục |

---

## Điều Khiển

### Điều Khiển Thiết Bị

| Phím | Hành động |
|------|-----------|
| `1` | Đế quay ngược chiều kim đồng hồ |
| `2` | Đế quay cùng chiều kim đồng hồ |
| `3` | Khớp vạn năng 1 quay ngược chiều kim đồng hồ |
| `4` | Khớp vạn năng 1 quay cùng chiều kim đồng hồ |
| `5` | Khớp vạn năng 2 quay ngược chiều kim đồng hồ |
| `6` | Khớp vạn năng 2 quay cùng chiều kim đồng hồ |
| `7` | Đĩa quay (Logo BK) quay ngược chiều kim đồng hồ |
| `8` | Đĩa quay (Logo BK) quay cùng chiều kim đồng hồ |
| `R` / `r` | Reset thiết bị về trạng thái ban đầu |
| `S` / `s` | Bật/tắt chế độ tô màu trơn (Smooth Shading) |

### Điều Khiển Camera

| Phím | Hành động |
|------|-----------|
| `+` | Tăng khoảng cách camera đến trục Oy |
| `-` | Giảm khoảng cách camera đến trục Oy |
| `↑` | Tăng chiều cao camera |
| `↓` | Giảm chiều cao camera |
| `←` | Camera quay ngược chiều kim đồng hồ quanh trục Oy |
| `→` | Camera quay cùng chiều kim đồng hồ quanh trục Oy |

Camera sử dụng `gluLookAt`, được tính lại qua 3 biến:
- `camera_angle` — góc quay quanh trục Oy
- `camera_height` — chiều cao so với mặt phẳng xOz
- `camera_dis` — khoảng cách đến trục Oy

---

## Thang Điểm

### Xây Dựng Mô Hình (4.00 điểm)

| Bộ phận | Điểm |
|---------|------|
| Đế | 0.50 |
| Khung | 0.50 |
| Khớp vạn năng 1 | 0.50 |
| Khớp vạn năng 2 | 0.50 |
| Trục | 0.50 |
| Đĩa quay (Logo Bách Khoa) | 0.75 |
| Sàn nhà (20×20 viên gạch có hoa văn, vẽ tay, không dùng texture) | 0.75 |

### Điều Khiển Thiết Bị (3.75 điểm)

| Phím | Điểm |
|------|------|
| `1`, `2` — Điều khiển đế | 0.50 |
| `3`, `4` — Điều khiển Gimbal 1 | 0.50 |
| `5`, `6` — Điều khiển Gimbal 2 | 0.50 |
| `7`, `8` — Điều khiển Logo BK | 0.50 |
| `R`, `r` — Reset | 0.25 |
| `S`, `s` — Chuyển chế độ tô màu | 1.50 |

### Điều Khiển Camera (0.75 điểm)

| Phím | Điểm |
|------|------|
| `+` / `-` — Khoảng cách | 0.25 |
| `↑` / `↓` — Chiều cao | 0.25 |
| `←` / `→` — Xoay camera | 0.25 |

### Tô Màu 3D (1.50 điểm)

| Nội dung | Điểm |
|----------|------|
| Tô màu tạo cảm giác 3 chiều (Lighting & Shading) | 1.50 |

**Tổng: 10.00 điểm**

---

## Yêu Cầu Kỹ Thuật

- **Môi trường lập trình:** Code::Blocks 13.12
- **Thư viện:** OpenGL (chỉ dùng thư viện OpenGL, không dùng thư viện ngoài)
- **Cấm dùng:** `glutSolidCube`, `glutSolidSphere`, `glutSolidCylinder` và các hàm vẽ sẵn tương tự — phải tự thiết kế trong lớp `Mesh`
- **Tiêu đề chương trình:** Phải ghi rõ **Họ tên** và **MSSV** (thiếu bị trừ 1.0 điểm)

---

## Nộp Bài

- **Hạn nộp:** 23:00 ngày **16/05/2026**
- **Email nộp:** dhmt.bku@gmail.com
- **Chỉ nộp đúng 2 file (không nén):**
  1. `assignment-MSSV.cpp` — toàn bộ mã nguồn gộp vào 1 file duy nhất, tên viết thường
  2. `Phieuchamdiem-MSSV.xls` — bảng tự chấm điểm

> Mọi file nộp khác ngoài 2 file trên sẽ bị tự động xóa khi chấm bài.

---

## Lưu Ý Về Học Thuật

Bài tập lớn phải tự làm. Mọi trường hợp gian lận (copy code, giống nhau bất thường giữa các bài nộp) sẽ bị **điểm 0**, không có ngoại lệ.
