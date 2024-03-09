#include <mutex>
#include <esp_task_wdt.h>
///* Serial interface pin definitions */
#define KEYBOARD_RX_PIN 5 
#define KEYBOARD_TX_PIN 16 
#define MVHR_RX_PIN 4 
#define MVHR_TX_PIN 17

HardwareSerial &kbdSerial  = Serial1;   // This goes to the keyboard
HardwareSerial &mvhrSerial = Serial2;   // This goes to the MVHR

TaskHandle_t taskSerial;
TaskHandle_t taskControl;

// Timing
unsigned long tLast=0;
unsigned long tLast2=0;
unsigned long tCurrent;
unsigned long tLastReceivedKbd=0;
unsigned long tLastReceivedMvhr=0;
#define TMR_INTERVAL 10000
#define TMR_INTERVAL2 3000
#define TMR_PACKET_END_SPACE 10         // there is 18 to 20ms between packets (from kbd) so 10ms should be enough to detect & process
#define MVHR_TO_KBD 1
#define KBD_TO_MVHR 2
#define H0_KEYPRESS 4
#define H0_MVHR_MESSAGE 2

#define KB_KEY_VENT 8
#define KB_KEY_UP 2
#define KB_KEY_DOWN 1
#define KB_KEY_SET 4

bool kbdNewData=false;
bool mvhrNewData=false;

String kbdBuff="";
String mvhrBuff="";
String kbdCache="";
String mvhrCache="";

std::mutex mtxKbdBuff;
std::mutex mtxMvhrBuff;
unsigned long tActual;
void taskSerialRuntime( void * pvParameters ){
  for(;;)
  {  
    tActual=millis();
    if (mvhrSerial.available() >0) {           // read tata sent from mvhr unit (display data)
      int mvhrIncoming = mvhrSerial.read();
      tLastReceivedMvhr=millis();
      std::lock_guard<std::mutex> lck(mtxMvhrBuff);
      mvhrCache.concat(char(mvhrIncoming));         // write to cache
      mvhrNewData=true;
    }
    if (kbdSerial.available() > 0) {        // read data from keyboard unit (button pressed)
      int kbdIncoming = kbdSerial.read();
      tLastReceivedKbd=millis();
      std::lock_guard<std::mutex> lck(mtxKbdBuff);
      kbdCache.concat(char(kbdIncoming));           // write to cache
      kbdNewData=true;
    }
    delay(1);
  }
}

void processPacket(unsigned int Type, String *buffer) 
{
  String tempPacket="";
  bool bValid=false;
  if (Type==MVHR_TO_KBD)
    { Serial.print("MVHR > KBD: "); }
  else
    { Serial.print("KBD > MVHR: "); }
  int bufferLength=buffer->length();
  if (bufferLength > 0)
  {
    tempPacket.concat(*buffer);
    buffer->remove(0,bufferLength);
    // Analyze and/or modify packet
    switch (tempPacket[0]){
      case H0_KEYPRESS:       Serial.print("(KeyPress )");
                              if (bufferLength>=8)
                              {
                                byte key=tempPacket[5];
                                if ((KB_KEY_VENT & key) == KB_KEY_VENT) { Serial.print("[VENT]"); bValid=true;}
                                if ((KB_KEY_UP & key) == KB_KEY_UP)           { Serial.print("[UP]");   bValid=true;}
                                if ((KB_KEY_DOWN & key)== KB_KEY_DOWN)        { Serial.print("[DN]");   bValid=true;}
                                if ((KB_KEY_SET & key ) == KB_KEY_SET)        { Serial.print("[SET]");  bValid=true;}
                              }
                              else
                              {
                                Serial.print(" TOO SHORT");
                              }
                              Serial.println();
                              break;
      case H0_MVHR_MESSAGE:   Serial.print("(Display  )");
                              if (bufferLength>=41)
                              {
                                for (int i=6; i<22; i++)
                                { Serial.print(tempPacket[i]); }
                                Serial.print(" / ");
                                for (int i=23;i<39;i++)
                                { Serial.print(tempPacket[i]); }
                                bValid=true;
                              }
                              else
                              {
                                Serial.println(" TOO SHORT");
                              }
                              Serial.println();
                              break;

      default:
                              Serial.println("Unknown packet");
                              break;
    }
  if (not bValid)
  {
    Serial.print(" [ ");
    for (int i=0; i < bufferLength; i++)
    {
      Serial.print(tempPacket[i],HEX);
      Serial.print(" ");
    }
    Serial.println("]");
  }
  }
  // route the packet to receiver
  if (Type==MVHR_TO_KBD)
  {
   kbdSerial.print(tempPacket);
   kbdSerial.flush();
  }
  else
  {
   mvhrSerial.print(tempPacket);
   mvhrSerial.flush();
  }
}


void taskControlRuntime( void * pvParameters ){
  for(;;)
  {
    // sleep(1.1);
    // Serial.print("Control running on core ");  Serial.println(xPortGetCoreID());
    tCurrent=millis();
    if (tCurrent >tLast2+TMR_INTERVAL2)
    {
      tLast2=tCurrent;
    }
    if (tCurrent>tLast+TMR_INTERVAL)
    {
      // Serial.println("===");
      // tLast=tCurrent;
      // Serial.print("Time:        ");Serial.println(tCurrent);
      // Serial.print("[KBD_BUFF]   ");Serial.println(kbdBuff);
      // Serial.print("[KBD_CACHE]  ");Serial.println(kbdCache);
      // Serial.print("[MVHR_BUFF]  ");Serial.println(mvhrBuff);
      // Serial.print("[MVHR_CACHE] ");Serial.println(mvhrCache);
      // Serial.print("Free heap:   ");Serial.println(ESP.getFreeHeap());
    }
    if ((kbdNewData) && (tCurrent > tLastReceivedKbd+TMR_PACKET_END_SPACE))
    {
      std::lock_guard<std::mutex> lck(mtxKbdBuff);
      kbdBuff.concat(kbdCache);
      kbdCache="";
      kbdNewData=false;
      processPacket(KBD_TO_MVHR,&kbdBuff);
    }
    if ((mvhrNewData) && (tCurrent > tLastReceivedMvhr+TMR_PACKET_END_SPACE))
    {
      std::lock_guard<std::mutex> lck(mtxMvhrBuff);
      mvhrBuff.concat(mvhrCache);
      mvhrCache="";
      mvhrNewData=false;
      processPacket(MVHR_TO_KBD,&mvhrBuff);
    }
   // esp_task_wdt_reset();
  }
}


void setup() {
  Serial.begin(115200);                                                                 // log interface
  pinMode(KEYBOARD_RX_PIN,INPUT);
  pinMode(KEYBOARD_TX_PIN,OUTPUT);
  pinMode(MVHR_RX_PIN,INPUT);
  pinMode(MVHR_TX_PIN,OUTPUT);
  kbdSerial.begin(9600,SERIAL_8N1,KEYBOARD_RX_PIN,KEYBOARD_TX_PIN);     
  mvhrSerial.begin(9600,SERIAL_8N1,MVHR_RX_PIN,MVHR_TX_PIN);            
  xTaskCreatePinnedToCore(taskSerialRuntime, "Serial",10000,NULL,1,&taskSerial,0);      // this runs on core 0
  xTaskCreatePinnedToCore(taskControlRuntime, "Control",10000,NULL,1,&taskControl,1);   // this runs on core 1
}

void loop() {

}
