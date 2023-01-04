/* Tom's Remote*/
byte localAddress = 0xB0;     // address of this device E1-E4
char BoatLetters[2] = "ST";  //Boatletters String
int mode = 1; //1 5R, 2 5U, 3 R, 4 3U, 4 9U, 6 ST, 7 ET
int dmode = 1; //initally we don't know the mode of the display
int rmode = 1; //retry mode change
int stat = 0; // 0 stoped 1 running
int dstat = 0; //set to zero unknown

int tm = -300; //tm = negative for start positive for race
const unsigned long period = 100; // weonly do seconds her
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
int sec;
int  hourU=0; int  minT=0; int  minU=0; int  secT=0; int  secU=0; String tmTxt= "0:00:00";


#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;        // LoRa radio reset
const int irqPin = 2;          // change for your board; must be a hardware interrupt pin

String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
byte destination = 0xBB;      // destination to send to **BB is startboat
long lastSendTime = 0;        // last send time
long lastSendTimeFinish = 0;        // last finish send time
int interval = 900;          // interval between sends

//Fuck the LED got an LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

//unsigned long countnumber;

// Variables to store individual numbers
int  hourUnit=0;
int  minTens=0;
int  minUnits=0;
int  secTens=0;
int  secUnits=0;
int  hundredSec=0;
int  sevennum=0;
int  eightnum=0;

//#include <TimeAlarms.h>
//#include <avr/wdt.h> //Watchdoog Timer

const long SPT = 1200; // New 1.2 seconds 
int lState1 = 1;  // the previous state from the input pin
int cState1;     // the current reading from the input pin
unsigned long pTime1  = 0;
unsigned long rTime1 = 0;
int lState2 = 1;  // the previous state from the input pin
int cState2;     // the current reading from the input pin
unsigned long pTime2  = 0;
unsigned long rTime2 = 0;
int lState3 = 1;  // the previous state from the input pin
int cState3;     // the current reading from the input pin
unsigned long pTime3  = 0;
unsigned long rTime3 = 0;

int Button1 = A1;       //far right header A1 GND 3.3v     MODE      
                                      
int Button2 = A0;       //second header A0 GND 3.3v        START
int Button3 = 4;       //third header D4 GND 3.3v          
int Button4 = 3;       //fourth header D3 GND 3.3v
int Button5 = 5;       //bottom outer solder (square)
int Button6 = 6;       //seconed to bottom outer solder (round) 

bool RaceTime = false; // Show race time elapsed.
bool Standby = false;
bool FinishTimeStored = false; //Has Finish Time Pressed
bool FinishTimeToSend = false; //Has Finish Time need to send Pressed
bool StartSequenceActive = false; //Start sequence Currently Active

const byte numBoats = 6; //Number of Boats in the Race

bool BoatSignOnStatus[numBoats] = {false,false,false,false,false,false}; //Position 0 is E1 ect

long HornTime = 0; //Horn Time
long HootTime = 1000;// Time that Hoot will sound in auto modes
long RaceTimeClock = 0; //Time Elapsed Since Race Start
long RaceClockRecieveTime = 0; //Time Recieve Message
long RaceClockRecieveTimeTemp = 0; //Time Recieve message unverified.
long RecievedRaceTimefromSB = 0; // Time Recieved from StartBoat via LoRa
long FinishTime = 0; //Finish time from button 6 press

String HootMessage; // New Horn Message
String ModeMessage; // New Mode Message
String StartMessage; // New Mode Message
String FinishMessage; // Finish time message
String SignOnMessage; // SignOn message
String Start5min; // SignOn message
String Start3min; // SignOn message
String Reset; // SignOn message

//Read String From Startboat

const byte RecChars = 48;        //MAX Recieve String Lenght Make Sure you dont go longer then this weird Shit Happens
const byte numChars = 24;
char receivedChars[RecChars];
char tempChars[RecChars];        // temporary array for use when parsing
String tempincoming = "";
//char incoming[numChars] = {0};

      // variables to hold the parsed data
char messageRecieved[RecChars] = {0};
//message format <messageHedder:messageData1:messageData2>
char messageHedder[numChars] = {0};  //LoRa String Hedder message format <messageHedder:messageData1:messageData2>
char messageData1[numChars] = {0};    //LoRa String Data1
char messageData2[numChars] = {0};    //LoRa String Data2
char RaceSeq[numChars] = {0};
//float RaceTimeClock = 0.0;

