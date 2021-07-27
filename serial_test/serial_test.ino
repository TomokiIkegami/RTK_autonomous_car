/*マイコンに送信した文字をそのまま表示するプログラム*/

void setup() {
  
  Serial.begin(115200);// arduino IDEモニタ用
  Serial3.begin(115200);// arduino TeraTermモニタ用
  
  while (!Serial); 
  delay(500);
  
}

void loop() {

//Serial.println("Ikegami");
//Serial3.println("Hasebe");
int delta_l;
  
  while(1){
    if(Serial.available()>0){
      byte cc = (byte)Serial.read();
      //if(cc == 'B'){ 
        //Serial.write(cc);
        //Serial3.write(cc);
        delta_l=(char)cc; //経路からのずれ量[cm]
        //Serial3.println(delta_l);
        
        break;
      //}
    }
  }




}
