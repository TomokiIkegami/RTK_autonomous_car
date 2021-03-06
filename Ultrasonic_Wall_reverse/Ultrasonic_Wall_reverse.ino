///////////////////////////////////////////////
// 池神・長谷部くん　作成プログラムを修正する 2021.5.25
//
// 超音波センサを、RTK測位と電子コンパスの代わりとして制御プログラムの確認をする
// 超音波センサは２つです 
//
// 斎藤くんプログラムを使用しました               //
//   atan(x) 答えの範囲 -pi/2 < atan(x) < pi/2 を使う
//   duble atan(double x)です　
///////////////////////////////////////////////
// モータードライバ TA8428K は前輪操舵のために使う
// バッテリーは６V使用する
// 後輪はモータードライバはリレー制御で前進のみ
// 前輪を左右に往復させロータリーエンコーダで回転角を検出する
// Rotary Encoder 読み込み：割り込み使用
// 初めにハンドルを左限界に回して enc_countA = 35　とする
// 反時計回り++   時計回り--
// 常にハンドル真ん中 enc_countA=17 で停止させる
//  2021.5.25
// *******************************************************/
#include<math.h>
#include <Wire.h> //I2Cライブラリ
#include <Stepper.h>
#define MOTOR_1  (32)  // orange
#define MOTOR_2  (33)  // yellow
#define MOTOR_3  (34)  // green
#define MOTOR_4  (35)  // blue

#define SW  (36)
#define MOTOR_STEPS (2048) // 出力軸1回転ステップ数:2048（2相磁励）
     // カタログ値です 1ステップ 0.17578度、10ステップ 1.7578度
// Relay pin //
#define RELAY1 (30)
#define RELAY2 (31)
#define RELAY3 (28)
#define RELAY4 (29)

/* Ultroasonic Sensor setting */
int trig1 = 3;// 出力ピン
int echo1 = 2;// 入力ピン
int trig2 = 45;// 出力ピン
int echo2 = 44;// 入力ピン
int Duration_1; // 継続時間
int Duration_2; // 継続時間
double Distance_1; // 距離 cm
double Distance_2; // 距離 cm
double Distance_c = 0.65; //超音波センサ1,2の中間距離[m]
double LL = 46.5; // 超音波センサ間距離 cm
double C_course; // 現在の方向保存用 degree 0-360 degree

const int pinA = 19;//ロータリーエンコーダA相 割り込み番号4
const int pinB = 18;//ロータリーエンコーダB相
volatile long enc_countA = 0;
const int STEER_IN1 = 7;   // 前輪操舵用
const int STEER_IN2 = 8;   // 前輪操舵用
const int STEER_IN1_B = 9;    // 前輪操舵用　予備
const int STEER_IN2_B = 10;   // 前輪操舵用　予備
const int duty0 = 0;

void compass_Rawdata();
void compass_Rawdata_Real();
double getDirection();
void stopMotor() ;

// ライブラリが想定している配線が異なるので2番、3番を入れ替える
Stepper myStepper(MOTOR_STEPS, MOTOR_1, MOTOR_3, MOTOR_2, MOTOR_4);

//pinAの割り込み処理
void enc_RisingPinA()
{
  if(( digitalRead(pinA)==0 && digitalRead(pinB)==1 )||( digitalRead(pinA)==1 && digitalRead(pinB)==0))
    --enc_countA; // ハンドルを上から見て時計回りで--
  else if(( digitalRead(pinA)==0 && digitalRead(pinB)==0 )||( digitalRead(pinA)==1 && digitalRead(pinB)==1))
    ++enc_countA; // ハンドルを上から見て反時計回りで++
}

int duty_s = 200; // 前輪操舵用モータduty比 (0~255)
unsigned long tm0;
int ii = 0;

const double One_step_angle = 360.0/MOTOR_STEPS;
const int User_steps = 57;// 回転してほしいステップ数 57で10度です
double v_angle = 0;
/*double cal_x = 0;
double cal_y = 0;
double cal_x_real = 0;
double cal_y_real = 0;
double ave_cal_x = 0;
double ave_cal_y = 0;
double cal_x_north = 0;
double cal_y_north = 0;
double rd = 0;
double rd_north = 0;*/
//double seihen = 0.16872;
//磁北の状態 旭川市春光台では西偏9度40分 = 9.667度 = 0.16872rad
// しかし電子コンパスは磁北を指さないので西偏値を引く必要は無い
//int count = 0;
int rot_dir = 0;
//int hensa = 0;
//ステッピングモータ回転方向 rot_dir=0 は上から見て反時計回り
//ステッピングモータ回転方向 rot_dir=1 は上から見て時計回り
//ステッピングモータ回転方向 rot_dir=4 は停止

