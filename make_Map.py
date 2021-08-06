from os import name
import folium #地図上に座標をプロットできるモジュール

import re #北緯と東経を分離するために使用
import time     # 時間関数用モジュール

#座標（スタート，中間点1,中間点2...,ゴール）のテキストファイルを開く
fp = open('左折用の位置座標.txt', 'r', encoding='UTF-8')

#ファイルを文字列として読み込む
route_txt=fp.read()

fp.close()

#それぞれの行のデータをリストの一要素にする
route_data_set=route_txt.splitlines()
#print(route_data_set)
pattern="(.*),(.*),(.*)"

Point_NUM=len(route_data_set)
print("経路上の点の個数：{0}個".format(Point_NUM))
print("")


#ファイルから取得した座標を表示する
for i in range(Point_NUM):
    Point_info=re.search(pattern, route_data_set[i])
    Label=Point_info.group(1); LAT=Point_info.group(2); LNG=Point_info.group(3)

    #スタート地点でマップの初期表示を設定する
    if i==0:
        map = folium.Map(location=[LAT, LNG], zoom_start=18) #18が最大の拡大率

    #地図上に座標をプロットする
    folium.Marker(location=[LAT, LNG], popup=Label).add_to(map)
    print("{0}: 北緯 {1}, 東経 {2}".format(Label,LAT,LNG))

map.save("RTK_Car_Route.html")

print("")

print("経路上の点を地図にプロットしました。\n")