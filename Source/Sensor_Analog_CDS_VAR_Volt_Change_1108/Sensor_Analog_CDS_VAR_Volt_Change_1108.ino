#include <TimerOne.h>

volatile bool timer1Flag = false;

void setup()
{
  Serial.begin(9600);
  Timer1.initialize(2000000); // us단위, 1초
  Timer1.attachInterrupt(timer1Isr); // 인터럽트 핸들러 함수 추가
}

void timer1Isr()
{
  timer1Flag = true;
}

void loop()
{
  char str[20];
  int cnt;

  cnt = Serial.readBytesUntil('\n', str, sizeof(str)); 
  str[cnt]= '\n';
  str[cnt+1]= '\0';
  
  if(timer1Flag)
  {
      int sensorValue = analogRead(A1);
      int CDSValue = analogRead(A0);
      
      // 실수형 계산시 소수점찍는 것 주의
      double a = (double)sensorValue * (5/1024.0);
      int a1 = map(sensorValue, 0, 1023, 0, 100);
      
      double b = (double)CDSValue * (5/1024.0);
      int b1 = map(CDSValue,0,1023,0,100);
      
      Serial.print("ADC: ");
      Serial.print(sensorValue);
      Serial.print(", Volt: ");
      Serial.print(a);
      Serial.print("V , ");
      Serial.print("percent: ");
      Serial.print(a1);
      Serial.print("% ,");
    
      Serial.print("CDS: ");
      Serial.print(CDSValue);
      Serial.print(", Volt: ");
      Serial.print(b);
      Serial.print(", percent: ");
      Serial.print(b1);
      Serial.println("%");

      timer1Flag = false;
  }
}
