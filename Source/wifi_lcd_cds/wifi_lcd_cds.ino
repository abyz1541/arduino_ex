/*
 WiFiEsp test: ClientTest
http://www.kccistc.net/
작성일 : 2019.12.17 
작성자 : IoT 임베디드 KSH
*/
#define DEBUG
//#define DEBUG_WIFI

#define AP_SSID "iotemb5"
#define AP_PASS "iotemb50"
#define SERVER_NAME "192.168.10.55"
#define SERVER_PORT 5000  
#define LOGID "LHO"
#define PASSWD "PASSWD"

#define WIFITX 7  //7:TX -->ESP8266 RX
#define WIFIRX 6 //6:RX-->ESP8266 TX
#define LED_BUILTIN_PIN 13
#define LED_LAMP_PIN 3
#define BUTTON_PIN 2
#define CDS_PIN A0

#define CMD_SIZE 50
#define ARR_CNT 5    

#include "WiFiEsp.h"
#include "SoftwareSerial.h"
#include <TimerOne.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

char sendBuf[CMD_SIZE];

bool timerIsrFlag = false;
unsigned int secCount;
int cds=0;
char sendId[10]="KSH_LDP";

SoftwareSerial wifiSerial(WIFIRX, WIFITX); 
WiFiEspClient client;

char lcdLine1[17]="Smart IoT By KSH";
char lcdLine2[17]="WiFi Connecting!";
boolean lastButton = LOW;     // 버튼의 이전 상태 저장
boolean currentButton = LOW;    // 버튼의 현재 상태 저장
boolean ledOn = false;      // LED의 현재 상태 (on/off)
boolean cdsFlag = false;

void setup() {
  // put your setup code here, to run once:
    lcd.init();
    lcd.backlight();
    lcdDisplay(0,0,lcdLine1);
    lcdDisplay(0,1,lcdLine2);
    
    pinMode(LED_BUILTIN_PIN, OUTPUT); //D13
    pinMode (LED_LAMP_PIN, OUTPUT);    // LED 핀을 출력으로 설정
    pinMode (BUTTON_PIN, INPUT);    // 버튼 핀을 입력으로 설정 (생략 가능)
    pinMode (CDS_PIN, INPUT);    // 조도 핀을 입력으로 설정 (생략 가능)
    
    Serial.begin(9600); //DEBUG
    Serial.print("sendid :");
    Serial.print(sendId);

    
    wifi_Setup();

    Timer1.initialize(1000000);
    Timer1.attachInterrupt(timerIsr); // timerIsr to run every 1 seconds
}

