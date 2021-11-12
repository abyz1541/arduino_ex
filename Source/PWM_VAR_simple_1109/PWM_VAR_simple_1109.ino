#define varPin A0

int led = 9;
int varValue, varValue_last;

void setup()
{
  //Serial.begin(9600);
  pinMode(led, OUTPUT);
}

void loop()
{
  varValue = analogRead(varPin);
  
  if(abs(varValue - varValue_last)>=7)
  {
    varValue_last = varValue;
    //Serial.print("varValue: ");
    //Serial.println(varValue);
    
    varValue = map(varValue,0,1023,0,255);
    analogWrite(led, varValue);

  }
}
