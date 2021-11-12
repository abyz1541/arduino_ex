const byte ledPin = 3;
const byte interruptPin = 2;

volatile byte state = LOW;

unsigned int cnt = 0;

// bool Type 0 아니면 1
volatile bool ext0Flag;

void setup()
{
  Serial.begin(9600);
  Serial.print("Setup() Start!");
  
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, RISING);
}

void loop()
{
  if(ext0Flag)
  {
    // Pin 상태를 읽어올 수 없기에 Led 상태로 대체
    if(digitalRead(ledPin))
    {
      digitalWrite(ledPin, LOW);
      Serial.println(cnt++);
    }else{
      digitalWrite(ledPin, HIGH);
    }
    
    ext0Flag = false;
  }
}

void blink()
{
//  인터럽트 함수는 최대한 긴급한 것만 처리 후 빠져나오는 것이 좋음
  ext0Flag = true;
}
