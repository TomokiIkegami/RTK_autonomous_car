////////////////////////////////////////////
// コンパスモジュールAE-BMX055 からの磁束密度生データを出力する
//  AE-BMX055             Arduino MEGA                             //
//    VCC                    +5V                                  //
//    GND                    GND                                  //
//    SDA                    D20(SDA)                              //
//    SCL                    D21(SCL)                              //
//   (JP6,JP4,JP5はショートした状態,JP4,JP5はI2Cプルアップ用）                              //
//
//　ステッピングモータを正転1回転（反時計回り）、逆転1回転（時計回り）してキャリブレーションもする
// キャリブレーションデータ取得　ばらつきを見る
// キャリブレーション後は磁北からのずれ角度をPCに送信する
// ステッピングモータは回さない
//   atan(x) 答えの範囲 -pi/2 < atan(x) < pi/2 を使う
//   duble atan(double x)です　
//*******************************************************
// モータードライバ TA8428K は前輪操舵のために使う
// バッテリーは６V使用する
// 後輪はモータードライバはリレー制御で前進のみ
// 前輪を左右に往復させロータリーエンコーダで回転角を検出する
// Rotary Encoder 読み込み：割り込み使用
// 初めにハンドルを左限界に回して enc_countA = 14　とする
// 反時計回り++   時計回り--
// 常にハンドル真ん中 enc_countA=7 で停止させる
//  2021.3.24
// *******************************************************/
#include <math.h>
#include<Wire.h>
#include <Stepper.h>
#define MOTOR_1  (32)  // orange
#define MOTOR_2  (33)  // yellow
#define MOTOR_3  (34)  // green
#define MOTOR_4  (35)  // blue

#define SW  (36)
#define MOTOR_STEPS (2048) // 出力軸1回転ステップ数:2048（2相磁励）
     // カタログ値です 1ステップ 0.17578度、10ステップ 1.7578度
// BMX055　磁気センサのI2Cアドレス
#define Addr_Mag 0x13   // (JP1,JP2,JP3 = Openの時)
// Relay pin //
#define RELAY1 (30)
#define RELAY2 (31)

// センサーの値を保存するグローバル関数
int   xMag  = 0;
int   yMag  = 0;
int   zMag  = 0;

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
double cal_x = 0;
double cal_y = 0;
double cal_x_real = 0;
double cal_y_real = 0;
double ave_cal_x = 0;
double ave_cal_y = 0;
double cal_x_north = 0;
double cal_y_north = 0;
double rd = 0;
double rd_north = 0;
//double seihen = 0.16872;
//磁北の状態 旭川市春光台では西偏9度40分 = 9.667度 = 0.16872rad
// しかし電子コンパスは磁北を指さないので西偏値を引く必要は無い
int count = 0;
int rot_dir = 0;
int hensa = 0;
//ステッピングモータ回転方向 rot_dir=0 は上から見て反時計回り
//ステッピングモータ回転方向 rot_dir=1 は上から見て時計回り
//ステッピングモータ回転方向 rot_dir=4 は停止

////////////////////////
// 前輪操舵制御 //
////////////////////////  
void Steering_pid(double sita){
int ii = 0;
int hang_ang;
<<<<<<< HEAD
double angle_num = 90; //舵角を12 から，35に変更

  if( sita/(PI/angle_num)*17 >= 17 ) hang_ang = -17;
  else if( sita/(PI/angle_num)*17<= -17 ) hang_ang = 17;
  else hang_ang = (int)( -sita/(PI/angle_num)*17 );
=======
double angle_num = 35.0; //舵角を12deg から，35degに変更

  if( sita/(PI/angle_num)*17.5 >= 17.5 ) hang_ang = -17.5;
  else if( sita/(PI/angle_num)*17.5 <= -17.5 ) hang_ang = 17.5;
  else hang_ang = (int)( -sita/(PI/angle_num)*17.5 );
>>>>>>> 85792274c96efcae57454724d576e6035174f90f

  while(1){
   if( enc_countA < hang_ang){
      //Serial.println("CCW"); // 反時計回り
      analogWrite(STEER_IN1, duty_s);
      analogWrite(STEER_IN2, duty0);
      Serial.print("1:enc_count,hang_ang, "); Serial.print(enc_countA);Serial.print(',');Serial.println(hang_ang);
      ii = 0;
    }
    else if( enc_countA == hang_ang){
      //Serial.println("Stop!");
      analogWrite(STEER_IN1, 255);// ブレーキ
      analogWrite(STEER_IN2, 255);// ブレーキ
      Serial.print("2:enc_count,hang_ang, "); Serial.print(enc_countA);Serial.print(',');Serial.println(hang_ang);
//      if(ii == 0){
//        tm0 = millis();
//        ii = 1;
//      }
//      else {
//        if((millis() - tm0) > 500){
//          //enc_countA = 0;
//          //dest = 0;
          break; 
//        }
//      }
    }
    else if( enc_countA > hang_ang){
      //Serial.println("CW"); // 時計回り
      analogWrite(STEER_IN1, duty0);
      analogWrite(STEER_IN2, duty_s); 
      Serial.print("3:enc_count,hang_ang, "); Serial.print(enc_countA);Serial.print(',');Serial.println(hang_ang);
      ii = 0;
    } 
  }        
} 

