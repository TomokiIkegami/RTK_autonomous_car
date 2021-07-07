import socket		# ソケット通信用モジュール
import math		# 数学関係モジュール 
import serial		# シリアル通信用モジュール
import time     # 時間関数用モジュール 

ser = serial.Serial('COM6',115200,timeout = 0.1)	# シリアル通信開始


while True:
    time.sleep(1.0)
    # while True:
    #         ser.write(b'A') # 'A' == 0x41
    #         #print("arduinoにAを送りました2021.5.17")   
    #         time.sleep(0.1)
    #         c = ser.read()
    #         if c == b'A':
    #             print(c)
    #             break

    

    d=-128 #ずれ量の値

    #ずれ量が負の場合は，符号なしの値に変換
    if d<0: 
        d=256-abs(d)



    #hex_d=hex(d)
    #print(hex_d)
    bina_d=bytes([d]) #距離の値をバイト型に変換

    ser.write(bina_d) # 'A' == 0x41 #距離の値をバイト型でマイコンに送信
    #print("arduinoにBを送りました2021.5.17")   
    time.sleep(0.1)
    c = ser.read() #マイコンから値を読み込む

    #マイコンから返ってきた値を表示している．
    print("byte型:{0}".format(c))   
    xx=int.from_bytes(c, 'big')
    print('byte→int:{0}'.format(xx))