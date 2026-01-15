#include "UsbKeyboard.h"
#include <util/delay.h>


/*=============BASIC CONNECTION CONFIGURURATION========================*/
#define BT1_PIN A0  //LEFT HAND
#define BT2_PIN A1  //RIGHT HAND
#define BT3_PIN A2  //EXT

#define BT1_LED 6
#define BT2_LED 7

#define LED_R A3
#define LED_G A4
#define LED_B A5
/*=====================================================================*/


/*============DEVICE BEHAVIOR CONFIGURURATION==========================*/
#define BYPASS_PASSIVE_DELAY 50 // The asynchronous rolling delay(count by times)

#define BYPASS_TIMER_ISR 1 // BYPASS TIME CORRECTER

#define USB_ATTACH_FAIL_COUNT 50000 // The max retry time before reset when Failt to attached to USB HOST

//#define ENABLE_DEBUG_OUTPUT  //Close it when using, for it will reduce the SPEED of refrush(We need DMA QAQ)

#define BT1_PIN_BEHAVIOR KEY_Z
#define BT2_PIN_BEHAVIOR KEY_X
#define BT3_PIN_BEHAVIOR KEY_ESC
/*=====================================================================*/


/*============INITAL GLOBLE VARIABLES==================================*/
bool fore_KEY_BT1 = false;        //The variables to hold the previous status of the button
double fore_KET_BT1_RUNTIME = 0;  //For asynchronous delay, the rolling times when the event is triggered

bool fore_KEY_BT2 = false;
double fore_KET_BT2_RUNTIME = 0;

bool fore_KEY_BT3 = false;
double fore_KET_BT3_RUNTIME = 0;

int BT1_LED_ALPHA = 0;           //The present brightness of the LED onto the button
int BT1_LED_ALPHA_TARGET = 0;    //The target brightness of the LED onto the button
int BT2_LED_ALPHA = 0;
int BT2_LED_ALPHA_TARGET = 0;

// We use HUE instead of RGB to have better controlling of the colour

int LED_TARGET_H = 0;  	//Hues
int LED_PRESENT_H = 0;

int LED_TARGET_U = 0;  	//Saturation	
int LED_PRESENT_U = 0;

int LED_TARGET_E = 0;  	//Brightness		
int LED_PRESENT_E = 0;


uchar reportBuffer[4] = {0,0,0,0};  
/* Report Buffer Prefix
 reportBuffer[0]-> NULL -> 0
 reportBuffer[1]-> BT2 -> KEY_X
 reportBuffer[2]-> BT3 -> KEY_ESC
 reportBuffer[3]-> BT1-> KEY_Z
 */
 
 long loop_route_time = 0;  //For asynchronous delay, the rolling times
 
 bool KEY_CHANGE_REQUEST = false; //For acknowledge the event need to be updated
 
 bool BT_STATUS[3] = {1,1,1};
 
/*======================================================================*/

