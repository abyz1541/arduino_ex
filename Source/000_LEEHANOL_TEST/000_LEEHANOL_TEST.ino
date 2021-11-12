/*
 WiFiEsp test: ClientTest
http://www.kccistc.net/
작성일 : 2019.12.17 
작성자 : IoT 임베디드 KSH
*/
//#define DEBUG
//#define DEBUG_WIFI

#include <WiFiEsp.h>
#include <SoftwareSerial.h>
#include <TimerOne.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define AP_SSID "iotemb5" // 공유기 정보
#define AP_PASS "iotemb50"
#define SERVER_NAME "192.168.1.55"
#define SERVER_PORT 5000  
#define LOGID "LHO"
#define PASSWD "PASSWD"

#define WIFITX 7  //7:TX -->ESP8266 RX
#define WIFIRX 6 //6:RX-->ESP8266 TX
#define LED_BUILTIN_PIN 13
#define CMD_SIZE 60
#define ARR_CNT 5  // 문자열 명령어 수 정의
#define buzerPin 9

// SW, LED, CDS Pin 정의
#define SW_PIN 2
#define LED_PIN 3
#define CDS_PIN A0
#define CDS_Value 50         

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial wifiSerial(WIFIRX, WIFITX); 
WiFiEspClient client;

char sendBuf[CMD_SIZE];
bool timerIsrFlag = false;
unsigned int secCount;
int sensorTime;

char lcdLine1[17]="Smart IoT By LHO";
char lcdLine2[17]="WiFi Connecting!";

const byte interruptPin = SW_PIN;
volatile byte state = LOW;
volatile bool ext0Flag;

char sendID[10] = "LHO_SMP";

int CDS, CDS_PERCENT, CDS_STATE, CDS_STATE_LAST;

void setup()
{
    lcd.init();
    lcd.backlight();
    lcdDisplay(0,0,lcdLine1);
    lcdDisplay(0,1,lcdLine2);
    
    pinMode(LED_BUILTIN_PIN, OUTPUT); //D13
    pinMode(buzerPin, OUTPUT); //D9
    pinMode(SW_PIN, INPUT); //D2
    pinMode(LED_PIN, OUTPUT); //D3
    
    Serial.begin(9600); //DEBUG
    wifi_Setup();

    Timer1.initialize(1000000);
    Timer1.attachInterrupt(timerIsr); // timerIsr to run every 1 seconds

    attachInterrupt(digitalPinToInterrupt(SW_PIN), blink, RISING);
}

void loop()
{
  if(client.available()) {
    socketEvent();
  }
  if (timerIsrFlag)
  {
      timerIsrFlag = false;
  
      //CDS값 1초마다 읽고 기준값 기준으로 STATE 변경해줌
      CDS = analogRead(CDS_PIN);
      CDS_PERCENT = map(CDS,0,1023,0,100);

      // CDS STATE가 변경되었을 때만 13번 LED On/off하고, 스마트폰에 CDS값 메세지 전송
      if(CDS_PERCENT > CDS_Value)
        CDS_STATE = 1;
      else
        CDS_STATE = 0;
  
      if(CDS_STATE != CDS_STATE_LAST)
      {
        if(CDS_STATE == 1)
          digitalWrite(LED_BUILTIN_PIN, HIGH);
        else if(CDS_STATE == 0)
          digitalWrite(LED_BUILTIN_PIN, LOW);
        CDS_STATE_LAST = CDS_STATE;
        sprintf(sendBuf,"[%s]CDS@%d\r\n", sendID ,CDS_PERCENT);
        client.write(sendBuf,strlen(sendBuf));
        client.flush();
      }
      
      if(!(secCount%5))
      {
        if (!client.connected()) { 
          lcdDisplay(0,1,"Server Down");
          server_Connect();
        }
      }
  }

  // 인터럽트 토글기능구현 ( 회로 지연회로로 체터링 제거필요 )
  // 인터럽트 신호 오면 13번 LED On/off하고, 스마트폰에 버튼 메세지 전송
  if(ext0Flag)
  {
    ext0Flag = false;
    if(digitalRead(LED_BUILTIN_PIN))
    {
      digitalWrite(LED_BUILTIN_PIN, LOW);
      sprintf(sendBuf,"[%s]BUTTON@OFF\r\n", sendID);
    }else{
      digitalWrite(LED_BUILTIN_PIN, HIGH);
      sprintf(sendBuf,"[%s]BUTTON@ON\r\n", sendID);
    }
    client.write(sendBuf,strlen(sendBuf));
    client.flush();
  }
}

