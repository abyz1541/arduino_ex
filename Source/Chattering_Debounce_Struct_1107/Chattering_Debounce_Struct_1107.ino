#define Serialonoff 0

struct TOGLE
{
  char togle_name[20];
  const int BT_pin;
  const int LED_pin;
  int LED_State;
  int BT_State;
  int last_BT_State;
  unsigned int cnt;
  int reading;
};

// (name, BT_pin, LED_pin, LED_State, BT_State, last_BT_State, cnt, reading)
TOGLE A = { "A", 2, 12, LOW, LOW, HIGH, 0};
TOGLE B = { "B", 3, 13, LOW, LOW, HIGH, 0};

// 소프트웨어적으로 처리하는 것 좋지않음
// 시정수를 계산 후 Delay Time을 지연회로로 처리하는 것이 바람직함
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 10;

void setup()
{
  set_togle_BT_LED(A.BT_pin,A.LED_pin);
  set_togle_BT_LED(B.BT_pin,B.LED_pin);

#if Serialonoff
  Serial.begin(9600);
  Serial.println("START SET UP");
#endif

}

void loop()
{
  togle_BT_LED(&A);
  togle_BT_LED(&B);
}

void set_togle_BT_LED (int BT_pin, int LED_pin)
{
  pinMode(BT_pin, INPUT);
  pinMode(LED_pin, OUTPUT);
}

void togle_BT_LED(TOGLE* T)
{
  T->reading = digitalRead(T->BT_pin);
  
  if (T->reading != T->last_BT_State)
  {
    lastDebounceTime = millis();
    T->last_BT_State = T->reading;
  }

  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    if (T->reading != T->BT_State)
    {
      T->BT_State = T->reading;
      if (T->BT_State == HIGH)
      {
        
        T->LED_State = !(T->LED_State);
        
#if Serialonoff
        Serial.print(T->togle_name);
        Serial.print(": ");
        Serial.println(T->cnt++);
#endif

      }
      digitalWrite(T->LED_pin, T->LED_State);
    }
  }
}
