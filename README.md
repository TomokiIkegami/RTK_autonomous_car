# RTK_autonomous_car
このレポジトリは、卒業研究のプログラム共有に使用しているものです。本研究では、RTK測位を用いた自律走行教材用マイコンカーの開発を目指しています。

# Research Outline
下の図が本研究で目指す自律走行(autonomous driving)の概要になります。RTK測位とはGPSのような位置測位技術のことです。GPSよりも高精度（ GPSは数[m]、RTK測位では数[cm]）で位置情報を取得出来るのがRTK測位の特長です。<br><br> 
![research_outline](./images/research_outline.png)

私たちが目指す自律走行マイコンカーの姿は、RTK測位を用いて直線経路上を走行することです。このマイコンカーのことを以降 "RTK台車(daisha)" と呼ぶことにします。


# What is "RTK daisha" ?
"RTK台車" は私たち研究チームが開発している自律走行マイコンカーの名前です。<br>
RTK台車には自律走行に必要なセンサとして、RTK用アンテナ(antenna)と電子コンパス(compass)を搭載しています。（下図参照）<br><br>
![reserch_outline](./images/RTK_daisha.png)


この2つのセンサ情報を用いることで、直線経路の自律走行を実現します。RTKアンテナから経路からのずれ量（lateral distance）を、電子コンパスから車体角度(angle difference)を計算します。（下図参照）
![reserch_outline](./images/what_is_distance_and_angle.png)

# Installation & Usage
RTK台車のセットアップ方法や動かし方については、以下のリンクを参考にして行ってください。

1. [RTK台車の作り方（ラジコンカーの改造方法）](https://asahikawa-nct.ac.jp/ts/systems/okashiwa/rtk_cart/)
2. [走行のプログラム導入手順など](http://onshape.thick.jp/onshape/108/)

# Author
* Tomoki Ikegami, Wataru Hasebe <br>
National Institute of Technology, Asahikawa Collage (graduated in 2022)
