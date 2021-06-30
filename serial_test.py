import socket		# ソケット通信用モジュール
import math		# 数学関係モジュール 
import serial		# シリアル通信用モジュール
import time     # 時間関数用モジュール 

ser = serial.Serial('COM6',115200,timeout = 0.1)	# シリアル通信開始


while True:
    time.sleep(2.0)
    while True:
            ser.write(b'A') # 'A' == 0x41
            #print("arduinoにAを送りました2021.5.17")   
            time.sleep(0.1)
            c = ser.read()
            if c == b'A':
                print(c)
                break

    ser.write(b'B') # 'A' == 0x41
    #print("arduinoにBを送りました2021.5.17")   
    time.sleep(0.1)
    c = ser.read()
    print(c)
