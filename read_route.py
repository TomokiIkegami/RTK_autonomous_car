import re #北緯と東経を分離するために使用
import time     # 時間関数用モジュール

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
    time.sleep(1)
