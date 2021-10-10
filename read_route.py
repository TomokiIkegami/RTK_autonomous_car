import re #北緯と東経を分離するために使用
import time     # 時間関数用モジュール
import math     #数式計算用


def deg_rad(phi_deg):#　度からラジアンに変換 
    phi_rad=phi_deg*math.pi/180.0
    return phi_rad

def get_course(LAT_s,LNG_s,LAT_f,LNG_f):# 始点と終点の緯度・経度から、コースの数式係数を取得するための関数
    # LAT_s 緯度スタート, LNG_s 経度スタート
    # LAT_f 緯度終点, LNG_f 経度終点
    R=6378100.0	#　地球半径[m]
    dy=deg_rad(LAT_f - LAT_s)*R	# y方向変位（緯度）
    dx=deg_rad(LNG_f - LNG_s)*R	# x方向変位（経度）
    course=[ dy , -dx , 0.0 ]   # ax+by=0(原点を通るから)　右方向のずれが正
    return course


#座標（スタート，中間点1,中間点2...,ゴール）のテキストファイルを開く
fp = open('左折用の位置座標.txt', 'r', encoding='UTF-8')

#ファイルを文字列として読み込む
route_txt=fp.read()

#それぞれの行のデータをリストの一要素にする．
route_data_set=route_txt.splitlines()
print(route_data_set)
pattern="(.*),(.*),(.*)"

Point_NUM=len(route_data_set)
print("経路上の点の個数：{0}個".format(Point_NUM))
print("")
time.sleep(1)

#ファイルから取得した座標を表示する．
for i in range(Point_NUM):
    Point_info=re.search(pattern, route_data_set[i])
    Label=Point_info.group(1); LAT=Point_info.group(2); LNG=Point_info.group(3)
    print("{0}: 北緯{1}, 東経{2}".format(Label,LAT,LNG))

print("")

time.sleep(1)

#出発点と目的地の変更を繰り返す．
for i in range(Point_NUM-1):
    print("")
    Point_info_s=re.search(pattern, route_data_set[i])
    Point_info_f=re.search(pattern, route_data_set[i+1])
    Label_s=Point_info_s.group(1); LAT_s=Point_info_s.group(2); LNG_s=Point_info_s.group(3)
    Label_f=Point_info_f.group(1); LAT_f=Point_info_f.group(2); LNG_f=Point_info_f.group(3)   
    print("{0}から{1}に向かって走行中".format(Label_s,Label_f))
    print("{0}: 北緯{1}, 東経{2}".format(Label_s,LAT_s,LNG_s))
    print("{0}: 北緯{1}, 東経{2}".format(Label_f,LAT_f,LNG_f))



    #コースの数式係数a,bを計算
    course=get_course(float(LAT_s),float(LNG_s),float(LAT_f),float(LNG_f))	# コースの数式係数を取得
    a=course[0];b=course[1];c=course[2]		# ax+by+c=0    

    print("コースの数式係数: {0}x+{1}y+{2}=0".format(a,b,c))


    #車体の角度は初めの直線を基準にするので、初めの直線の数式係数a0,b0,c0を保存しておく
    if i==0:
        a0=a
        b0=b
        c0=c

    #最初の直線と、最後の直線がなす角度を計算（正接関数の加法定理を使用）

    tan_phi0=-a0/b0
    tan_phi=-a/b

    if i!=0:

        theta_rad=math.atan((tan_phi-tan_phi0)/(1+tan_phi*tan_phi0))
        theta_deg=theta_rad*180/math.pi
        print("基準直線との角度差 θ={0}".format(theta_deg))

    time.sleep(1)

#print(a0,b0,c0)

#最初の直線と、最後の直線がなす角度を計算（正接関数の加法定理を使用）
tan_phi0=-a0/b0
tan_phi=-a/b

theta_rad=math.atan((tan_phi-tan_phi0)/(1+tan_phi*tan_phi0))
theta_deg=theta_rad*180/math.pi
print("θ={0}".format(theta_deg))

