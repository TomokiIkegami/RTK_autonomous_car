###########################################################################
#### ノートPCで動作するRTK台車制御用Pythonプログラム 浅野くん作成プログラム ####
####   直線移動　出発点と目的地の2点だけを入力する    2021.4.28 大柏変更   ####
####   出発点と目的地を結ぶ直線からのずれをシリアルから出力する　
####   タブレットでシリアル出力を受信し正しく、文字が出力されているか確認する
# ##########################################################################
import socket		# ソケット通信用モジュール
import math		# 数学関係モジュール 
import serial		# シリアル通信用モジュール
import time     # 時間関数用モジュール 

####### 出発点と目的地　########################################
isStartNorth = 43.812389700  # 出発点　北緯　ddd.ddddd表記です
isStartEast = 142.352324100  # 出発点　東経　ddd.ddddd表記です
isDestiNorth = 43.812274230  # 目的地　北緯　ddd.ddddd表記です
isDestiEast = 142.352165560  # 目的地　東経　ddd.ddddd表記です
###############################################################

def check_fix():    # fixしているか確認
    print("\n現在fixしているか確認しています...")
    while True:
        codelist_check=get_codelist()	# $GNGGAセンテンスを取得 
        if int(codelist_check[6])==4:
            print("現在fixしています。\n")
            # time.sleep(1) # 1秒処理停止
            break

def min_deg(phi_min):#　分から度に変換
    # 緯度、経度共に dddmm.mmmm表記なので100で割って分をすべて小数点以下にする
    f,i=math.modf(phi_min/100.0)  # i 整数部（ddd度） , f 小数部（.mmmmmm)
    phi_deg=i+f*100.0/60.0 # ddd.dddd度表記になる
    return phi_deg
    
def deg_rad(phi_deg):#　度からラジアンに変換 
    phi_rad=phi_deg*math.pi/180.0
    return phi_rad

def get_course(LAT_s,LNG_s,LAT_f,LNG_f):# コースの数式係数を取得 
    # LAT_s 緯度スタート, LNG_s 経度スタート
    # LAT_f 緯度終点, LNG_f 経度終点
    R=6378100.0	#　地球半径[m]
    dy=deg_rad(LAT_f - LAT_s)*R	# y方向変位（緯度）
    dx=deg_rad(LNG_f - LNG_s)*R	# x方向変位（経度）
    course=[ dy , -dx , 0.0 ]   # ax+by=0(原点を通るから)　右方向のずれが正
    return course

def get_codelist():	#　$GNGGAセンテンスを取得 
    while True:
        j=0	# $の場所の定義
        data1 = str(s.recv(300)) # データ 300バイト分取得
        while True:
            data2 = str(s.recv(300))
            data1 += data2	# データを追加する
            if len(data2) < 300: # １パケット分を取得したらループ抜ける
               break 

        j=data1.find("$GNGGA")	# 取得データから$GNGGAの頭文字$の要素番号を取得
        code=data1[j:j+88]		# $GNGGAの頭文字要素数から88文字目までを読み込む
        if(code.count("$")==1)and(code.count("'")==0):
            # 欠陥がなければちょうど88文字で $マークが1個
            # なぜか'が入るときがあり，エラーが出るから
            codelist=code.split(',')	# $GNGGAセンテンスを','区切りでリスト化
            break
    return codelist # コードのリストを返す

def get_d(a,b,c,LAT,LNG,LAT_s,LNG_s): #経路からのずれの量を取得
    R=6378100.0  #地球半径[m]
    y=deg_rad(LAT - LAT_s)*R  #現在地と出発地のy方向変位
    x=deg_rad(LNG - LNG_s)*R  #現在地と出発地のx方向変位
    d = (a*x + b*y + 0.0) / math.sqrt(a*a + b*b) #経路からのずれ量
    return d #経路からのずれ量を返す

def get_limit_d(LAT,LNG,LAT_f,LNG_f): #経路からのずれの量を取得
    R=6378100.0  #地球半径[m]
    y=deg_rad(LAT - LAT_f)*R  #現在地と出発地のy方向変位
    x=deg_rad(LNG - LNG_f)*R  #現在地と出発地のx方向変位
    limit_d = math.sqrt(x*x + y*y) #目的地からの距離
    return limit_d #目的地からの距離  