//*************************************************************************************************
void setup() {
  
//===============INIT THE SYSTEM PRINT OUT BOOT INFORMATION============
  Serial.begin(115200);

  Serial.println("=========================================================");
  Serial.println("     Arduino OSU Keyboard    Designed by MartinT3CH      ");
  Serial.println("          GITHUB https://github.com/MarsTechHAN          ");
  Serial.println("         Opensource under lincense GNU/GPL v3            ");
  Serial.println("            BUILD   "__DATE__" "__TIME__"                      ");
  Serial.println("=========================================================");
  
 //====================DEBUG OPTION-> ACKNOWLEDGEMENT===================
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.println("WARNING: DEBUG OUTPUT has been turned on, which may redu-");
  Serial.println("ce the performance! Disable #define ENABLE_DEBUG_OUTPUT !");
  Serial.println("");
  Serial.print(  "SYSTEM BOOTING.........................................");
#endif

//=======================INIT THE PIN====================================
  pinMode(BT1_PIN, INPUT_PULLUP);
  pinMode(BT2_PIN, INPUT_PULLUP);
  pinMode(BT3_PIN, INPUT_PULLUP);

  pinMode(BT1_LED , OUTPUT);
  pinMode(BT2_LED , OUTPUT);

  pinMode(LED_R , OUTPUT);
  pinMode(LED_G , OUTPUT);
  pinMode(LED_B , OUTPUT);

  analogWrite(BT1_LED, 255);
  analogWrite(BT2_LED, 255);

  analogWrite(LED_R, 255);
  analogWrite(LED_G, 255);
  analogWrite(LED_B, 255);

 //====================DISABLE THE TIMER0=============================
#if BYPASS_TIMER_ISR
  // disable timer 0 overflow interrupt (used for millis)
  TIMSK0 &= !(1 << TOIE0); // ++
#endif                 

//=====================SELF DIGNOISIS(OPTIONAL)=========================
 int fail_count = 0;
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.println("OK");
  Serial.print("USB ATTACHED.........................................");
  while(!usbInterruptIsReady()){
	UsbKeyboard.update();   
    fail_count++;
	delayMs(1);
    if(fail_count >= USB_ATTACH_FAIL_COUNT) {
      Serial.println("FAIL");
      Serial.println("CRITICAL ERROR: UNABLE TO ATACHED USB, REBOOT IN 3s");
      delayMs(3000);
      asm volatile ("jmp 0");
    }
  }
  Serial.println("..OK");
  Serial.println();
#else
  while(!usbInterruptIsReady()){
	UsbKeyboard.update();   
    fail_count++;
	delayMs(1);
    if(fail_count >= USB_ATTACH_FAIL_COUNT) {
      delayMs(3000);
      asm volatile ("jmp 0");
    }
  }
#endif


}

//*************************************************************************************************
#if BYPASS_TIMER_ISR
void delayMs(unsigned int ms) {
  /*
  */
  for (int i = 0; i < ms; i++) {
    delayMicroseconds(1000);
  }
}
#endif
//*************************************************************************************************


