#define CDS_Value 60

#include <TimerOne.h>

volatile bool timer1Flag = false;

int CDS, CDS_PERCENT, CDS_STATE, CDS_STATE_LAST;

void setup()
{
    Serial.begin(9600);
    Serial.println("Set up. Start!");

    Timer1.initialize(500000); // us단위, 0.5초
    Timer1.attachInterrupt(timer1Isr); // 인터럽트 핸들러 함수 추가
}

void loop()
{
  if(timer1Flag)
  {
    CDS = analogRead(A1);
    CDS_PERCENT = map(CDS,0,1023,0,100);

    if(CDS_PERCENT > CDS_Value)
    {
      CDS_STATE = 1;
    }
    else
    {
      CDS_STATE = 0;
    }

    timer1Flag = false;  
  }
  
  if(Serial.available())
  {
    char str[20];
    int cnt, sen, VAR_PERCENT;

    sen = analogRead(A0);
    VAR_PERCENT = map(sen,0,1023,0,100);

    cnt = Serial.readBytesUntil('\n', str, sizeof(str)); 
    str[cnt]= '\n';
    str[cnt+1]= '\0';

    if(!strcmp(str,"getsensor\n"))
    { 
      Serial.print("VAR: ");
      Serial.print(VAR_PERCENT);
      Serial.print("% ,");
    
      Serial.print("CDS: ");
      Serial.print(CDS_PERCENT);
      Serial.println("%");
    }
  }

  if(CDS_STATE != CDS_STATE_LAST)
  {
      if(CDS_STATE == 1)
      {
        Serial.println("LampOn");
      }
      else if(CDS_STATE == 0)
      {
        Serial.println("LampOff");
      }
      CDS_STATE_LAST = CDS_STATE;
  }
}

void timer1Isr()
{
  timer1Flag = true;
}