void socketEvent()
{
  int i=0;
  char * pToken;
  char * pArray[ARR_CNT]={0};
  char recvBuf[CMD_SIZE]={0}; 
  int len;

  len =client.readBytesUntil('\n',recvBuf,CMD_SIZE-1);
  recvBuf[len] = '\0'; // 문자열 완성
  client.flush(); // 수신된 버퍼에 남아있는 내용을 지우는 함수
  
#ifdef DEBUG
  Serial.print("recv : ");
  Serial.print(recvBuf);
#endif

  pToken = strtok(recvBuf,"[@]");
  while(pToken != NULL)
  {
    pArray[i] =  pToken;
    if(++i >= ARR_CNT)
      break;
    pToken = strtok(NULL,"[@]");
  }
  if(!strncmp(pArray[1]," New",4))  // New Connected
  {
    lcdDisplay(0,1,"New connected!");
    return ;
  }
  else if(!strncmp(pArray[1]," Alr",4)) //Already logged
  {
    client.stop();
    server_Connect();
    return ;
  }
  else if(!strncmp(pArray[1],"LEDO",4))
  {
      if(pArray[1][4]=='N') //Already logged
      {
        digitalWrite(LED_BUILTIN_PIN, HIGH);
      }
      else if(pArray[1][4]=='F') //Already logged
      {
        digitalWrite(LED_BUILTIN_PIN, LOW);
      }
      sprintf(sendBuf,"[%s]%s\n",pArray[0],pArray[1]);
  }
  else if(!strcmp(pArray[1],"LAMP")) //LAMP@ON/OFF 명령어 처리 (D3 LED On/off)
  {
      if(pArray[2][1]=='N') //Already logged
      {
        digitalWrite(LED_PIN, HIGH);
      }
      else if(pArray[2][1]=='F') //Already logged
      {
        digitalWrite(LED_PIN, LOW);
      }
      sprintf(sendBuf,"[%s]D3연결된 LED %s\n", pArray[0],pArray[2]);
  }
#if 0 // BUZZER
  else if(!strncmp(pArray[1],"BURO",4)) //Already logged
  {
    if(pArray[1][4] == 'N')
    {
      digitalWrite(buzerPin, HIGH);
    }
    else if(pArray[1][4] == 'F')
    {
      digitalWrite(buzerPin, LOW);
    }
    sprintf(sendBuf,"[%s]%s\n",pArray[0],pArray[1]);
  }
#endif
  
  client.write(sendBuf,strlen(sendBuf));
  client.flush();

#ifdef DEBUG
  Serial.print(", send : ");
  Serial.println(sendBuf);
#endif
}

void timerIsr()
{
  timerIsrFlag = true;
  secCount++;
}

void wifi_Setup()
{
  wifiSerial.begin(19200);
  wifi_Init(); // wifi module 초기화
  server_Connect();
}

void wifi_Init()
{
  // 하드웨어 상태 check
  do {
    WiFi.init(&wifiSerial);
    if (WiFi.status() == WL_NO_SHIELD) {
#ifdef DEBUG_WIFI    
      Serial.println("WiFi shield not present");
#endif 
    }
    else
      break;   
  }while(1);

#ifdef DEBUG_WIFI    
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(AP_SSID);
#endif
  // 접속이 안되면 무한 loop()  
  while(WiFi.begin(AP_SSID, AP_PASS) != WL_CONNECTED) {   

#ifdef DEBUG_WIFI  
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(AP_SSID);   
#endif   
  }
  sprintf(lcdLine1,"ID:%s",LOGID);  
  lcdDisplay(0,0,lcdLine1);
  //버퍼오버플로우 주의!, 버퍼오버플로우 리셋됨, 디버그 어려움.
  sprintf(lcdLine2,"%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  lcdDisplay(0,1,lcdLine2);
  
#ifdef DEBUG_WIFI      
  Serial.println("You're connected to the network");    
  printWifiStatus();
#endif 
}

int server_Connect()
{
#ifdef DEBUG_WIFI     
  Serial.println("Starting connection to server...");
#endif  

  if (client.connect(SERVER_NAME, SERVER_PORT)) {
#ifdef DEBUG_WIFI     
    Serial.println("Connected to server");
#endif
    // define된 문자열을 사용하기위해 " "로 작은단위 문자열 구분
    client.print("["LOGID":"PASSWD"]");
  }
  else
  {
#ifdef DEBUG_WIFI      
     Serial.println("server connection failure");
#endif    
  } 
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  // 수신감도 출력
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void lcdDisplay(int x, int y, char * str)
{
  int len = 16 - strlen(str);
  lcd.setCursor(x,y);
  lcd.print(str);
  for(int i=len;i>0;i--)
    lcd.write(' ');
}

void blink()
{
  ext0Flag = true;
}