<<<<<<< HEAD
int dest = 18;//ロータリーエンコーダとギヤ取り付け部がソフトなのでずれるが７にする 8から18に変更
=======
int dest = 17.5;//ロータリーエンコーダとギヤ取り付け部がソフトなのでずれるが７にする 8から17.5に変更
>>>>>>> 85792274c96efcae57454724d576e6035174f90f
void setup() {
  Serial.begin(115200);// arduino IDEモニタ用
  while (!Serial); 

  // RELAY Setting //
  pinMode(RELAY1,OUTPUT);
  pinMode(RELAY2,OUTPUT);  
  digitalWrite(RELAY1,1);// 0 -> RELAY on , 1 -> RELAY off
  digitalWrite(RELAY2,1);    
  // Rotary Encoder Setting //
  pinMode(pinA,INPUT);
  pinMode(pinB,INPUT);  
  attachInterrupt(4, enc_RisingPinA, CHANGE); //両エッジで割り込み発生  

  TCCR2B = (TCCR2B & 0b11111000) | 0x07; //30.64 [Hz]
  TCCR4B = (TCCR4B & 0b11111000) | 0x05; //30.64 [Hz]
    
  myStepper.setSpeed(5);//5rpm ステッピングモータの回転速度
  // 勘違いしないように！！　この回転速度で回り続けるのではなく
  // stepper.step(steps)で設定したsteps数をこの回転速度で回るのです
  // steps=10 ならば１０ステップをこの回転速度で回るのです 

  // Wire(Arduino-I2C)の初期化
  Wire.begin();
  //BMX055 初期化
  BMX055_Init();
  delay(300); 

  pinMode(STEER_IN1, OUTPUT);
  pinMode(STEER_IN2, OUTPUT);
  analogWrite(STEER_IN1, 255);
  analogWrite(STEER_IN2, 255);  
  enc_countA = 35; //14から35に変更

  while(1){
    Serial.println(enc_countA);
     
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

  Serial.println(enc_countA);
  ii = 0;

  //ステッピングモーターを往復回転させ、電子コンパスのキャリブレーションをする
  //電子コンパスから出力される楕円データの中心点を求める
  // (0,0) にはならずずれてる
  while(rot_dir != 5){
    if(rot_dir == 0){// rot_dir=0 は上から見て反時計回り
      myStepper.step(User_steps);  // User_stepsだけ回す
      v_angle = v_angle + 10.0;
      // 電子コンパスのデータ取得
      count++; 
      compass_Rawdata();
      if(v_angle >= 360.0) rot_dir=1;   
    }
    else if(rot_dir == 1){// rot_dir=1 は上から見て時計回り
      myStepper.step(-1*User_steps);  // -1*User_stepsだけ回す
      v_angle = v_angle - 10.0; 
      // 電子コンパスのデータ取得
      count++; 
      compass_Rawdata(); 
      if(v_angle <= 0.0) rot_dir=4; 
    } 
    else if(rot_dir == 2){
      myStepper.step(User_steps);  // User_stepsだけ回す
      v_angle = v_angle + 10.0;
      // 電子コンパスのデータ取得
      count++; 
      compass_Rawdata();
      if(v_angle >= 360.0) rot_dir=3;   
    }
    else if(rot_dir == 3){
      myStepper.step(-1*User_steps);  // -1*User_stepsだけ回す
      v_angle = v_angle - 10.0; 
      // 電子コンパスのデータ取得
      count++; 
      compass_Rawdata(); 
      if(v_angle <= 0.0) rot_dir=4; 
    } 
    else if(rot_dir == 4){
      stopMotor(); 
      delay(500);
      rot_dir = 5;
      Serial.println("平均値");  
      ave_cal_x = cal_x/count;//楕円中心のｘ座標 
      Serial.println(ave_cal_x);
      ave_cal_y = cal_y/count;//楕円中心のｙ座標
      Serial.println(ave_cal_y);
 
      compass_Rawdata_Real();
      cal_x_north = cal_x_real -ave_cal_x; 
      cal_y_north = cal_y_real -ave_cal_y; 
      rd_north = getDirection(cal_x_north, cal_y_north);// y軸が角度の基準としている
      // rd_north は台車を置いた位置での磁北の方向
    }
    delay(100);  
  }
  
  // 後輪駆動モータ回転　前進します
<<<<<<< HEAD
  digitalWrite(RELAY1,0);// 0 -> RELAY on , 1 -> RELAY off
  digitalWrite(RELAY2,0);
=======
  //digitalWrite(RELAY1,0);// 0 -> RELAY on , 1 -> RELAY off
  //digitalWrite(RELAY2,0);
>>>>>>> 85792274c96efcae57454724d576e6035174f90f
  delay(400);
}

double sita;

void loop() {
  
  ////////////////////////
  // コンパスデータ受信です //
  ////////////////////////  
  compass_Rawdata_Real();
  cal_x_real = cal_x_real -ave_cal_x; 
  cal_y_real = cal_y_real -ave_cal_y;   
  rd = getDirection(cal_x_real, cal_y_real);
  // rd は現在の台車位置での磁北の方向
//  Serial.print(rd);Serial.print(',');Serial.print(rd_north);Serial.print(',');
//  Serial.print(cal_x_real);Serial.print(',');Serial.println(cal_y_real);
//  delay(100); 

  sita = rd_north - rd;
<<<<<<< HEAD
  if( sita < -PI ) sita = sita + 2*PI; //atanの計算をするときの値が-π~πなるように
=======
  if( sita < -PI ) sita = sita + 2*PI;
>>>>>>> 85792274c96efcae57454724d576e6035174f90f
  else if( sita > PI ) sita = sita - 2*PI;

  ////////////////////////
  // 前輪操舵制御 //
  ////////////////////////  
  Steering_pid(sita);
  delay(500);

} // end of loop

// コンパスの生データを取得する
void compass_Rawdata(){
  //BMX055 磁気の読み取り
  BMX055_Mag();
  Serial.print("xMag,yMag,zMag,rot_dir,count :  ");
  Serial.print(xMag); cal_x += (double)xMag;
  Serial.print(",");
  Serial.print(yMag); cal_y += (double)yMag;
  Serial.print(",");
  Serial.print(zMag);
  Serial.print(","); 
  Serial.print(rot_dir);
  Serial.print(","); 
  Serial.print(count);
  Serial.print(","); 
  Serial.println();
}

void compass_Rawdata_Real(){
  //BMX055 磁気の読み取り
  BMX055_Mag();
  cal_x_real= (double)xMag;
  cal_y_real= (double)yMag;
}

//** 角度を求める 出力は Radian **//
double getDirection(double x, double y){
  double dir;
  if(y == 0) dir = 0;
  else if(x == 0 && y > 0) dir = PI/2.0;
  else if(x == 0 && y < 0) dir = -PI/2.0;
  else {
    dir = atan(y/x);// 戻り値 -pi/2 から +pi/2
    if(x < 0 && y >= 0) dir = dir + PI;
    else if(x < 0 && y <= 0) dir = dir - PI;
  }
  // 結果として　-PI < dir < PI
  return dir;
}

// モーターへの電流を止める
void stopMotor() {
  digitalWrite(MOTOR_1, LOW);
  digitalWrite(MOTOR_2, LOW);
  digitalWrite(MOTOR_3, LOW);
  digitalWrite(MOTOR_4, LOW);
}

//=====================================================================================//
void BMX055_Init()
{
//BMX055 初期化
//------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x4B);  // Select Mag register
  Wire.write(0x83);  // Soft reset
  Wire.endTransmission();
  delay(100);
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x4C);  // Select Mag register
  Wire.write(0x00);  // Normal Mode, ODR = 10 Hz
  Wire.endTransmission();
 //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x4E);  // Select Mag register
  Wire.write(0x84);  // X, Y, Z-Axis enabled
  Wire.endTransmission();
 //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x51);  // Select Mag register
  Wire.write(0x04);  // No. of Repetitions for X-Y Axis = 9
  Wire.endTransmission();
 //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x52);  // Select Mag register
  Wire.write(0x0F);  // No. of Repetitions for Z-Axis = 15
  Wire.endTransmission();
}

//=====================================================================================//
void BMX055_Mag()
{
   //BMX055 磁気の読み取り
 unsigned int data[6];

  for (int i = 0; i < 6; i++)
  {
    // Start I2C Transmission
    Wire.beginTransmission(Addr_Mag);
    // Select data register
    Wire.write((66 + i));
    // Stop I2C Transmission
    Wire.endTransmission();

    // Request 1 byte of data
    Wire.requestFrom(Addr_Mag, 1);

    // Read 6 bytes of data
    // xMag lsb, xMag msb, yMag lsb, yMag msb, zMag lsb, zMag msb
    if (Wire.available() == 1)
    {
      data[i] = Wire.read();
    }
  }

  // Convert the data
  xMag = ((data[1] * 256) + (data[0] & 0xF8)) / 8;
  if (xMag > 4095)
  {
    xMag -= 8192;
  }
  yMag = ((data[3] * 256) + (data[2] & 0xF8)) / 8;
  if (yMag > 4095)
  {
    yMag -= 8192;
  }
  zMag = ((data[5] * 256) + (data[4] & 0xFE)) / 2;
  if (zMag > 16383)
  {
    zMag -= 32768;
  }
}
