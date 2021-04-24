////////////////////////////////////////////
// コンパスモジュールAE-BMX055 からの磁束密度生データを出力する
//  AE-BMX055             Arduino MEGA                             //
//    VCC                    +5V                                  //
//    GND                    GND                                  //
//    SDA                    D20(SDA)                              //
//    SCL                    D21(SCL)                              //
//   (JP6,JP4,JP5はショートした状態,JP4,JP5はI2Cプルアップ用）                              //
//
//　ステッピングモータを正転1回転、逆転1回転してキャリブレーションもする
// キャリブレーションデータ取得　ばらつきを見る
// キャリブレーション後は磁北からのずれ角度をPCに送信する
// ステッピングモータは回さない
//   atan(x) 答えの範囲 -pi/2 < atan(x) < pi/2 を使う
//   duble atan(double x)です　 2021.3.24
/////////////////////////////////////
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

// センサーの値を保存するグローバル関数
int   xMag  = 0;
int   yMag  = 0;
int   zMag  = 0;

void compass_Rawdata();
void compass_Rawdata_Real();
double getDirection();
void stopMotor() ;

// ライブラリが想定している配線が異なるので2番、3番を入れ替える
Stepper myStepper(MOTOR_STEPS, MOTOR_1, MOTOR_3, MOTOR_2, MOTOR_4);

void setup() {
  Serial.begin(115200);// arduino IDEモニタ用
  while (!Serial); 
  myStepper.setSpeed(5);//5rpm ステッピングモータの回転速度
  // 勘違いしないように！！　この回転速度で回り続けるのではなく
  // stepper.step(steps)で設定したsteps数をこの回転速度で回るのです
  // steps=10 ならば１０ステップをこの回転速度で回るのです 

  // Wire(Arduino-I2C)の初期化
  Wire.begin();
  //BMX055 初期化
  BMX055_Init();
  delay(300); 
  
}

const double One_step_angle = 360.0/MOTOR_STEPS;
const int User_steps = 57;// 回転してほしいステップ数 57で10度です
double v_angle = 0;
int ii = 0;
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
//ステッピングモータ回転方向 rot_dir=2 は停止

void loop() {
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
      rot_dir = 5;
      Serial.println("平均値");  
      ave_cal_x = cal_x/count; 
      Serial.println(ave_cal_x);
      ave_cal_y = cal_y/count;
      Serial.println(ave_cal_y);
      stopMotor(); 
      delay(500);
      compass_Rawdata_Real();
      cal_x_north = cal_x_real -ave_cal_x; 
      cal_y_north = cal_y_real -ave_cal_y; 
      rd_north = getDirection(cal_x_north, cal_y_north);// y軸が角度の基準としている
      // rd_north は台車を置いた位置での磁北の方向
    }
    delay(100);  
  }

  int iik=0;
  while(rot_dir == 5){
    iik++;
    compass_Rawdata_Real();
    cal_x_real = cal_x_real -ave_cal_x; 
    cal_y_real = cal_y_real -ave_cal_y;   
    rd = getDirection(cal_x_real, cal_y_real);
    // rd は現在の台車位置での磁北の方向
    Serial.print(rd);Serial.print(',');Serial.print(rd_north);Serial.print(',');
    Serial.print(cal_x_real);Serial.print(',');Serial.println(cal_y_real);
    delay(100); 
    while(iik == 30);
  }
}

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

//** 角度を求める **//
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