void loop() {
  // put your main code here, to run repeatedly:
  if(client.available()) {
    socketEvent();
  }
  if (timerIsrFlag)
  {
    timerIsrFlag = false; 
    if(!(secCount%5))
    {
      if (!client.connected()) { 
        lcdDisplay(0,1,"Server Down");
        server_Connect();
      }
    } 
//    if(!(secCount%10))
    {
      cds = map(analogRead(CDS_PIN),0,1023,0,100);
      Serial.print("CDS : ");
      Serial.println(cds);
      if((cds >= 50) && cdsFlag)
      {
        cdsFlag = false;
        sprintf(sendBuf,"[%s]CDS@%d\n",sendId,cds);    
        client.write(sendBuf,strlen(sendBuf));
        client.flush();
        digitalWrite(LED_BUILTIN_PIN, HIGH);     // LED 상태 변경  
      }else if((cds < 50) && !cdsFlag) 
      {
        cdsFlag = true;
        sprintf(sendBuf,"[%s]CDS@%d\n",sendId,cds);    
        client.write(sendBuf,strlen(sendBuf));
        client.flush();   
        digitalWrite(LED_BUILTIN_PIN, LOW);     // LED 상태 변경  
      }
    }     
  }
  
  currentButton = debounce(lastButton);   // 디바운싱된 버튼 상태 읽기
  if (lastButton == LOW && currentButton == HIGH)  // 버튼을 누르면...
  {
    ledOn = !ledOn;       // LED 상태 값 반전
    digitalWrite(LED_BUILTIN_PIN, ledOn);     // LED 상태 변경
    sprintf(sendBuf,"[%s]BUTTON@%s\n",sendId,ledOn?"ON":"OFF");    
    client.write(sendBuf,strlen(sendBuf));
    client.flush();
  }
  lastButton = currentButton;     // 이전 버튼 상태를 현재 버튼 상태로 설정
  
  
}
void socketEvent()
{
  int i=0;
  char * pToken;
  char * pArray[ARR_CNT]={0};
  char recvBuf[CMD_SIZE]={0}; 
  int len;

  sendBuf[0] ='\0';
  len =client.readBytesUntil('\n',recvBuf,CMD_SIZE); 
  client.flush();
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
  //[KSH_ARD]LED@ON : pArray[0] = "KSH_ARD", pArray[1] = "LED", pArray[2] = "ON"
  if((strlen(pArray[1]) + strlen(pArray[2])) < 16)
  {
    sprintf(lcdLine2,"%s %s",pArray[1],pArray[2]);
    lcdDisplay(0,1,lcdLine2);
  }
  if(!strncmp(pArray[1]," New",4))  // New Connected
  {
    Serial.write('\n');
    strcpy(lcdLine2,"Server Connected");
    lcdDisplay(0,1,lcdLine2);
    return ;
  }
  else if(!strncmp(pArray[1]," Alr",4)) //Already logged
  {
    Serial.write('\n');
    client.stop();
    server_Connect();
    return ;
  }   
  else if(!strcmp(pArray[1],"LED")) {
    if(!strcmp(pArray[2],"ON")) {
      digitalWrite(LED_BUILTIN_PIN,HIGH);
    }
    else if(!strcmp(pArray[2],"OFF")) {
      digitalWrite(LED_BUILTIN_PIN,LOW);
    }
    sprintf(sendBuf,"[%s]%s@%s\n",pArray[0],pArray[1],pArray[2]);
  } else if(!strcmp(pArray[1],"LAMP")) {
    if(!strcmp(pArray[2],"ON")) {
      digitalWrite(LED_LAMP_PIN,HIGH);
    }
    else if(!strcmp(pArray[2],"OFF")) {
      digitalWrite(LED_LAMP_PIN,LOW);
    }
    sprintf(sendBuf,"[%s]%s@%s\n",pArray[0],pArray[1],pArray[2]);
  } else if(!strcmp(pArray[1],"GETSTATE")) {
    strcpy(sendId,pArray[0]);
    if(!strcmp(pArray[2],"DEV")) {
      sprintf(sendBuf,"[%s]DEV@%s\n",pArray[0],digitalRead(LED_BUILTIN_PIN)?"ON":"OFF");
    }
  } else
      return;
  
  client.write(sendBuf,strlen(sendBuf));
  client.flush();

#ifdef DEBUG
  Serial.print(", send : ");
  Serial.print(sendBuf);
#endif
}
void timerIsr()
{
//  digitalWrite(LED_BUILTIN_PIN,!digitalRead(LED_BUILTIN_PIN));
  timerIsrFlag = true;
  secCount++;
}
void wifi_Setup() {
  wifiSerial.begin(19200);
  wifi_Init();
  server_Connect();
}
void wifi_Init()
{
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
  while(WiFi.begin(AP_SSID, AP_PASS) != WL_CONNECTED) {   
#ifdef DEBUG_WIFI  
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(AP_SSID);   
#endif   
  }
  sprintf(lcdLine1,"ID:%s",LOGID);  
  lcdDisplay(0,0,lcdLine1);
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
    Serial.println("Connect to server");
#endif  
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
boolean debounce(boolean last)
{
  boolean current = digitalRead(BUTTON_PIN);  // 버튼 상태 읽기
  if (last != current)      // 이전 상태와 현재 상태가 다르면...
  {
    delay(5);         // 5ms 대기
    current = digitalRead(BUTTON_PIN);  // 버튼 상태 다시 읽기
  }
  return current;       // 버튼의 현재 상태 반환
}
