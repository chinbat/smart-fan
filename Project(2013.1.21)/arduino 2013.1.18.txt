#include <Servo.h>
#include <stdio.h>
#include <string.h>

Servo myservo;
Servo my2servo;
char c;

void setup() 
{ 
  myservo.attach(9);
  myservo.write(85);
  my2servo.attach(10);
  my2servo.write(110);
  Serial.begin(9600);  // arduinoのシリアルポートをオープン
} 
void loop() 
{ 
  int i = 0;
  int x,y,z;  //サーボモータへ代入する値
  int hantei = 0;  // _ が来たことを示す
  int point = 0;    //どの配列に入れるかを判断するための変数
  char x1[20] = {0};  //x軸の値をchar型でとる
  char y1[20] = {0};  //y軸の値をchar型でとる
  char z1[20] = {0};  //z軸の値をchar型でとる
  
  while (1) {
    if (Serial.available()) { // バッファにシリアルデータがあるかどうかをチェック
      c = Serial.read(); // シリアルデータをバイト単位で読み込む      
      if (c == 'F')
        break; // 文字列の終わりは\0で判断
      if (c == '_'){ // x軸の数字の終わりの判断のために使う
        point++;
        hantei = 1;
      }

      if(point == 0)
        x1[i] = c;
      
      if(point == 1 && hantei == 0)
        y1[i] = c;     
      
      if(point == 2 && hantei == 0)
        z1[i] = c;
        
      i++;
      
      if(hantei == 1)
        i = 0;
        
      hantei = 0;   
    }
  }
    x = atoi(x1);
    y = atoi(y1);
    z = atoi(z1);
  if(x==200 && y==200 && z==200){
    myservo.write(85);
    my2servo.write(110);
    analogWrite(11, 0);
  }
  else{
    myservo.write (x);
    my2servo.write(y);
    analogWrite(11, z);
  }
  Serial.flush();	//バッファのクリア   
}