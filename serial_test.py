import socket		# ソケット通信用モジュール
import math		# 数学関係モジュール 
import serial		# シリアル通信用モジュール
import time     # 時間関数用モジュール 

ser = serial.Serial('COM6',115200,timeout = 0.1)	# シリアル通信開始


while True:
    time.sleep(2.0)
    # while True:
    #         ser.write(b'A') # 'A' == 0x41
    #         #print("arduinoにAを送りました2021.5.17")   
    #         time.sleep(0.1)
    #         c = ser.read()
    #         if c == b'A':
    #             print(c)
    #             break
    
    d=-100
    #hex_d=hex(d)
    #print(hex_d)

    if d>=0:
        

        while True:
                ser.write(b'+') # 'A' == 0x41
                #print("arduinoにAを送りました2021.5.17")   
                time.sleep(0.1)
                c = ser.read()
                if c == b'+':
                    print(c)
                    break

        bina_d=bytes([d])

        ser.write(bina_d) # 'A' == 0x41
        #print("arduinoにBを送りました2021.5.17")   
        time.sleep(0.1)

        c = ser.read()


        print("byte型:{0}".format(c))
        xx=int.from_bytes(c, 'big')
        print('byte→int:{0}'.format(xx))

    else:

        #マイナス符号をマイコンに送信
        #ser.write(b"-")

        while True:
                ser.write(b'-') # 'A' == 0x41
                #print("arduinoにAを送りました2021.5.17")   
                time.sleep(0.1)
                c = ser.read()
                if c == b'-':
                    print(c)
                    break


        #距離の絶対値をマイコンに送信
        bina_d=bytes([abs(d)])
        ser.write(bina_d) # 'A' == 0x41

        #マイコンから値を読み込む
        c = ser.read()

        #マイコンから読み込んできた値を表示する．
        print("byte型:{0}".format(c))
        xx=int.from_bytes(c, 'big')
        print('byte→int:{0}'.format(xx))

        


