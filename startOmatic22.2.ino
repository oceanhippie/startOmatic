int cnt = 0; //how many loops before we send an update to time
/* loRa Drubbger */
byte localAddress = 0xB1;     // address of this device E1-E4
char BoatLetters[2] = "MW";  //Boatletters String
int mode = 1; //1 5R, 2 5U, 3 R, 4 3U, 6 ST, 7 ET
int disp =1;  //waht mode to put he display in
int stat = 0; //not running
String ModeMessage; // New Mode Message
String TimeMessage; // new Time Message
String HootMessage; // new Time Message
//new timer

int tm = -300; //tm = negative for start positive for race
const unsigned long period = 847; // weonly do seconds her
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
int sec;
int hourT=0; int hourU=0; int  minT=0; int  minU=0; int  secT=0; int  secU=0; String tmTxt= "0:00:00"; 
int minUP =0; int  minTP=0; int; int hourUP=0; int hourTP=0; int secTP=0;
byte flip; //for flashing the :

#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>
#include <fonts/Arial_Black_16.h>
#include <fonts/imp36.h>
#include <fonts/Droid_Sans_12.h>
#include <fonts/Droid_Sans_24.h>
#include <NMEAGPS.h>
#include <GPSport.h>
#include <Streamers.h>

//Fuck the LED got an LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;        // LoRa radio reset
const int irqPin = 2;          // change for your board; must be a hardware interrupt pin

//DMD for the big display DMD Pins for MEGA
#define pin_a 22 //was D6 A      
#define pin_b 24 //was D7 B     
#define pin_sck 26 //was D8 sclk      
#define pin_noe 28 // was D9 nOE      
#define pin_r_data 30 // was d11 R      
#define pin_clk 32 //was 13 CLK     

SoftDMD dmd( 2, 2,  pin_noe,  pin_a,  pin_b,  pin_sck,  pin_clk,  pin_r_data);
// GPS
static const int32_t          zone_hours   = +9L; // DARWIN 9.5
static const int32_t          zone_minutes =  30L; // usually zero 30 for Darwin 9.5
static const NeoGPS::clock_t  zone_offset  =
  zone_hours   * NeoGPS::SECONDS_PER_HOUR +
  zone_minutes * NeoGPS::SECONDS_PER_MINUTE;
String rhoursTens, rhoursUnits, rminsTens, rminsUnits, rsecsTens, rsecsUnits;
static NMEAGPS  gps;
static gps_fix  fix;

String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
byte destination = 0xBB;      // destination to send to **BB is startboat
long lastSendTime = 0;        // last send time
long lastSendTimeFinish = 0;        // last finish send time
int interval = 2000;          // interval between sends

int Button1 = A0;                                                        
int Button2 = A1;
int Button3 = A2;
int Button4 = A3;
int Button5 = A4;
int Button6 = A5;

bool Standby = false;
const byte numBoats = 6; //Number of Boats in the Race

bool BoatSignOnStatus[numBoats] = {false,false,false,false,false,false}; //Position 0 is E1 ect


String FinishMessage; // Finish time message
String SignOnMessage; // SignOn message
String Start5min; // SignOn message
String Start3min; // SignOn message
String Reset; // SignOn message

//Read String From Startboat

const byte numChars = 42;        //MAX Recieve String Lenght Make Sure you dont go longer then this weird Shit Happens
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing
String tempincoming = "";
//char incoming[numChars] = {0};

      // variables to hold the parsed data
char messageRecieved[numChars] = {0};
//message format <messageHedder:messageData1:messageData2>
char messageHedder[numChars] = {0};  //LoRa String Hedder message format <messageHedder:messageData1:messageData2>
char messageData1[numChars] = {0};    //LoRa String Data1
char messageData2[numChars] = {0};    //LoRa String Data2
char RaceSeq[numChars] = {0};
//float RaceTimeClock = 0.0;

boolean newData = false;

