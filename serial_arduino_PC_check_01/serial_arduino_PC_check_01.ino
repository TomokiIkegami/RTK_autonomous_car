////////////////////////////////////////////
//  arduinoとpythonのシリアル通信のチェック
//  2021.5.17
/////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);// arduino IDEモニタ用
  while (!Serial); 
  delay(500);
}


void loop() {
  while(1){
    if(Serial.available()>0){
      byte cc = (byte)Serial.read();
      //int cc = Serial.read();
      if(cc == 'A'){ // 0x41 == 'A'
        //Serial.print(cc);   
        Serial.write(cc);
        break;
      }
    }
  }
    //delay(1000);
  while(1){
    if(Serial.available()>0){
      byte cc = (byte)Serial.read();
      //if(cc == 'B'){ 
        Serial.write(cc);   
        break;
      //}
    }
  }
}    