def get_edge(LAT1,LNG1,LAT2,LNG2):	# ２地点の距離を取得
    R=6373100.  #地球半径[m]
    dy=deg_rad(LAT2 - LAT1)*R
    dx=deg_rad(LNG2 - LNG1)*R
    edge=math.sqrt((dx)*(dx) + (dy)*(dy))
    return edge

limit_d=0.2			# ずれ量許容範囲[m]
limit_LL=0.0000004		#　緯度経度許容範囲[deg]

ser = serial.Serial('COM8',115200,timeout = 0.1)	# シリアル通信開始

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:	# ソケット通信開始
    s.connect(('localhost',50000))		# IPアドレス、ポート番号指定
    check_fix()					# fixしているか確認

    #input("計測を開始するときはEnterを押してください。")
    print("計測開始")
    print("出発点から動きます")
    
    LAT_s=isStartNorth; LNG_s=isStartEast # 出発点
    LAT_f=isDestiNorth; LNG_f=isDestiEast # 目的地

    course=get_course(LAT_s,LNG_s,LAT_f,LNG_f)	# コースの数式係数を取得
    a=course[0];b=course[1];c=course[2]		# ax+by+c=0

    starttime=time.time()
    			
    while True:
        codelist=get_codelist()
        Time=round((float(codelist[1])/10000+9),4)	# 日本時間
        NS=codelist[3]			#  北緯南緯
        LAT=min_deg(float(codelist[2]))	#　緯度の単位を度に変換
        EW=codelist[5]			# 東経西経
        LNG=min_deg(float(codelist[4]))	# 経度の単位を度に変換
        FF=int(codelist[6])		# fixかfloatか : fixは 4, floatは 5を判断

        if FF==4:				# fixは 4, floatは 5を判断
            d=get_d(a,b,c,LAT,LNG,LAT_s,LNG_s)	# 経路からのずれの量[m]を取得
            edge=get_edge(LAT_s,LNG_s,LAT,LNG)	# 出発地と現在地の距離を取得
            limit_d=get_limit_d(LAT,LNG,LAT_f,LNG_f) # 目的地からの距離[m]を取得  
            currenttime=time.time()
            #print("arduinoにAを送るよ2021.5.17") 
            if(currenttime-starttime > 1.0):	#　2秒毎にシリアル通信
                starttime=currenttime
                #print("arduinoまできた2021.5.17") 
                while True:
                    ser.write(b'A') # 'A' == 0x41
                    #print("arduinoにAを送りました2021.5.17")   
                    time.sleep(0.1)
                    c = ser.read()
                    if c == b'A':
                        print(c)
                        break

                if(limit_d <= 0.1):ser.write(b"s") #　停止
                elif(d <= -0.8):ser.write(b"0")    #　右折 
                elif(-0.8 < d <= -0.6):ser.write(b"1")  #  右折する
                elif(-0.6 < d <= -0.4):ser.write(b"2")  #  右折する
                elif(-0.4 < d <= -0.2):ser.write(b"3")  #  右折する
                elif(-0.2 < d <= -0.1):ser.write(b"4")  #  右折する
                elif(-0.1 < d < 0.1):ser.write(b"5")	#　ハンドル真っ直ぐ
                elif(0.1 <= d < 0.2):ser.write(b"6")    #  左折する
                elif(0.2 <= d < 0.4):ser.write(b"7")    #  左折する
                elif(0.4 <= d < 0.6):ser.write(b"8")    #  左折する
                elif(0.6 <= d < 0.8):ser.write(b"9")    #  左折する
                elif(0.8 <= d):ser.write(b"a")        #  左折する
                
        else:
            print("float")	#　Fix解以外をまとめてFloat解とする
            continue
 
        c = ser.read()
        if( c==b'0' or c==b'1' or c==b'2' or c==b'3' or c==b'4' or c==b'5' or c==b'6' or c==b'7' or c==b'8' or c==b'9' or c==b'a' or c==b's'):
            print(c) 
            print("fix,",Time,",",NS,LAT,"[deg],",EW,LNG,"[deg],d=",round(d,4),"[m]") # 緯度経度出力


