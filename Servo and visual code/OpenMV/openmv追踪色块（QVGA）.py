import sensor, image, time, math, pyb
from pyb import UART

uart = pyb.UART(3, 115200)  # 初始化串口

# 初始化摄像头
sensor.reset()
sensor.set_pixformat(sensor.RGB565)  # 设置为RGB565模式，便于颜色识别
sensor.set_framesize(sensor.QVGA)    # 设置分辨率 320x240
sensor.skip_frames(time=2000)        # 跳过前几帧，等待摄像头稳定

# 数据打包函数
def pack_data(x, y):
    # 限制坐标范围
    x = max(0, min(x, 319))  # QVGA 分辨率为 320x240
    y = max(0, min(y, 239))

    # 打包为 2 字节数据
    data = bytearray([
        0x2C,                     # 起始字节
        (x >> 8) & 0xFF,          # X 高字节
        x & 0xFF,                 # X 低字节
        (y >> 8) & 0xFF,          # Y 高字节
        y & 0xFF,                 # Y 低字节
        0x5B                      # 结束字节
    ])
    return data

# 寻找最大色块函数
def find_max_blob(blobs):
    max_area = 0
    max_blob = None
    for blob in blobs:
        if blob.area() > max_area:
            max_area = blob.area()
            max_blob = blob
    return max_blob

clock = time.clock()

# 红色阈值 (L_min, L_max, A_min, A_max, B_min, B_max)
# 根据实际情况调整这些值以获得最佳检测效果
threshold = (15, 75, 20, 90, 0, 60)

# 最小色块大小阈值
min_blob_area = 100

while True:
    clock.tick()

    # 捕获图像
    img = sensor.snapshot()

    # 检测红色色块
    blobs = img.find_blobs([threshold], area_threshold=min_blob_area, pixels_threshold=50)

    if blobs:
        # 寻找最大色块
        max_blob = find_max_blob(blobs)
        if max_blob:
            # 绘制最大色块的外接矩形和中心十字
            img.draw_rectangle(max_blob.rect(), color=(255, 0, 0))  # 红色边框
            img.draw_cross(max_blob.cx(), max_blob.cy(), color=(0, 255, 0))  # 绿色十字

            # 获取中心坐标
            center_x = max_blob.cx()
            center_y = max_blob.cy()

            # 打包数据并发送
            try:
                data = pack_data(center_x, center_y)
                uart.write(data)
                print("Sent data: X={}, Y={}".format(center_x, center_y))
            except Exception as e:
                print("UART write error:", e)
    else:
        # 如果未检测到Blob，发送默认数据
        try:
            default_data = bytearray([0x2C, 0, 0, 0, 0, 0x5B])
            uart.write(default_data)
            print("Sent default data:", default_data)
        except Exception as e:
            print("UART write error:", e)

    # 打印帧率
    print("FPS: {}".format(clock.fps()))
