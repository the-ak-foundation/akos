# Thuật toán sinh map BSP trong 3D Map

## 1. BSP là gì?

`BSP` là viết tắt của **Binary Space Partitioning**, nghĩa là chia một vùng
không gian thành hai phần, sau đó tiếp tục chia từng phần nhỏ thành hai phần
khác.

Trong project Game Hub, BSP được dùng để tạo một bản đồ gồm nhiều phòng và
cửa nối phòng. Map không được lưu cố định trong Flash. Mỗi lần mở `3D MAP`,
map được sinh lại từ một giá trị `seed`.

Code liên quan nằm trong:

- `map3d_generate()`: khởi tạo map và tường bao.
- `map3d_split_room()`: chia phòng bằng BSP.
- `map3d_set_wall()`: đặt hoặc xóa một ô tường.
- `map3d_random()`: bộ sinh số giả ngẫu nhiên `xorshift32`.

## 2. Cấu trúc map

Map có kích thước:

```text
16 cột × 16 hàng = 256 ô
```

Mỗi ô chỉ cần một bit:

```text
0 = sàn, có thể đi qua
1 = tường, không thể đi qua
```

Vì vậy toàn bộ map chỉ chiếm:

```text
256 bit / 8 = 32 byte RAM
```

Mỗi ô `(x, y)` được chuyển thành chỉ số tuyến tính:

```c
index = y * MAP3D_WIDTH + x;
```

Byte và bit chứa ô đó:

```c
byte = index >> 3;
bit  = index & 7;
```

## 3. Bước khởi tạo

Ban đầu tất cả ô bên trong là sàn. Bốn cạnh ngoài được đặt thành tường để
người chơi không thể đi ra khỏi map:

```text
################
#              #
#              #
#              #
#              #
#              #
#              #
#              #
#              #
#              #
#              #
#              #
#              #
#              #
#              #
################
```

Ký hiệu:

- `#`: tường.
- Khoảng trắng: sàn.

Vùng được đưa vào BSP là phần bên trong:

```c
map3d_split_room(game, 1, 1, 14, 14);
```

## 4. Chia một phòng

Mỗi lời gọi `map3d_split_room()` nhận vào:

```text
x, y          tọa độ góc trên bên trái
width         chiều rộng vùng
height        chiều cao vùng
```

Kích thước phòng tối đa hiện tại là 7 ô:

```c
maximum_room = 7;
```

Nếu cả chiều rộng và chiều cao đều không lớn hơn 7, thuật toán dừng chia:

```text
width <= 7 và height <= 7 → phòng hoàn thành
```

Nếu phòng còn quá lớn, thuật toán chọn hướng chia.

### 4.1. Phòng cao hơn chiều rộng

Nếu `height > width`, một bức tường ngang được tạo:

```text
Trước khi chia:

+--------------+
|              |
|              |
|              |
|              |
|              |
|              |
+--------------+

Sau khi chia:

+--------------+
|   phòng A    |
|              |
|##############|
|   phòng B    |
|              |
+--------------+
```

Sau đó một ô ngẫu nhiên trên tường được xóa để làm cửa:

```text
+--------------+
|   phòng A    |
|              |
|#####  #######|
|   phòng B    |
|              |
+--------------+
       ^^
       cửa
```

Hai vùng A và B tiếp tục được đưa vào `map3d_split_room()`.

### 4.2. Phòng rộng hơn hoặc bằng chiều cao

Một bức tường dọc được tạo:

```text
+------+-------+
|      #       |
|      #       |
| phòng# phòng |
|   A      B   |  <- cửa
|      #       |
|      #       |
+------+-------+
```

Tương tự, một ô trên tường được xóa để tạo cửa. Hai vùng bên trái và bên phải
tiếp tục được chia.

## 5. Tại sao gọi là "Binary"?

Mỗi phòng chỉ sinh ra tối đa hai phòng con:

```text
Map ban đầu
├── Phòng A
│   ├── Phòng A1
│   └── Phòng A2
└── Phòng B
    ├── Phòng B1
    └── Phòng B2
```

Cấu trúc chia này giống một cây nhị phân. Tuy nhiên project không cần lưu cây
BSP trong RAM. Tường được ghi trực tiếp vào map rồi hàm tiếp tục đệ quy.

## 6. Pseudocode

Phiên bản rút gọn của thuật toán hiện tại:

```text
function split_room(x, y, width, height):
    if width <= 7 and height <= 7:
        return

    if height > width:
        chọn ngẫu nhiên vị trí split_y
        vẽ một tường ngang tại split_y
        mở một cửa ngẫu nhiên trên tường

        split_room(vùng phía trên)
        split_room(vùng phía dưới)
    else:
        chọn ngẫu nhiên vị trí split_x
        vẽ một tường dọc tại split_x
        mở một cửa ngẫu nhiên trên tường

        split_room(vùng bên trái)
        split_room(vùng bên phải)
```

## 7. Vai trò của seed

`seed` khởi tạo trạng thái của `xorshift32`. Cùng một seed luôn tạo cùng một
map:

```text
seed 1234 → map A
seed 1234 → map A
seed 5678 → map B
```

Game Manager truyền thời gian hoạt động hiện tại làm seed. Vì thời điểm mở
game thường khác nhau nên map được tạo cũng khác nhau.

Đây là số giả ngẫu nhiên, không phải ngẫu nhiên tuyệt đối. Đặc điểm này hữu
ích cho debug vì có thể dùng lại seed để tái tạo chính xác một map bị lỗi.

## 8. Vị trí xuất phát

Sau khi BSP hoàn thành, người chơi được đặt tại ô:

```text
(MAP3D_WIDTH / 2, MAP3D_HEIGHT / 2) = (8, 8)
```

Một đường hình dấu cộng quanh vị trí này được mở:

```text
   .
  ...
   .
```

Trong đó dấu `.` là ô sàn. Việc này bảo đảm:

- Người chơi không sinh ra bên trong tường BSP.
- Không bị kẹt tại giao điểm của hai bức tường.
- Có thể rời vị trí xuất phát theo bốn hướng.

## 9. BSP bảo đảm điều gì?

Thuật toán hiện tại bảo đảm:

- Map luôn có tường bao.
- Mỗi lần chia đều tạo một cửa nối hai phòng con.
- Các phòng được tạo bởi cùng một nhánh BSP luôn kết nối với nhau.
- Không cần lưu danh sách phòng hoặc cây BSP trong RAM.
- Chi phí RAM của map chỉ là 32 byte.

BSP hiện tại không tạo:

- Kẻ địch hoặc vật phẩm.
- Điểm kết thúc map.
- Cửa có trạng thái đóng/mở.
- Đường đi có trọng số.
- Bảo đảm khoảng cách tối thiểu từ điểm đầu tới một điểm đích.

Các phần này có thể được bổ sung sau khi map BSP đã hoàn thành.

## 10. Ví dụ một map hoàn chỉnh

```text
################
#      #       #
#      #       #
#              #
#      #       #
### ####    # ##
#       ## ### #
#       #      #
#       # S    #
#       #      #
#       #   # ##
###### ######  #
#    #         #
#              #
#    #         #
################
```

`S` là vị trí xuất phát ở gần chính giữa map. Những khoảng trống trên các
bức tường là cửa được BSP tạo ra để nối các phòng.

