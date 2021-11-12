#define LED 12

const int Switch_down = 2;
const int Switch_up = 3;
const int Switch_inup = 4;

int KeyCount = 0;

int preSwitch = LOW;

int buttonState_up = 0;
int buttonState_down = 0;
int buttonState_inup = 0;

void setup()
{
   Serial.begin(9600);
   pinMode(Switch_down, INPUT);
   pinMode(Switch_up, INPUT);
   pinMode(Switch_inup, INPUT_PULLUP);
   pinMode(LED, OUTPUT);
}

void loop()
{

#if 1 // PULL_UP
  buttonState_up = digitalRead(Switch_up);

  // 버튼이 눌렸을 때만 실행
  if(buttonState_up != preSwitch)
  {
      delay(5);
      buttonState_up = digitalRead(Switch_up);
      
      Serial.println(KeyCount++);

      if(buttonState_up == HIGH)
      {
         Serial.println("LOW");
         digitalWrite(LED, LOW);
      }
      else
      {
        Serial.println("HIGH");
        digitalWrite(LED, HIGH);
      }
      
      preSwitch = buttonState_up;
  }
#endif

#if 0 // PULL_DOWN
  buttonState_down = digitalRead(Switch_down);
  
  if(buttonState_down == HIGH)
  {
     Serial.println("HIGH");
  }
  else
  {
    Serial.println("LOW");
  }
#endif

#if 0 // PULL_UP_IN
  buttonState_inup = digitalRead(Switch_inup);
  
  if(buttonState_inup == HIGH)
  {
     Serial.println("HIGH");
  }
  else
  {
    Serial.println("LOW");
  }
#endif


}