////////////////////////
// 前輪操舵制御 //
////////////////////////  
/*int Steering_pid(double sita, byte dchar){
double dcv;
double kdcv=0.1;// フィードバックゲイン
int stop_flag=0;// 後輪モータ停止フラグ
int hang_ang;
///////////////////////////////////////////////////////////  
  if(dchar == 's') stop_flag=1;//停止：後輪モータを止める //if(limit_d <= 0.1):ser.write(b"s")
  else if(dchar == '0') dcv=-5.0;//1番大きく右折する //elif(d <= -0.8):ser.write(b"0")
  else if(dchar == '1') dcv=-4.0;//2番目に大きく右折する //  elif(-0.8 < d <= -0.6):ser.write(b"1")
  else if(dchar == '2') dcv=-3.0;//3番目に大きく右折する //  elif(-0.6 < d <= -0.4):ser.write(b"2")
  else if(dchar == '3') dcv=-2.0;//4番目に大きく右折する //  elif(-0.4 < d <= -0.2):ser.write(b"3")
  else if(dchar == '4') dcv=-1.0;//5番目に大きく右折する //  elif(-0.2 < d <= -0.1):ser.write(b"4")
  else if(dchar == '5') dcv=0.0;//ハンドル真っすぐ //  elif(-0.1 < d < 0.1):ser.write(b"5")
  else if(dchar == '6') dcv=1.0;//5番目に大きく左折する //  elif(0.1 <= d < 0.2):ser.write(b"6")
  else if(dchar == '7') dcv=2.0;//4番目に大きく左折する //  elif(0.2 <= d < 0.4):ser.write(b"7")
  else if(dchar == '8') dcv=3.0;//3番目に大きく左折する //  elif(0.4 <= d < 0.6):ser.write(b"8")
  else if(dchar == '9') dcv=4.0;//2番目に大きく左折する //  elif(0.6 <= d < 0.8):ser.write(b"9")
  else if(dchar == 'a') dcv=5.0;//1番大きく左折する //  elif(0.8 <= d):ser.write(b"a")
///////////////////////////////////////////////////////////////////////////

//int ii = 0;
double angle_num = 6; //この値を12→35→6に変更 ここの数字が大きくなると，蛇行が大きくなる。小さくし過ぎても操舵が出来なくなった。
double DR=17; //DR:Dual Rate ，舵角のこと。中心から片側に操舵したときに出力される、（ロータリーエンコーダの）パルスのカウント数。
  if( (sita/(PI/angle_num)*DR + kdcv*dcv) >= DR ) hang_ang = -DR;
  else if( (sita/(PI/angle_num)*DR + kdcv*dcv)<= -DR ) hang_ang = DR;
  else hang_ang = (int)( -sita/(PI/angle_num)*DR - kdcv*dcv);

  if(stop_flag==1){
    //Serial.println("Stop!");  // 後輪駆動モータ停止します
    digitalWrite(RELAY1,1);// 0 -> RELAY on , 1 -> RELAY off
    digitalWrite(RELAY2,1);

    return (1); 
  }

 while(1){
   if( enc_countA < hang_ang){
      //Serial.println("CCW"); // 反時計回り
      analogWrite(STEER_IN1, duty_s);
      analogWrite(STEER_IN2, duty0);
      Serial.print("1:enc_count,hang_ang, "); Serial.print(enc_countA);Serial.print(',');Serial.println(hang_ang);
      //ii = 0;
    }
    else if( enc_countA == hang_ang){
      //Serial.println("Stop!");
      analogWrite(STEER_IN1, 255);// ブレーキ
      analogWrite(STEER_IN2, 255);// ブレーキ
      Serial.print("2:enc_count,hang_ang, "); Serial.print(enc_countA);Serial.print(',');Serial.println(hang_ang);
      break; 
    }
    else if( enc_countA > hang_ang){
      //Serial.println("CW"); // 時計回り
      analogWrite(STEER_IN1, duty0);
      analogWrite(STEER_IN2, duty_s); 
      Serial.print("3:enc_count,hang_ang, "); Serial.print(enc_countA);Serial.print(',');Serial.println(hang_ang);
      //ii = 0;
    } 
  } 
  return (0);       
} */

int dest = 17;//ロータリーエンコーダとギヤ取り付け部がソフトなのでずれるが７にする 8から17.5に変更
double sita;
double delta_l;

