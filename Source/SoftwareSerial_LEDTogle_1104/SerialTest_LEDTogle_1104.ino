void setup()
{
  Serial.begin(9600);
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
}

void loop()
{
  char str[20];
  int cnt;

  if (Serial.available())
  {
    cnt = Serial.readBytesUntil('\n', str, sizeof(str)); 
    str[cnt]= '\n';
    str[cnt+1]= '\0';
    
    Serial.print(str);
    
    if(!strncmp(str,"ledon",5))
      str[5] == '0' ? digitalWrite(12,HIGH) : digitalWrite(13,HIGH);
    else if (!strncmp(str,"ledoff",6))
      str[6] == '0' ? digitalWrite(12,LOW) : digitalWrite(13,LOW);
  }
}
