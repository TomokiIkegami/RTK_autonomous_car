/*マイコンに送信した文字をそのまま表示するプログラム*/

void setup() {
  
  Serial.begin(115200);// arduino IDEモニタ用
  while (!Serial); 
  delay(500);
  
}

void loop() {
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