void setup() {
  Serial.begin(115200);// arduino IDEモニタ用
  while (!Serial); 
  Serial.println("プログラム開始");

  // RELAY Setting //
  pinMode(RELAY1,OUTPUT);
  pinMode(RELAY2,OUTPUT);  
  digitalWrite(RELAY1,1);// 0 -> RELAY on , 1 -> RELAY off
  digitalWrite(RELAY2,1);    


  pinMode(RELAY3,OUTPUT);
  pinMode(RELAY4,OUTPUT);  
  digitalWrite(RELAY3,1);// 0 -> RELAY on , 1 -> RELAY off
  digitalWrite(RELAY4,1); 
  
  // Rotary Encoder Setting //
  pinMode(pinA,INPUT);
  pinMode(pinB,INPUT);  
  attachInterrupt(4, enc_RisingPinA, CHANGE); //両エッジで割り込み発生  

  // 前輪操舵PWM用
  // Timer2(8bit timer) -> pwm 10, 9
  TCCR2B = (TCCR2B & 0b11111000) | 0x07; //30.64 [Hz]
  // Timer4(16bit timer) -> pwm 8, 7, 6
  TCCR4B = (TCCR4B & 0b11111000) | 0x05; //30.64 [Hz]

  myStepper.setSpeed(5);//5rpm ステッピングモータの回転速度
  // 勘違いしないように！！　この回転速度で回り続けるのではなく
  // stepper.step(steps)で設定したsteps数をこの回転速度で回るのです
  // steps=10 ならば１０ステップをこの回転速度で回るのです 

 // 超音波センサ用端子 Timer3(16bit timer) -> pwm 5,3,2 
  pinMode(trig1,OUTPUT);
  pinMode(echo1,INPUT);
  // 超音波センサ用端子 timer5(16bit timer) -> pwm 46,45,44
  pinMode(trig2,OUTPUT);
  pinMode(echo2,INPUT);

  // 前輪操舵ピン設定
  pinMode(STEER_IN1, OUTPUT);
  pinMode(STEER_IN2, OUTPUT);
  analogWrite(STEER_IN1, 255);
  analogWrite(STEER_IN2, 255);  
  enc_countA = 35; //14から35に変更

  while(1){
    //Serial.println(enc_countA);
     
    if( enc_countA < dest){
      //Serial.println("CCW"); // 反時計回り
      analogWrite(STEER_IN1, duty_s);
      analogWrite(STEER_IN2, duty0);
      ii = 0;
    }
    else if( enc_countA == dest){
      //Serial.println("Stop!");
      analogWrite(STEER_IN1, 255);
      analogWrite(STEER_IN2, 255);
      if(ii == 0){
        tm0 = millis();
        ii = 1;
      }
      else {
        if((millis() - tm0) > 3000){
          enc_countA = 0;
          dest = 0;
          break; 
        }
      }
    }
    else if( enc_countA > dest){
      //Serial.println("CW"); // 時計回り
      analogWrite(STEER_IN1, duty0);
      analogWrite(STEER_IN2, duty_s); 
      ii = 0;
    }         
  } 

  //Serial.println(enc_countA);
  ii = 0;

  //ステッピングモーターを往復回転だけさせる
  while(rot_dir != 5){
    if(rot_dir == 0){// rot_dir=0 は上から見て反時計回り
      myStepper.step(User_steps);  // User_stepsだけ回す
      v_angle = v_angle + 10.0;
      // 電子コンパスのデータ取得
      //count++; 
      //compass_Rawdata();
      if(v_angle >= 360.0) rot_dir=1;   
    }
    else if(rot_dir == 1){// rot_dir=1 は上から見て時計回り
      myStepper.step(-1*User_steps);  // -1*User_stepsだけ回す
      v_angle = v_angle - 10.0; 
      // 電子コンパスのデータ取得
      //count++; 
      //compass_Rawdata(); 
      if(v_angle <= 0.0) rot_dir=4; 
    } 
    else if(rot_dir == 2){
      myStepper.step(User_steps);  // User_stepsだけ回す
      v_angle = v_angle + 10.0;
      // 電子コンパスのデータ取得
      //count++; 
      //compass_Rawdata();
      if(v_angle >= 360.0) rot_dir=3;   
    }
    else if(rot_dir == 3){
      myStepper.step(-1*User_steps);  // -1*User_stepsだけ回す
      v_angle = v_angle - 10.0; 
      // 電子コンパスのデータ取得
      //count++; 
      //compass_Rawdata(); 
      if(v_angle <= 0.0) rot_dir=4; 
    } 
    else if(rot_dir == 4){
      stopMotor(); 
      delay(500);
      rot_dir = 5;
      /*Serial.println("平均値");  
      ave_cal_x = cal_x/count;//楕円中心のｘ座標 
      Serial.println(ave_cal_x);
      ave_cal_y = cal_y/count;//楕円中心のｙ座標
      Serial.println(ave_cal_y);
 
      compass_Rawdata_Real();
      cal_x_north = cal_x_real -ave_cal_x; 
      cal_y_north = cal_y_real -ave_cal_y; 
      rd_north = getDirection(cal_x_north, cal_y_north);// y軸が角度の基準としている
      */
      // rd_north は台車を置いた位置での磁北の方向
    }
    delay(100);  
  }  
  int ult=0;
  
  for(ult=0;ult<10;ult++){
  // 超音波の出力終了
  digitalWrite(trig1,LOW);
  delayMicroseconds(1);
  // 超音波を出力
  digitalWrite(trig1,HIGH);
  delayMicroseconds(11);
  // 超音波を出力終了
  digitalWrite(trig1,LOW);
   // パルスを送信してから受信するまでの時間（マイクロ秒）
  Duration_1 = pulseIn(echo1,HIGH);
    
  // 超音波の出力終了
  digitalWrite(trig2,LOW);
  delayMicroseconds(1);
  // 超音波を出力
  digitalWrite(trig2,HIGH);
  delayMicroseconds(11);
  // 超音波を出力終了
  digitalWrite(trig2,LOW);
  // 出力した超音波が返って来る時間を計測
  Duration_2 = pulseIn(echo2,HIGH);

  if (Duration_1 > 0) {
    Distance_1 = Duration_1 / 2; // パルスを送信してから戻ってくるまでの時間 ÷ 2 = 対象物までの片道時間
    // Distance1 単位 cm
    Distance_1 = Distance_1 * 340.0 * 100.0 / 1000000.0; // ultrasonic speed is 340m/s = 34000cm/s = 0.034cm/us 
  }

  if (Duration_2 > 0) {
    Distance_2 = Duration_2 / 2; // パルスを送信してから戻ってくるまでの時間 ÷ 2 = 対象物までの片道時間
    // Distance2 単位 cm
    Distance_2 = Distance_2 * 340.0 * 100.0 / 1000000.0; // ultrasonic speed is 340m/s = 34000cm/s = 0.034cm/us 
  }
    
  // double atan(double x) = －π/2 から π/2
  // 方位 radian
  sita = atan( (Distance_1 - Distance_2) / LL );
  // 基準位置からの距離 [m] 
  delta_l = (Distance_1 + Distance_2)/2.0/100.0 - Distance_c;// [m]にした  
  Serial.print("prepare:"); Serial.print(ult);Serial.print(','); Serial.print("d:"); Serial.print(delta_l);Serial.print(','); Serial.print("θ:");Serial.println(sita*180/PI);
  
  delay(100);

  }
  
  

  // 後輪駆動モータ回転　前進します

  digitalWrite(RELAY3,0);// 0 -> RELAY on , 1 -> RELAY off
  digitalWrite(RELAY4,0);
  digitalWrite(RELAY1,0);// 0 -> RELAY on , 1 -> RELAY off
  digitalWrite(RELAY2,0);



  delay(400);
}

