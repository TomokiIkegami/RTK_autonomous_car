/////////////////////////////////////////////////
// Rotary Encoder による操舵角チェックプログラム
// 手動で前輪を傾けて限界角度を調べる
//
// 実験結果　右向き限界から左向き限界まで１４パルス
// 　反時計回り＋＋　　時計回り―
// 真ん中で7パルス
//     2021.3.18
/////////////////////////////////////////////////
const int pinA = 19;//A相 割り込み番号4
const int pinB = 18;//B相
volatile long enc_countA = 0;

void setup()
{
  Serial.begin(115200);
  while(!Serial);
    
  pinMode(pinA,INPUT);
  pinMode(pinB,INPUT);  
  attachInterrupt(4, enc_RisingPinA, CHANGE);
  //RISING:pinAの信号がLOWからHIGHに変わったときに割り込み発生
  //FALLING:pinAの信号がHIGHからLOWに変わったときに割り込み発生
  //CHANGE:pinAの信号が両エッジで割り込み発生
  //LOW:pinAの信号がLOWレベルで割り込み発生
}

void loop()
{
  //シリアルコンソールに現在の値を出力する
  Serial.println(enc_countA);
  //delayMicroseconds(10);
}

//pinAの割り込み処理
void enc_RisingPinA()
{
  if(( digitalRead(pinA)==0 && digitalRead(pinB)==1 )||( digitalRead(pinA)==1 && digitalRead(pinB)==0))
    --enc_countA; // ハンドルを上から見て時計回りで--
  else if(( digitalRead(pinA)==0 && digitalRead(pinB)==0 )||( digitalRead(pinA)==1 && digitalRead(pinB)==1))
    ++enc_countA; // ハンドルを上から見て反時計回りで++
}