void setup() {

  // Setpins
  pinMode(40, OUTPUT);
  digitalWrite(40, LOW); //turn off horn
  pinMode(A0, INPUT_PULLUP); //RESET - RUN SWITCH
  pinMode(A1, INPUT_PULLUP); //541GO / SternChaser SWITCH
  pinMode(A2, INPUT_PULLUP); //HORN OUT
  pinMode(A3, INPUT_PULLUP); //AUTO / MANUAL SWITCH
  pinMode(A4, INPUT_PULLUP); //UPLOAD / GPS Switch (shared serial port)

  Serial.begin(9600);                   // initialize serial
  while (!Serial);
  
  Serial.print("Intiating LoRa Duplex");
  if (!LoRa.begin(915E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  Serial.println(", LoRa init succeeded.");
  Serial.println("Intiating GPS");
  setupGPS();
  Serial.println("GPS init compelete.");
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("oceanhippie.net");
  lcd.setCursor(0,1);
  lcd.print("loRa   Sniffer");
  delay(1000);
  lcd.clear();
  
            Reset = "<Reset:";
          Reset += BoatLetters[0];
          Reset += BoatLetters[1];
          Reset += ":";
          Reset += "000000";
          Reset += ">"; //Message End
          
          HootMessage = "<Hoot:";
          HootMessage += BoatLetters[0];
          HootMessage += BoatLetters[1];
          HootMessage += ":";
          HootMessage += tm;
          HootMessage += ">"; //Message End
  
  intro(); // setup sceen and run the animation
}
//end setup
void loop() {
 if (stat==1) { //we're running do stuff
  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= period)  //test whether the period has elapsed
  {
    if(mode==2 && tm==0) tm=-300; //deal with repeats 
    if(mode==4 && tm==0) tm=-180; //deal with repeats
  lcd.setCursor(0,0);
  bitTime();
  lcd.print(tmTxt);
  updDisp(); //update diplay
   ++cnt;
   if(cnt >= 10) { cnt=0; btm(); sendMessage(TimeMessage); } 
   //do hoots
   if(tm==-300 || tm == -60 || tm == -240 || (tm==-120 && mode==3) || (tm==-120 && mode==3)) {
    sendMessage(HootMessage);
      Serial.println("TOOOOOOOT!");
      digitalWrite(40, HIGH);
   } else {
    digitalWrite(40, LOW);
   }
     if(mode!=5) ++tm;
   startMillis = currentMillis;  //IMPORTANT to save the last time
  }
}
 
        // try to parse packet
        //int packetSize = LoRa.parsePacket();
        onReceive(LoRa.parsePacket());
        /*if (packetSize) {
          // received a packet
          Serial.print("Received packet '");

          // read packet
          while (LoRa.available()) {
            Serial.print((char)LoRa.read());  
          }
          // print RSSI of packet
          Serial.print("' with RSSI ");
          Serial.println(LoRa.packetRssi());
          Serial.print((char)LoRa.read());

         // parse for a packet, and call onReceive with the result:
         Serial.print("Sending.. ");
        onReceive(LoRa.parsePacket());
        Serial.println(" ..sent");
       
        } */
}
//send messages
void sendMessage(String outgoing) {
  Serial.print("sending" );
  Serial.println(outgoing);
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

  if (packetSize == 0)  return;          // if there's no packet, return
  
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

 // if (incomingLength != incoming.length()) {   // check length for error
 //   Serial.println("error: message length does not match length");
  //  return;                             // skip rest of function
 // }

  // if the recipient isn't this device or broadcast,
  //if (recipient != localAddress && recipient != 0xFF) {
  //  Serial.println("This message is not for me.");
  //  return;                             // skip rest of function
 // }

  // if message is for this device, or broadcast, print details:
  /*
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
  */
          lcd.setCursor(0,0);
          lcd.print("from: 0x" + String(sender, HEX));
          lcd.setCursor(8,0);
          lcd.print("to: 0x" + String(recipient, HEX));
          lcd.setCursor(-1,1);
          lcd.print(incoming);
  //strcpy(tempChars, incoming.c_str());
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

//============

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
    mode = messageData2Temp.toInt();
    if(mode==5) stat=1; //we're allways running in SternChaser
    Serial.print("Changed mode to ");
    Serial.println(mode);
          ModeMessage = "<Mode";
          ModeMessage += ":";
          ModeMessage += BoatLetters[0];
          ModeMessage += BoatLetters[1];
          ModeMessage += ":";
          ModeMessage += mode;
          ModeMessage += ">"; //Message End
          sendMessage(ModeMessage);  
    }
if(stat==0) { //we're not runng look for starts  
if(strcmp(messageHedder, "Start5m")==0 ) {// we got a start
    stat=1; //running
    tm=-300; //541go
      if(mode==3 || mode==4) tm=-180; //341Go
    Serial.println("start recived");
       btm (); sendMessage(TimeMessage);
       dmd.selectFont(imp36);//gp big font
       bitTime(); 
    }
    if(strcmp(messageHedder, "Start3m")==0 ) {// we got a start
    stat=1; //running
    tm=-180; //321go
    Serial.println("start recived");
       btm (); sendMessage(TimeMessage);
       dmd.selectFont(imp36);//gp big font
       bitTime(); 
    }
 }//end we're running
 if(strcmp(messageHedder, "Reset")==0) // Reply to changepode Message
  {
    Serial.println("Reset Recived");
    stat=0; //stop
    if(mode<=2) tm=-300; //541go RESET
    if(mode==3 || mode==4) tm=-180; //341Go RESET
    sendMessage(Reset);  //Pass it on

 dmd.drawBox(0, 0, 63, 31);
  //delay(1000);
  dmd.selectFont(Arial_Black_16);
  dmd.drawString(26, 9, "O");
  //delay(1000);
  dmd.selectFont(Droid_Sans_12);
  dmd.drawString(0, 10, "start");
  dmd.selectFont(Droid_Sans_12);
  dmd.drawString(37, 10, "matic");
  dmd.drawString(1, 1, String(mode));
  dmd.selectFont(imp36);//gp big font
    bitTime();
    }

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
void bitTime() {
  //makes time for dispalaying
  if(mode == 6) { //sternchaser
  if (fix.valid.time && fix.valid.date) {
    adjustTime( fix.dateTime );
    }
    } else {
  if(tm < 0) {sec = tm * -1;} else {sec=tm;}
  Serial.print(tm);
    hourUP=hourU; minTP=minT; secTP=secT; //store old values
    hourU = (sec /3600) % 24;
  minT =  ((sec /60) % 60)/10;
  minU = (sec /60) % 10;
  secT =  (sec %60) / 10;
  secU = sec % 10;
  
  //change fonts only when necessary
  if(minT!=minTP || hourU!=hourUP) { //dips change requred.
    if(hourU>0) {
    disp=3;
    dmd.selectFont(Droid_Sans_24); //go MED font
    dmd.clearScreen(); 
    } else {
    if(minT>0) {//2 digit miunites
        disp=2;
    dmd.selectFont(Droid_Sans_24); //go MED font
    dmd.clearScreen(); 
    } else { //9 mins or less big 
      disp=1;
      dmd.selectFont(imp36);//gp big font
      dmd.clearScreen(); 
    }
    }
  } //end disp change.
}//end is SternChaser ornormal


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
void updDisp() {
  switch (disp) {
  case 1:
    //big font change  only what we need
    if(minUP!=minU) dmd.drawString( 0,  -1, "  "); 
    dmd.drawString(0, -1, String(minU));
    if ( flip ) {dmd.drawString( 20,  -2, ":"); flip = 0; } else { dmd.drawString( 20,  -2, " ");  flip = 1; } //toggle ;
    if(secTP!=secT) dmd.drawString( 28,  -1, "  "); dmd.drawString( 28,  -1, String(secT)); 
    dmd.drawString( 46,  -1, "  ");  dmd.drawString( 46,  -1, String(secU));
    break;
  case 2:
    //Med Font
    if(minTP!=minT) dmd.drawString( 0,  5, "  "); dmd.drawString(2, 5, String(minT));
    if(minUP!=minU) dmd.drawString( 15,  5, "  "); dmd.drawString(15, 5, String(minU));
    if ( flip ) {dmd.drawString( 30,  5, ":"); flip = 0; } else { dmd.drawString( 30,  5, " ");  flip = 1; } //toggle ;
        if(secTP!=secT) dmd.drawString( 38,  5, "  "); dmd.drawString( 38,  5, String(secT));
         dmd.drawString( 52,  5, "  ");  dmd.drawString( 52,  5, String(secU)); 
 
    break;
      case 3:
    //Med Font
    if(hourUP!=hourU) dmd.drawString( -1,  5, "  "); dmd.drawString(-1, 5, String(hourU));
    dmd.drawString( 8,  5, ":");
    if(minTP!=minT) dmd.drawString( 12,  5, "  "); dmd.drawString(12, 5, String(minT));
    if(minUP!=minU) dmd.drawString( 24,  5, "  "); dmd.drawString(24, 5, String(minU));
    if ( flip ) {dmd.drawString( 36,  5, ":"); flip = 0; } else { dmd.drawString( 36,  5, " ");  flip = 1; } //toggle ;
        if(secTP!=secT) dmd.drawString( 40,  5, "  "); dmd.drawString( 40,  5, String(secT));
         dmd.drawString( 52,  5, "  ");  dmd.drawString( 52,  5, String(secU)); 
 
    break;
  default:
    // if nothing else matches, do the default
    // default is optional
    break;
}

}
void btm() {
          TimeMessage = "<Time";
          TimeMessage += ":";
          TimeMessage += BoatLetters[0];
          TimeMessage += BoatLetters[1];
          TimeMessage += ":";
          TimeMessage += tm;
          TimeMessage += ">"; //Message End
           Serial.println(TimeMessage); 
}
void setupGPS(){

    //gps checks - runs on setup stuffed down here for clairity
  Serial.print( F("NMEA.INO: started\n") );
  Serial.print( F("  fix object size = ") );
  Serial.println( sizeof(gps.fix()) );
  Serial.print( F("  gps object size = ") );
  Serial.println( sizeof(gps) );
  Serial.println( F("Looking for GPS device on " GPS_PORT_NAME) );

#ifndef NMEAGPS_RECOGNIZE_ALL
#error You must define NMEAGPS_RECOGNIZE_ALL in NMEAGPS_cfg.h!
#endif

#ifdef NMEAGPS_INTERRUPT_PROCESSING
#error You must *NOT* define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
#endif

#if !defined( NMEAGPS_PARSE_GGA ) & !defined( NMEAGPS_PARSE_GLL ) & \
      !defined( NMEAGPS_PARSE_GSA ) & !defined( NMEAGPS_PARSE_GSV ) & \
      !defined( NMEAGPS_PARSE_RMC ) & !defined( NMEAGPS_PARSE_VTG ) & \
      !defined( NMEAGPS_PARSE_ZDA ) & !defined( NMEAGPS_PARSE_GST )

  Serial.println( F("\nWARNING: No NMEA sentences are enabled: no fix data will be displayed.") );

#else
  if (gps.merging == NMEAGPS::NO_MERGING) {
    Serial.print  ( F("\nWARNING: displaying data from ") );
    Serial.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
    Serial.print  ( F(" sentences ONLY, and only if ") );
    Serial.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
    Serial.println( F(" is enabled.\n"
                          "  Other sentences may be parsed, but their data will not be displayed.") );
  }
#endif

  Serial.print  ( F("\nGPS quiet time is assumed to begin after a ") );
  Serial.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
  Serial.println( F(" sentence is received.\n"
                        "  You should confirm this with NMEAorder.ino\n") );

  trace_header( DEBUG_PORT );
  Serial.flush();

  gpsPort.begin( 9600 );
}

void adjustTime( NeoGPS::time_t & dt ) { //timezonefix
  Serial.println("adj tm..");
  NeoGPS::clock_t seconds = dt; // convert date/time structure to seconds
  seconds += zone_offset;  //  First, offset from UTC to the local timezone
  tm = seconds; // convert to seconds for Tom Stucture.
  dt = seconds; // convert seconds back to a date/time structure
}
void intro() {
  Serial.print("intro, pausing for 2 seconds");
  delay(2000);
  Serial.print(", runing config");
  dmd.setBrightness(255);
  dmd.selectFont(SystemFont5x7);
  dmd.begin();
  dmd.clearScreen();
 Serial.print(", initialized");
    //ready display
  dmd.drawBox(0, 0, 63, 31);
  //delay(1000);
  dmd.selectFont(Arial_Black_16);
  dmd.drawString(26, 9, "O");
  //delay(1000);
  dmd.selectFont(Droid_Sans_12);
  dmd.drawString(0, 10, "start");
  dmd.selectFont(Droid_Sans_12);
  dmd.drawString(37, 10, "matic");
  //delay(6000);
  dmd.clearScreen();
  dmd.selectFont(imp36);
  dmd.drawString(3, 0, "TOM");
  //delay(4000);
  dmd.clearScreen();
  dmd.drawBox(0, 0, 63, 31);
  //delay(1000);
  dmd.selectFont(Droid_Sans_12);
  dmd.drawString(2, 2, "oceanhippie");
  //delay(1000);
  dmd.drawString(44, 19, ".net");
  delay(3000);
  //reset stuuf and choose font
  dmd.clearScreen();
   Serial.println(", intro complete.");
}