boolean newData = false;

void setup() {

    pinMode(Button1, INPUT_PULLUP);
    pinMode(Button2, INPUT_PULLUP);
    pinMode(Button3, INPUT_PULLUP);
    pinMode(Button4, INPUT_PULLUP);
    pinMode(Button5, INPUT_PULLUP);
    pinMode(Button6, INPUT_PULLUP);
  
  Serial.begin(9600);                   // initialize serial
  while (!Serial);

  Serial.println("LoRa Duplex");

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  if (!LoRa.begin(915E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  Serial.println("LoRa init succeeded.");
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("oceanhippie.net");
  lcd.setCursor(2,1);
  delay(1000);
  lcd.print("startOmatic");
  lcd.setCursor(0,0);
  lcd.print(BoatLetters);
  lcd.setCursor(2,0);
  lcd.print(":             ");
  lcd.setCursor(11,0);
  lcd.print("11--0");
 
          HootMessage = "<Hoot:";
          HootMessage += BoatLetters[0];
          HootMessage += BoatLetters[1];
          HootMessage += ":";
          HootMessage += "111111";
          HootMessage += ">"; //Message End

  //LLEGACY MESSAGES
            //Build SignOn string to send, <BoatSignOn:E1:000000>
          SignOnMessage = "<BoatSignOn:";
          SignOnMessage += BoatLetters[0];
          SignOnMessage += BoatLetters[1];
          SignOnMessage += ":";
          SignOnMessage += "000000";
          SignOnMessage += ">"; //Message End

            //Build SignOn string to send, <BoatSignOn:E1:000000>
          Start5min = "<Start5m:SB:111111>";
          //build FAKE String to trip StartOMatic
          Start5min = "<RaceActive:FF:0000000>";
            
            //Build SignOn string to send, <BoatSignOn:E1:000000>
          Start3min = "<Start3m:SB:111111>";
                      //Build SignOn string to send, <BoatSignOn:E1:000000>
          Start3min = "<Start3m:SB:111111>";
      
            
            //Build SignOn string to send, <BoatSignOn:E1:000000>
          Reset = "<Reset:";
          Reset += BoatLetters[0];
          Reset += BoatLetters[1];
          Reset += ":";
          Reset += "000000";
          Reset += ">"; //Message End
  
}

void loop() {

//horn is ALWAYS availabe
  if(digitalRead(Button4)  == LOW) { Serial.println("Horn Sent"); sendMessage(HootMessage); }
 // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
 //local v remote mode 
 if(mode != dmode && millis() - lastSendTime > interval) {
     Serial.print(mode); Serial.print(dmode);
     Serial.println("Remote Apparently Wrong");
     bmm();   sendMessage(ModeMessage);
     lastSendTime = millis();   
 } else { 
    //OK make sure the remoteis doing what we are
  if(stat != dstat) {
       //Serial.println("Remote Staus Wrong");
     //sendMessage(ModeMessage); stuff goes here.
  }
 }
  //set the mode lonmg press
cState1 = digitalRead(Button1);
  if(lState1 == 1 && cState1 == 0)        // button is pressed wasn
    pTime1 = millis();
  if(lState1 == 0 && cState1 == 1) { // button is released
     rTime1 = millis();
    long pDuration = rTime1 - pTime1;
    if( pDuration > SPT ) {
      Serial.println(" Changing modes");
      mode = mode+1;
      if(mode>7) mode=1; 
      bmm();   sendMessage(ModeMessage);
        lcd.setCursor(11,0);
        lcd.print(mode);      
    }
  }
  // save the the last state1
lState1 = cState1;
//end set mode

//do the startfinish button
if (stat!=0) { //running
    if(digitalRead(Button2)  == LOW) { //we're running jsut send a finish
      if(mode==7 && FinishTimeToSend == false) {  mattFinishTime(); } else { Serial.println("Finish Sent");sendMessage(FinishMessage);}
 }  
} else { //not running send start
// need a long press  
cState2 = digitalRead(Button2);
  if(lState2 == 1 && cState2 == 0)        // button is pressed wasn
    pTime2 = millis();
  if(lState2 == 0 && cState2 == 1) { // button is released
     rTime2 = millis();
    long pDuration = rTime2 - pTime2;
    if( pDuration > SPT ) {
      Serial.println(" Sending start");
      bsm(); sendMessage(StartMessage);      
    }
  }
    // save the the last state1
lState2 = cState2;
} //end if running

//reset avaialbe all the time
cState3 = digitalRead(Button3);
  if(lState3 == 1 && cState3 == 0)        // button is pressed wasn
    pTime3 = millis();
  if(lState3 == 0 && cState3 == 1) { // button is released
     rTime3 = millis();
    long pDuration = rTime3 - pTime3;
    if( pDuration > SPT ) {
      Serial.println(" Reset");
      Serial.println(Reset);
      sendMessage(Reset);
    }
  }
  // save the the last state3
  lState3 = cState3;
   
   if (stat==1) { //we're running do stuff
  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= period)  //test whether the period has elapsed
  {
    if(mode==2 && tm==0) tm=-300; //deal with repeats 
    if(mode==4 && tm==0) tm=-180; //deal with repeats
  lcd.setCursor(0,1);
  bitTime();
  lcd.print(tmTxt);
        ++tm;
       startMillis = currentMillis;  //IMPORTANT to save the start time of the current LED state.
       
  }
   }

//OLD CODE---------------------------------------------------------------
  
//Boat Sign On requred 

  if (millis() - lastSendTime > interval && BoatSignOnStatus[(BoatLetters[1] -'0') -1] == false  && Standby == true) {  //Signon When Start timer is in Standy
    sendMessage(SignOnMessage);
    Serial.println("Sending " + SignOnMessage);
    lastSendTime = millis();            // timestamp the message
    interval = random(1500) + 700;    // 0.7-2.2 seconds
  }
/* Duplicate Tom
//if(digitalRead(Button1)  == LOW){
// sendMessage(Start5min);
//    Serial.println("Sending " + Start5min); 
//}
//if(digitalRead(Button2)  == LOW){
// sendMessage(Start3min);
//    Serial.println("Sending " + Start5min); 
//}

if(digitalRead(Button1)  == LOW){
 sendMessage(Start5min);
    Serial.println("Sending " + Start5min); 
}

 
   if(digitalRead(Button3)  == LOW)
   {
    Serial.println("Button3");
    Serial.println("Sequence Cancel");
    RaceTime = false;
    RaceTimeClock = 0; //Zero Race Clock
     sendMessage(Reset);
    Serial.println("Sending " + Reset); 
   }
*/
    if(RaceTime == true || FinishTimeStored == true)   // Sequence Active Print Display
      {
        printtime(); // Display Countdown/Elapsed Race Time
        //Serial.println("Update Display");
      } 
        else{                                 // No Sequence Active Blank Display
                  if (BoatSignOnStatus[0] == true){
                  //lc.setChar(0, 0,  1, false);
                          lcd.setCursor(3,0);
                          lcd.print("S");
                }
                else{
                  //lc.setChar(0, 0,  0x20, false);
                                    //lc.setChar(0, 0,  1, false);
                          lcd.setCursor(3,0);
                          lcd.print("N");
                }    
                if (BoatSignOnStatus[1] == true){
                  //lc.setChar(0, 1,  2, false);
                          lcd.setCursor(4,0);
                          lcd.print("S");
                }
                else{
                  //lc.setChar(0, 1,  0x20, false); 
                  lcd.setCursor(4,0);
                  lcd.print("N"); 
                }
                if (BoatSignOnStatus[2] == true){
                  //lc.setChar(0, 2,  3, false);
                          lcd.setCursor(5,0);
                          lcd.print("S");
                }
                else{
                  //lc.setChar(0, 2,  0x20, false);
                  lcd.setCursor(5,0);
                  lcd.print("N");
                }
                if (BoatSignOnStatus[3] == true){
                  //lc.setChar(0, 3,  4, false);
                         lcd.setCursor(6,0);
                          lcd.print("S");
                }
                else{
                  //lc.setChar(0, 3,  0x20, false);
                  lcd.setCursor(6,0);Nice one!
                  lcd.print("N");
                }
                if (BoatSignOnStatus[4] == true){
                  //lc.setChar(0, 4,  5, false);
                          lcd.setCursor(7,0);
                          lcd.print("S");
                }
                else{
                  //lc.setChar(0, 4,  0x20, false);
                  lcd.setCursor(7,0);
                          lcd.print("N");
                }
                if (BoatSignOnStatus[5] == true){
                  //lc.setChar(0, 5, 6, false);
                          lcd.setCursor(8,0);
                          lcd.print("S");

                }
                else{
                  //lc.setChar(0, 5, 0x20, false);
                                    //lc.setChar(0, 5, 6, false);
                          lcd.setCursor(8,0);
                          lcd.print("N");
                }
             
             //lc.setChar (0, 7, BoatLetters[0] , false); //Boat Number Display
             //lc.setChar (0, 6, BoatLetters[1] , true); //Boat Number Display
}


}

void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize) {

  if (packetSize == 0) return;          // if there's no packet, return
  
  RaceClockRecieveTimeTemp = millis(); //Message Recieved Time
  Serial.print("Message Recieve time millis: ");
  Serial.println(RaceClockRecieveTimeTemp);
  
  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";
  //char incoming[numChars] = {0};

  
  while (LoRa.available()) {
    incoming += (char)LoRa.read();  
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  //if (recipient != localAddress && recipient != 0xFF) {
 //   Serial.println("This message is not for me.");
 //   return;                             // skip rest of function
 // }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();

  //strcpy(tempChars, incoming.c_str());
  Serial.print("Message Recieve time millis: ");
  Serial.println(RaceClockRecieveTimeTemp);
  
  incoming.toCharArray(tempChars,incomingLength+1);
  parseData();
}

void parseData() {      // split the data into its parts
    
    char * strtokIndx; // this is used by strtok() as an index
    char messageHedderTemp[numChars];

    strtokIndx = strtok(tempChars,":");      // get the first part - the string
    //strcpy(messageRecieved, strtokIndx); // copy it to messageRecieved
    strcpy(messageHedderTemp, strtokIndx);
    if(messageHedderTemp[0] == 0x3c){ // Charecter is <
       char * messageHedder2 = messageHedderTemp + 1;
       strcpy(messageHedder, messageHedder2);
      }
 
    strtokIndx = strtok(NULL,":");      // get the second part - the string
    //strcpy(RaceSeq, strtokIndx);     // convert this part to an integer
    strcpy(messageData1, strtokIndx);
    
    strtokIndx = strtok(NULL, ":");                //Get the third part of the string
    //RecievedRaceTimefromSB = atol(strtokIndx);     // Change Race Time into unsigned long for work
    strcpy(messageData2, strtokIndx);
    
    byte lastChar = strlen(messageData2)-1; // Get the position of the last char
    if(messageData2[lastChar] == 0x3e){ // Charecter is >
     messageData2[lastChar] = '\0'; //replace it with a NULL 
    }
     
showParsedData();
}

void showParsedData() {  
    Serial.print("Message Hedder: ");
    Serial.println(messageHedder);
    Serial.print("Message Data1: ");
    Serial.println(messageData1);
    Serial.print("Message Data2: ");
    Serial.println(messageData2);
    Serial.println();
   
 //Check Stats of Race
 //String messageData1Temp(messageData1);
 //String messageData2Temp(messageData2);
if(strcmp(messageHedder, "Mode")==0) // Reply to changepode Message
  {
      String messageData2Temp(messageData2);
    dmode = messageData2Temp.toInt();
            lcd.setCursor(12,0);
        lcd.print(dmode); 
    Serial.println(" Remote Changed mode to" + dmode);
    }
    
if(strcmp(messageHedder, "Time")==0) // Reply to changepode Message
  {
      String messageData2Temp(messageData2);
    tm  = messageData2Temp.toInt();
    bitTime();
    stat=1;
        lcd.setCursor(0,1);
        lcd.print(tmTxt); 
    Serial.print(tm);
    Serial.println(" Time recieved");
    }
    
 if(strcmp(messageHedder, "Reset")==0) // Recive Reset from MW
  {
    Serial.println("Reset Recived");
    stat=0; //stop
    if(mode<=2) tm=-300; //541go RESET
    if(mode==3 || mode==4) tm=-180; //341Go RESET
    }

if(strcmp(messageHedder, "RaceActive")==0) // Clock Message
  {
    Serial.println(strcmp(messageHedder, "RaceActive"));
    RaceTime = true;     // Display Race Time
    Standby = false;

  
    
    RecievedRaceTimefromSB = atol(messageData2); // Convert Time to Unsigned Long
    RaceClockRecieveTime = RaceClockRecieveTimeTemp;
    /*Serial.print("RecievedRaceTimefromSB: ");
    Serial.println(RecievedRaceTimefromSB);
    Serial.print("RaceClockRecieveTimeTemp: ");
    Serial.println(RaceClockRecieveTimeTemp);
    Serial.print("RaceClockRecieveTime: ");
    Serial.println(RaceClockRecieveTime);
    Serial.println("Race time: true"); //Debug Line
    */
  }
//<FinishTimeRec:E1:308294> finish time confirm recieved.
if((strcmp(messageHedder,"FinishTimeRec")==0))
  {
  Serial.println("Finish Time Reply Recieved from Startboat");
  long RecievedRaceTimefromSBTemp = atol(messageData2);
    if(RecievedRaceTimefromSBTemp == FinishTime){
      FinishTimeToSend = false; //Boat Signed On
  }
}


  
if ((strcmp(messageHedder, "Standby")==0)){
  if(Standby == false){
      //BoatSignOnStatus[(BoatLetter2 -'0') -1] = false; //sign on startboat has restrated.
    }
    
  Standby = true;
  RaceTime = false;
  FinishTimeStored = false;
  Serial.println("Standby Flags Cleared"); //Debug Line 
}
//<SignOnRecieved:E1:111111> Sign on Confirmed Recieved
//<SignOnRecieved:E1:111111>
if((strcmp(messageHedder, "SignOnRecieved")==0) && (strcmp(messageData1, BoatLetters)==0) && (strcmp(messageData2, "111111")==0)) 
  {
    BoatSignOnStatus[(BoatLetters[1] -'0') -1] = true; //Boat Signed On
  }

  if((strcmp(messageHedder, "Standby")==0) && strcmp(messageData1, "Standby")==0) 
  {
    if(messageData2[(BoatLetters[1] - '0') - 1] == 0x30){
    BoatSignOnStatus[(BoatLetters[1] -'0') -1] = false; //Boat not Signed On
    Serial.println("Boat not Currently Signed ON"); 
    }
       int i = 0;
       int j = numBoats;
       while (i < j){
        if (messageData2[i] == 0x30){ //equal to 0
        BoatSignOnStatus[i] = false;
        }
        if (messageData2[i] == 0x31){ //equal to 1
        BoatSignOnStatus[i] = true;
        }
       i++;
       }
  }


}

void printtime(){  //Print Time in Serial and on Dispaly
  long DisplayClock = 0; //Time to Display
  /*Serial.print("Current millis()=");
  Serial.println(millis());
  Serial.print("Message Recieve Time =");
  Serial.println(RaceClockRecieveTime);
  Serial.print("Current Recieve time from SB=");
  Serial.println(RecievedRaceTimefromSB);  
  */
  if (RaceTime){
    RaceTimeClock = (RecievedRaceTimefromSB) +(millis()-RaceClockRecieveTime);
    if (RaceTimeClock < 0){ //Race time is - then the race is in Countdown
      DisplayClock = RaceTimeClock * -1;
      StartSequenceActive = true;
    }
    else{             //if recieved time is + then the race has started
      //RaceTimeClock = (RecievedRaceTimefromSB) +(millis()-RaceClockRecieveTime); 
      StartSequenceActive = false;
      DisplayClock = RaceTimeClock;
    }
    
 /*   Serial.print("millis(");
    Serial.println(millis());
    Serial.print("RaceClockRecieveTime(");
    Serial.println(RaceClockRecieveTime);
    Serial.print("millis()-RaceClockRecieveTime(");
    Serial.println(millis()-RaceClockRecieveTime);
    Serial.print("DisplayClock(");
    Serial.println(DisplayClock);
    */
  }
  if (FinishTimeStored){
    DisplayClock = FinishTime; // Display Finish Time
    
  }
  hourUnit = ((DisplayClock / 1000)/3600) % 24;
  minTens =  (((DisplayClock / 1000)/60) % 60)/10;
  minUnits = ((DisplayClock / 1000)/60) % 10;
  secTens =  ((DisplayClock / 1000)%60) / 10;
  secUnits = DisplayClock / 1000 % 10;
  hundredSec = DisplayClock / 100 % 10;
  
  if (hourUnit == 0 || (FinishTimeToSend == false && FinishTimeStored == true)){
    if(FinishTimeToSend == false && FinishTimeStored == true){
    //lc.setChar(0, 5, 'F', false);  //Display F to Confirm Finish
    }
    else{
     //lc.setChar(0, 5, ' ', false); 
    }
    
      //lc.setDigit(0, 4, minTens, false);
      //lc.setDigit(0, 3, minUnits, true);
      //lc.setDigit(0, 2, secTens, false);
      //lc.setDigit(0, 1, secUnits, true);
                          lcd.setCursor(0,1);
                          lcd.print(DisplayClock);
  }
  else{
    if(FinishTimeToSend == false && FinishTimeStored == true){ //Display F to Confirm Finish
      //lc.setChar(0, 5, 'F', false);  
      }
    else{
                          lcd.setCursor(0,1);
                          lcd.print(DisplayClock);
     //lc.setDigit(0, 5, hourUnit, true); 
     }
    //lc.setDigit(0, 4, minTens, false);
    //lc.setDigit(0, 3, minUnits, true);
    //lc.setDigit(0, 2, secTens, false);
    //lc.setDigit(0, 1, secUnits, true);
                          lcd.setCursor(0,1);
                          lcd.print(DisplayClock);
  }
  //lc.setDigit(0, 0, hundredSec, false);
  //Print Boat Letters
  //lc.setChar (0, 7, BoatLetters[0] , false); //Boat Number Display
  //lc.setChar (0, 6, BoatLetters[1] , true); //Boat Number Display
  //print in serial for debug
  Serial.print(hourUnit);
  Serial.print(':');
  Serial.print(minTens);
  Serial.print(minUnits);
  Serial.print(':');
  Serial.print(secTens);
  Serial.print(secUnits);
  Serial.print(':'); 
  Serial.println(hundredSec);

}

void bmm() {
          ModeMessage = "<Mode";
          ModeMessage += ":";
          ModeMessage += BoatLetters[0];
          ModeMessage += BoatLetters[1];
          ModeMessage += ":";
          ModeMessage += mode;
          ModeMessage += ">"; //Message End
}
void bsm() {
          if(mode==1 || mode ==2) {StartMessage = "<Start5m:";} else { StartMessage = "<Start3m:"; } 
          StartMessage += BoatLetters[0];
          StartMessage += BoatLetters[1];
          StartMessage += ":";
          StartMessage += "111111";
          StartMessage += ">"; //Message End 
}
void mattFinishTime() {
          {
        //Get Current Race time and store
        if(FinishTimeStored == false){ //Store on beginning of press only
          FinishTime = RaceTimeClock;
          FinishTimeStored = true;
          FinishTimeToSend = true; 
          //Build string to send, <FinishTime:E1:1378212>
          FinishMessage = "<FinishTime:";
          FinishMessage += BoatLetters[0];
          FinishMessage += BoatLetters[1];
          FinishMessage += ":";
          FinishMessage += FinishTime;
          FinishMessage += ">"; //Message End
          }
    Serial.println("Button6");
    Serial.print("Post Finish Time ");
    Serial.print(BoatLetters[0]);
    Serial.print(BoatLetters[1]);
    Serial.print(':');
    Serial.println(FinishTime);
    Serial.println(FinishMessage);
    }
//Send Finish Time
      if (millis() - lastSendTimeFinish > interval && FinishTimeStored == true && FinishTimeToSend == true) {  //Signon When Start timer is in Standy
    sendMessage(FinishMessage);
    Serial.println("Sending " + FinishMessage);
    lastSendTimeFinish = millis();            // timestamp the message
    interval = random(1500) + 700;    // 0.7-2.2 seconds
  }
}
void bitTime() {
  //makes time for dispalaying
  if(tm < 0) {sec = tm * -1;} else {sec=tm;}
  Serial.print(tm);
  hourU = (sec /3600) % 24;
  minT =  ((sec /60) % 60)/10;
  minU = (sec /60) % 10;
  secT =  (sec %60) / 10;
  secU = sec % 10;

  tmTxt = hourU;
          tmTxt += ":";
          tmTxt += minT;
          tmTxt += minU;
          tmTxt += ":";
          tmTxt += secT;
          tmTxt += secU; //Message End
  Serial.print(" Time: ");
  Serial.println(tmTxt);

}
