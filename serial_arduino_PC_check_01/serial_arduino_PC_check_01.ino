/*/////////////////////////////////////////////////////////
PCから整数を読み取って，整数をPCに返すプログラムです．
受け取った整数は符号なし(0~255)なので，符号付(-127~128)の値に変換しています．
/////////////////////////////////////////////////////////*/

//#include<LiquidCrystal_I2C.h>
//LiquidCrystal_I2C lcd(0x27,16,2);

void setup() {
  Serial.begin(115200);// arduino IDEモニタ用
  while (!Serial); 
  delay(500);
//  lcd.init();
//  lcd.backlight();
//  lcd.setCursor(0,0);
//  lcd.print("Good morning!");
//  lcd.setCursor(0,1);
//  lcd.print("Are you happy?");
}


void loop() {
//  lcd.init(); //ディスプレイの表示を初期化
//  lcd.setCursor(0,0);
//  lcd.print("Receiving Dis");

  while(1){
    if(Serial.available()>0){
     byte cc = (byte)Serial.read(); //PCから，距離の値を読み込みます(符号なし0~255)
      char real_d=(char)cc; //値を符号付きに変換 (-128~127)←計算にはこの値を使用すればよい．
      //if(cc == 'B'){ 
        Serial.write((byte)cc); //値をPCに送信(符号なし0~255)
//        lcd.setCursor(0,1);
//        lcd.print("D=");
//        lcd.setCursor(2,1);
//        lcd.print((int)real_d);
        break;
      //}
    }
  }
}    
