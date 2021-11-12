// const 변수를 상수화, 다른값으로 변경 못하게 고정
const int SW = 2;
const int led = 13;

int led_State = LOW;
int SW_State = 0;;
int lastSW_State = LOW;
int reading = LOW;

unsigned long last_DebounceTime = 0;
unsigned long debounce_Delay = 10;

volatile bool ext0Flag = false;

void setup()
{
  Serial.begin(9600);
  Serial.println("START SET UP");
  pinMode(SW, INPUT);
  pinMode(led, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(SW), blink, RISING);
}

void loop()
{
  if(ext0Flag)
  {
      ext0Flag = false;
      reading = digitalRead(SW);
      
      if (reading != lastSW_State)
      {
          last_DebounceTime = millis();
          lastSW_State = reading; // 상태가 바뀌면 저장
      }
    
      if(millis() - last_DebounceTime > debounce_Delay)
      {
          if(reading != SW_State)
          {
              SW_State = reading;
              
              if (SW_State == HIGH)
              {
                 led_State = !led_State;
              }
              digitalWrite(led, led_State); // 지연시간 후 출력신호
          }
      }
  }
}

void blink()
{
//  인터럽트 함수는 최대한 긴급한 것만 처리 후 빠져나오는 것이 좋음
  ext0Flag = true;
}
