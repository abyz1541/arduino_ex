#include "LowPower.h"

const byte ledPin = 12;
const byte interruptPin = 2;

const byte ledPin1 = 13;
const byte interruptPin1 = 3;

volatile byte state = LOW;

unsigned int cnt = 0;

// bool Type 0 아니면 1
volatile bool ext0Flag;
volatile bool ext1Flag;

void setup()
{
  Serial.begin(9600);
  Serial.print("Setup() Start!");
  
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT);

  pinMode(ledPin1, OUTPUT);
  pinMode(interruptPin1, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, RISING);
  attachInterrupt(digitalPinToInterrupt(interruptPin1), blink1, RISING);
}

void loop()
{
  Serial.print("loop test : ");
  Serial.println(cnt++);
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
  else if(ext1Flag)
  {
    //state = !state;
    if(digitalRead(ledPin1))
    {
      digitalWrite(ledPin1, LOW);
      Serial.println(cnt++);
    }else{
      digitalWrite(ledPin1, HIGH);
    }
    
    ext1Flag = false;
  }

  // Sleep Mode
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
   
}

void blink()
{
  //  인터럽트 함수는 최대한 긴급한 것만 처리 후 빠져나오는 것이 좋음
  ext0Flag = true;
}

void blink1()
{
  ext1Flag = true;
}
