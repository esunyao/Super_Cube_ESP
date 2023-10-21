import _thread
import threading
import time

import serial
from serial import SerialException

print("\033[00m[\033[01m\033[36mDEBUG\033[00m] \033[36m\033[04mSuper_Cube\033[00m | \033[00m")

# 串口配置
ser = serial.Serial(
    port='COM5',  # 串口名称，根据您的情况更改
    baudrate=115200,  # 波特率
    timeout=1  # 超时时间（秒）
)

fl = 1
class myThread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.setDaemon(True)

    def run(self):
        global fl
        print("开始线程：" + self.name)
        while True:
            aasdfasdfasd = input("")
            print("input: "+aasdfasdfasd)
            if aasdfasdfasd.strip() == 'N':
                fl = 0
                print("input: set 0")
                continue
            if aasdfasdfasd.strip() == 'Y':
                fl = 1
                print("input: set 1")
                continue
            aasdfasdfasd += '\n'
            try:
                ser.write(aasdfasdfasd.encode())
            except Exception as e:
                print(e.args)


myThre = myThread()
myThre.start()
if ser.is_open:
    print("串口已打开")

while True:
    try:
        if not ser.is_open:
            print(ser.is_open)
            ser.open()
            time.sleep(5)
        while True:
            # 读取一行数据（以换行符 '\n' 为分隔符）
            data = ser.readline().decode('utf-8').strip()
            # 打印接收到的数据
            if data != '':
                print(data)

    except SerialException:
        try:
            if fl == 1:
                ser = serial.Serial(
                    port='COM5',  # 串口名称，根据您的情况更改
                    baudrate=115200,  # 波特率
                    timeout=1  # 超时时间（秒）
                )
        except:
            print("NoCon")
        time.sleep(5)
    except Exception as e:
        print(e.args)
        # time.sleep(5)