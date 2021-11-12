#include <TimerOne.h>
#define LED0 13
#define VAR A0
#define CDS A1

volatile bool timer1Flag=false;
volatile unsigned long secCount = 0;

int varValue;
int cdsValue;

bool cdsState = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Test Serial");
  
  Timer1.initialize(1000000); //uSec 1000000:1Sec
  Timer1.attachInterrupt(timer1Isr);

  pinMode(LED0,OUTPUT);
}

void loop() { 
  char str[20] = {0}; // 20bytes 전체를 NULL(\0)문자로 초기화
  char sendBuff[20];
  int cnt;

  if (Serial.available())
  {
      cnt = Serial.readBytesUntil('\n',str,sizeof(str)-2);
      str[cnt] = '\0';
      
      if(!strcmp(str,"ledon"))
      {
          digitalWrite(LED0,HIGH);
      }
      else if(!strcmp(str,"ledoff"))
      {
          digitalWrite(LED0,LOW);
      }     
      else if(!strncmp(str,"getsensor",9))
      {
         if(str[9] == '\0'){
            sprintf(sendBuff,"var:%d%% cds:%d%%\n", varValue, cdsValue);
            Serial.print(sendBuff);
            getSensorTime = 0;
            return;
         }else{
           getSensorTime = atoi(str+10);
         }
      }
      str[cnt] = '\n';
      str[cnt+1] = '\0';
      Serial.print(str);
  }
  
  if(timer1Flag)
  {
    varValue = analogRead(VAR);
    varValue = map(varValue,0,1023,0,100);
    cdsValue = analogRead(CDS);
    cdsValue = map(analogRead(CDS),0,1023,0,100);

    if( !cdsState && cdsValue >= 50) {
      sprintf(sendBuff,"lampOn\n");
      Serial.print(sendBuff);
      cdsState = true;
    }else if (cdsState && cdsValue < 50){
      sprintf(sendBuff,"lampOff\n");
      Serial.print(sendBuff);
      cdsState = false;
    }

    if(getSensorTime != 0 && !(secCount%getSensorTime)){
       sprintf(sendBuff,"var:%d%% cds:%d%%\n", varValue, cdsValue);
       Serial.print(sendBuff);
    }
    
    timer1Flag = false;
  }
}

void timer1Isr()
{
  timer1Flag = true;
  secCount++;
}