double U;   //制御量　ロータリエンコーダのパルス数
double Sum_y = 0.0; //積分要素
//フィードバック制御プログラム
int Feed_Back(double delta_rad, double delta_m){
  
// delta_rad [rad]です。  delta_m [m]です。
float k[3];     //フィードバックゲイン

double angle_num = 6; //この値を12→35→6に変更 ここの数字が大きくなると，蛇行が大きくなる。小さくし過ぎても操舵が出来なくなった。
double DR=17; //DR:Dual Rate ，舵角のこと。中心から片側に操舵したときに出力される、（ロータリーエンコーダの）パルスのカウント数。
 
   /////////////////////////////////////////////////////
  // 比例・積分制御                                   //
  // 進行方向の角度と壁からの平均距離の２つの比例制御 //
  ////////////////////////////////////////////////////// 
  double hen_rad = -delta_rad/(PI/angle_num)*DR;

  //k[0] = 1.0;// 進行方向の角度のゲイン
  k[0] = 2.0;// 進行方向の角度のゲイン
  k[1] = 25.0;// ２つの超音波センサの壁からの平均距離（アンテナ位置までの距離）のゲイン
  //k[1] = 0.0;
  k[2] = 0.00;  // ２つの超音波センサの壁からの平均距離（アンテナ位置までの距離）の積分のゲイン
 
  U =  (-k[0] * hen_rad  + k[1] * delta_m + k[2] * Sum_y); 
  Sum_y = delta_m + Sum_y;
  //delta_theta0 = delta_theta;
  
  if(U >= DR) U = DR;
  else if(U <= -DR) U = -DR;

  /*if(stop_flag==1){
    //Serial.println("Stop!");  // 後輪駆動モータ停止します
    digitalWrite(RELAY1,1);// 0 -> RELAY on , 1 -> RELAY off
    digitalWrite(RELAY2,1);
    return (1); 
  } */

  while(1){
   if( enc_countA < (int)U){
      //Serial.println("CCW"); // 反時計回り
      analogWrite(STEER_IN1, duty_s);
      analogWrite(STEER_IN2, duty0);
      //Serial.print("1:enc_count,U, "); Serial.print(enc_countA);Serial.print(',');Serial.println(U);
      //ii = 0;
    }
    else if( enc_countA == (int)U){
      //Serial.println("Stop!");
      analogWrite(STEER_IN1, 255);// ブレーキ
      analogWrite(STEER_IN2, 255);// ブレーキ
      //Serial.print("2:enc_count,U, "); Serial.print(enc_countA);Serial.print(',');Serial.println(U);
      break; 
    }
    else if( enc_countA > (int)U){
      //Serial.println("CW"); // 時計回り
      analogWrite(STEER_IN1, duty0);
      analogWrite(STEER_IN2, duty_s); 
      //Serial.print("3:enc_count,U, "); Serial.print(enc_countA);Serial.print(',');Serial.println(U);
      //ii = 0;
    } 
  } 
  return (0);     
}



