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

  byte cc;
  
  while(1){
    if(Serial.available()>0){
      cc = (byte)Serial.read();
      //int cc = Serial.read();
      if(cc == '-'){ // 0x41 == 'A'
        //Serial.print(cc);   
        Serial.write(cc);
        break;
      }else{
        Serial.write('+');
        break;
        }
    }
  }

  
    //delay(1000);
  while(1){
    if(Serial.available()>0){
      byte dd = (byte)Serial.read();
      if(cc == '-'){
        delay(1000);
        Serial.write('X');
        //byte dd = (byte)Serial.read();
        //Serial.write(dd);
        break;
        
        }else{
        Serial.write(dd);
        break;
          }

      //Serial.write(dd);
      //break;
      
      //if(cc == 'B'){ 
        //Serial.write(cc);   
        //break;
      //}
    }
  }
}    