//*************************************************************************************************
void loop() {
  
  UsbKeyboard.update();

  ++loop_route_time;
  
  BT_STATUS[0] = digitalRead(BT1_PIN);
  BT_STATUS[1] = digitalRead(BT2_PIN);
  BT_STATUS[2] = digitalRead(BT3_PIN);

  if (fore_KEY_BT1 == false && BT_STATUS[0] == false) {
    fore_KET_BT1_RUNTIME = loop_route_time;
    reportBuffer[3] = KEY_Z;
    fore_KEY_BT1 = true;
    KEY_CHANGE_REQUEST = true;
#ifdef ENABLE_DEBUG_OUTPUT
    Serial.println("BT1_PRESSED_DOWN_EVENT TRIGGERED");
    Serial.println("reportBuffer 3 LOADED -> KEY_Z");
    Serial.print("BT1 EVENT LOG RUNTIME:"); 
    Serial.println(loop_route_time);
    Serial.println("USB STATUS UPDATA FLAG SET");
    Serial.println("");
#endif

  }

  if (fore_KEY_BT2 == false && BT_STATUS[1] == false) {
    fore_KET_BT2_RUNTIME = loop_route_time;
    reportBuffer[1] = KEY_X;
    fore_KEY_BT2 = true;
    KEY_CHANGE_REQUEST = true;

#ifdef ENABLE_DEBUG_OUTPUT
    Serial.println("BT2_PRESSED_DOWN_EVENT TRIGGERED");
    Serial.println("reportBuffer 1 LOADED -> KEY_X");
    Serial.print("BT2 EVENT LOG RUNTIME:"); 
    Serial.println(loop_route_time);
    Serial.println("USB STATUS UPDATA FLAG SET");
    Serial.println("");
#endif

  }

  if (fore_KEY_BT3 == false && BT_STATUS[2] == false) {
    fore_KET_BT3_RUNTIME = loop_route_time;
    reportBuffer[2] = KEY_ESC;
    fore_KEY_BT3 = true;
    KEY_CHANGE_REQUEST = true;

#ifdef ENABLE_DEBUG_OUTPUT
    Serial.println("BT3_PRESSED_DOWN_EVENT TRIGGERED");
    Serial.println("reportBuffer 2 LOADED -> KEY_ESC");
    Serial.print("BT3 EVENT LOG RUNTIME:"); 
    Serial.println(loop_route_time);
    Serial.println("USB STATUS UPDATA FLAG SET");
    Serial.println("");
#endif
  }



  if (fore_KEY_BT1 == true && BT_STATUS[0] == true) {
#ifdef ENABLE_DEBUG_OUTPUT
    Serial.println("BT1_RELEASED_EVENT TRIGGERED");
    Serial.print("BYPASS_PASSIVE_LOOP :");
    Serial.println(abs(fore_KET_BT1_RUNTIME - loop_route_time));
#endif
    if (loop_route_time - fore_KET_BT1_RUNTIME > BYPASS_PASSIVE_DELAY) {
      reportBuffer[3] = 0;
      fore_KEY_BT1 = false;
      KEY_CHANGE_REQUEST = true;
#ifdef ENABLE_DEBUG_OUTPUT
      Serial.println("reportBuffer 3 LOADED -> KEY_NONE");
      Serial.println("USB STATUS UPDATA FLAG SET");
      Serial.println("");
#endif
    }
    else{
      fore_KET_BT1_RUNTIME = loop_route_time;
#ifdef ENABLE_DEBUG_OUTPUT
      Serial.println("BYPASS_PASSIVE_LOOP NOT REACH CRITICAL");
      Serial.println("EVENT INGORED");
      Serial.println("");
#endif
    }
  }

  if (fore_KEY_BT2 == true && BT_STATUS[1] == true) {
#ifdef ENABLE_DEBUG_OUTPUT
    Serial.println("BT2_RELEASED_EVENT TRIGGERED");
    Serial.print("BYPASS_PASSIVE_LOOP :");
    Serial.println(abs(fore_KET_BT2_RUNTIME - loop_route_time));
#endif
    if (loop_route_time - fore_KET_BT2_RUNTIME > BYPASS_PASSIVE_DELAY) {
      reportBuffer[1] = 0;
      fore_KEY_BT2 = false;
      KEY_CHANGE_REQUEST = true;
#ifdef ENABLE_DEBUG_OUTPUT
      Serial.println("reportBuffer 1 LOADED -> KEY_NONE");
      Serial.println("USB STATUS UPDATA FLAG SET");
      Serial.println("");
#endif
    }
    else{
      fore_KET_BT2_RUNTIME = loop_route_time;
#ifdef ENABLE_DEBUG_OUTPUT
      Serial.println("BYPASS_PASSIVE_LOOP NOT REACH CRITICAL");
      Serial.println("EVENT INGORED");
      Serial.println("");
#endif
    }
  }

  if (fore_KEY_BT3 == true && BT_STATUS[2] == true) {
#ifdef ENABLE_DEBUG_OUTPUT
    Serial.println("BT3_RELEASED_EVENT TRIGGERED");
    Serial.print("BYPASS_PASSIVE_LOOP :");
    Serial.println(abs(fore_KET_BT3_RUNTIME - loop_route_time));
#endif
    if (loop_route_time - fore_KET_BT1_RUNTIME > BYPASS_PASSIVE_DELAY) {
      reportBuffer[2] = 0;
      fore_KEY_BT3 = false;
      KEY_CHANGE_REQUEST = true;
#ifdef ENABLE_DEBUG_OUTPUT
      Serial.println("reportBuffer 2 LOADED -> KEY_NONE");
      Serial.println("USB STATUS UPDATA FLAG SET");
      Serial.println("");
#endif
    }
    else{
      fore_KET_BT3_RUNTIME = loop_route_time;
#ifdef ENABLE_DEBUG_OUTPUT
      Serial.println("BYPASS_PASSIVE_LOOP NOT REACH CRITICAL");
      Serial.println("EVENT INGORED");
      Serial.println("");
#endif
    }
  }

  if (KEY_CHANGE_REQUEST) {
    
    if (!usbInterruptIsReady()) 
    {
#ifdef ENABLE_DEBUG_OUTPUT
      Serial.println("USB INTERRUPT NOT READY YET, LOOP!");
#endif
      return;
    }
    usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
    KEY_CHANGE_REQUEST = false;
    loop_route_time = loop_route_time + 100;
#ifdef ENABLE_DEBUG_OUTPUT
    Serial.println("DATA PACKAGED READY TO SEND, WAIT FOR INTERRUPT");
    Serial.println("USB STATUS UPDATA FLAG CLEAR");
    Serial.println("");
#endif
  }else{delayMicroseconds(100);}
  
}
//*************************************************************************************************