void loop() {
  Ultrasonic();
  
  ////////////////////////
  // 前輪操舵制御 //
  ////////////////////////  
  int stop_f = Feed_Back(sita, delta_l);
  if(stop_f == 1) exit(0);
  delay(300); 
}

void Ultrasonic(void){
  // 超音波の出力終了
  digitalWrite(trig1,LOW);
  delayMicroseconds(1);
  // 超音波を出力
  digitalWrite(trig1,HIGH);
  delayMicroseconds(11);
  // 超音波を出力終了
  digitalWrite(trig1,LOW);
   // パルスを送信してから受信するまでの時間（マイクロ秒）
  Duration_1 = pulseIn(echo1,HIGH);
    
  // 超音波の出力終了
  digitalWrite(trig2,LOW);
  delayMicroseconds(1);
  // 超音波を出力
  digitalWrite(trig2,HIGH);
  delayMicroseconds(11);
  // 超音波を出力終了
  digitalWrite(trig2,LOW);
  // 出力した超音波が返って来る時間を計測
  Duration_2 = pulseIn(echo2,HIGH);

  if (Duration_1 > 0) {
    Distance_1 = Duration_1 / 2; // パルスを送信してから戻ってくるまでの時間 ÷ 2 = 対象物までの片道時間
    // Distance1 単位 cm
    Distance_1 = Distance_1 * 340.0 * 100.0 / 1000000.0; // ultrasonic speed is 340m/s = 34000cm/s = 0.034cm/us 
  }

  if (Duration_2 > 0) {
    Distance_2 = Duration_2 / 2; // パルスを送信してから戻ってくるまでの時間 ÷ 2 = 対象物までの片道時間
    // Distance2 単位 cm
    Distance_2 = Distance_2 * 340.0 * 100.0 / 1000000.0; // ultrasonic speed is 340m/s = 34000cm/s = 0.034cm/us 
  }
    
  // double atan(double x) = －π/2 から π/2
  // 方位 radian
  sita = atan( (Distance_1 - Distance_2) / LL );
  // 基準位置からの距離 [m] 
  delta_l = (Distance_1 + Distance_2)/2.0/100.0 - Distance_c;// [m]にした  
  Serial.print("d:"); Serial.print(delta_l);Serial.print(','); Serial.print("θ:");Serial.println(sita*180/PI);
}

// モーターへの電流を止める
void stopMotor() {
  digitalWrite(MOTOR_1, LOW);
  digitalWrite(MOTOR_2, LOW);
  digitalWrite(MOTOR_3, LOW);
  digitalWrite(MOTOR_4, LOW);
}
