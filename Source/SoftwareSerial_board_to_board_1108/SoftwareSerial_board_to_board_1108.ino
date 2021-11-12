#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX

void setup() 
{
  Serial.begin(9600);
  while (!Serial) 
  {
  }

  Serial.println("Goodnight moon!");

  mySerial.begin(9600); // set the data rate for the SoftwareSerial port
  mySerial.println("Hello, world?");
  
  pinMode(13,OUTPUT);
}

void loop() 
{ 
  char str[20];
  int cnt;

#if 0
  if (mySerial.available())
  {
    cnt = mySerial.readBytesUntil('\n', str, sizeof(str)); // 
    str[cnt]= '\n';   // 
    str[cnt+1]= '\0'; // 문자열 끝 처리
    Serial.print(str);

    if(!strncmp(str,"ledon",5))
    {
      digitalWrite(13,HIGH);
    }
    else if (!strncmp(str,"ledoff",6))
    {
      digitalWrite(13,LOW);
    }
  }
#endif

#if 1
  if (mySerial.available())
  {
    cnt = mySerial.readBytesUntil('\n', str, sizeof(str)); 
    str[cnt]= '\n';
    str[cnt+1]= '\0';
    
    Serial.print(str);
    
    if(!strncmp(str,"ledon",5))
      str[5] == '0' ? digitalWrite(12,HIGH) : digitalWrite(13,HIGH);
    else if (!strncmp(str,"ledoff",6))
      str[6] == '0' ? digitalWrite(12,LOW) : digitalWrite(13,LOW);
  }
#endif

  if (Serial.available()) 
  {
    mySerial.write(Serial.read());// 연결된 board에 보냄
  }
}
