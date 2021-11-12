#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>
#define buzerPin 9

LiquidCrystal_I2C lcd(0x27, 16, 2);

volatile bool timer1Flag=false;
volatile unsigned long secCount = 0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 10;

char lcdLine1[17]="Smart IoT By KSH";
char lcdLine2[17]="WiFi Connecting!";

typedef struct{
  char togle_name;
  const int BT_pin;
  int BT_State;
  int last_BT_State;
  int cnt;
  int reading;
}TOGLE;

typedef struct{
  int hour;
  int min;
  int sec;
  char state;
}TIME;

TOGLE A = { 'A', 10, LOW, HIGH, 0};
TOGLE B = { 'B', 11, LOW, HIGH, 0};
TOGLE C = { 'C', 12, LOW, HIGH, 0};

TIME Time = {3,0,30};
TIME Alarm = {0,0,0};

bool LCD1state = false;
bool LCD2state = false;
bool AlarmState = false;

int Bstate = 1;
int AlarmTime = 100;

void setup()
{
  Serial.begin(9600);
  Serial.println("test");

  lcd.init();
  lcd.backlight();

  sprintf(lcdLine1, "Time %02d: %02d: %02d", Time.hour, Time.min, Time.sec);
  lcdDisplay(0,0,lcdLine1);
  sprintf(lcdLine2, "Alar %02d: %02d: %02d", Alarm.hour, Alarm.min, Alarm.sec);
  lcdDisplay(0,1,lcdLine2);
  
  Timer1.initialize(1000000); //uSec 1000000:1Sec
  Timer1.attachInterrupt(timer1Isr);

  pinMode(A.BT_pin, INPUT);
  pinMode(B.BT_pin, INPUT);
  pinMode(C.BT_pin, INPUT);

  pinMode(buzerPin, OUTPUT);
}

void loop()
{
   togle_BT(&A);
   togle_BT(&B);
   togle_BT(&C);

   if(timer1Flag)
   {
      sprintf(lcdLine1, "Time %02d: %02d: %02d%c", Time.hour, Time.min, Time.sec, Time.state);
      lcdDisplay(0,0,lcdLine1);
      sprintf(lcdLine2, "Alar %02d: %02d: %02d%c", Alarm.hour, Alarm.min, Alarm.sec, Alarm.state);
      lcdDisplay(0,1,lcdLine2);

      if(Time.hour==Alarm.hour && Time.min==Alarm.min)
      {
              if(Time.sec==Alarm.sec)
              {
                AlarmTime = 0;
                Bstate = 0;
                AlarmState = true;
              }

              if(AlarmTime < 60)
              {
                  if(AlarmState == true)
                  {
                      if(Bstate == 0)
                      {
                        digitalWrite(buzerPin, HIGH);
                        Bstate = 1;
                      }
                      else if(Bstate == 1)
                      {
                        digitalWrite(buzerPin, LOW);
                        Bstate = 0;
                      }
                      AlarmTime++; 
                  }
              }
      }
      
      Time.sec++;
      if ( Time.sec == 60)
      {
          Time.sec = 0;
          Time.min++;
          if ( Time.min == 60)
          {
              Time.min = 0;
              Time.hour++;
              if(Time.hour == 24)
              {
                  Time.hour = 0;
              }
          }
      }
      
      timer1Flag = false;
   }
}

void lcdDisplay(int x, int y, char * str)
{
  int len = 16 - strlen(str);
  lcd.setCursor(x,y);
  lcd.print(str);
  for(int i=len;i>0;i--)
    lcd.write(' ');
}

void timer1Isr()
{
  timer1Flag = true;
  secCount++;
}

void togle_BT(TOGLE* T)
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
                if(T->togle_name == 'A')
                {
                     AlarmState = false;
                     if(LCD1state == true)
                     {
                        if(Time.hour==24)
                        {
                          Time.hour = 0;
                        }
                        Time.hour++;
                     }
                     else if (LCD2state == true)
                     {
                        if(Alarm.hour==24)
                        {
                          Alarm.hour = 0;
                        }
                        Alarm.hour++;
                     }
                }
                else if(T->togle_name == 'B')
                {
                     AlarmState = false;
                     if(LCD1state == true)
                     {
                        if(Time.min==60)
                        {
                          Time.min = 0;
                        }
                        Time.min++;
                     }
                     else if (LCD2state == true)
                     {
                        if(Alarm.min==60)
                        {
                          Alarm.min = 0;
                        }
                        Alarm.min++;
                     }
                }
                else if(T->togle_name == 'C')
                {
                    T->cnt++;
                    AlarmState = false;

                    unsigned long BTtime = millis();

                    while(digitalRead(T->BT_pin)==HIGH)
                    {
                        if(millis()+2000)
                        {
                            Time.state = '\0';
                            Alarm.state = '\0';
                            Time = {3,0,30};
                            Alarm = {0,0,0};
                            LCD1state = false;
                            LCD2state = false;
                        }
                    }
                    
                    if(T->cnt == 1)
                    {
                        Time = {3,0,30};
                        Alarm = {0,0,0};
                        LCD1state = false;
                        LCD2state = false;
                    }
                    else if (T->cnt == 2)
                    {
                        Time.state = '*';
                        Alarm.state = '\0';
                        LCD1state = true;
                        LCD2state = false;
                    }
                    else if (T->cnt == 3)
                    {
                        Time.state = '\0';
                        Alarm.state = '*';
                        LCD1state = false;
                        LCD2state = true;
                        Bstate = 0;
                    }
                    else if (T->cnt == 4)
                    {
                        Time.state = '\0';
                        Alarm.state = '\0';
                        LCD1state = false;
                        LCD2state = false;
                        Bstate = 0;
                        T->cnt = 0;
                    }
                }
            }
        }
    }
}
