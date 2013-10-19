
#include <EasyTransfer.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Z_OSC.h>
#include <Wire.h>
#include <eepromi2c.h>
#include <RTClib.h>
#include <IRremote.h>

#define RESET 22
#define STROBE 23
#define IN 0
#define DS1307_ADDRESS 0x68;

#define BUTTONSPIN A15
#define SWITCHPIN 6
#define RELAYPIN 5

//Buttons and switch
byte buttonModes[12]={13,14,12,15,21,31,32,16,17,27,28,29};
byte lastButtonState = 0;
long lastDebounceButtonTime = 0;
byte debounceDelay=50;
byte buttonState = 0;
boolean previousSwitchState = false;
boolean switchState = true;
boolean relayState = true;

//IR Remote
int RECV_PIN = 11;

IRrecv irrecv(RECV_PIN);
IRsend irsend; //Pin 46 (modify to use timer 5 in IRremoteInt.h
decode_results results;


//Buffers
int spectrumValue[7];
//Strings
String s;
String str;
char buffer[100];
char charbuffer[32];
int i = 0;
byte j = 0;
//Mode-selection
byte readyToSend = 0;
byte previousMode = 0;
int vorigGemiddeld = 0;
int game = 0;
byte lastMode = 0;
//Timing
unsigned int timer1 = 0;
//Connections
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0, 51);
//IPAddress ip(10,0,1, 201);
byte myIpOSC[]  = { 192, 168,0, 51 };
//byte myIpOSC[]  = { 10, 0, 1, 201 };
int  serverPortOSC  = 8000;
EthernetServer ethernetServer(80);
Z_OSCServer OSCServer;
Z_OSCMessage *rcvMes;
String ethernetbuffer=String(50);

EasyTransfer trans; 

struct RECEIVE_DATA_STRUCTURE{
  byte mode;
  byte strLength;
  char str[10];
  byte value0;
  byte value1;
  byte value2;
  byte value3;
  byte value4;
  byte value5;
  byte value6;
  unsigned int value7;
  unsigned int value8;
};

RECEIVE_DATA_STRUCTURE data;


//Opslag
byte color[4];
unsigned int strobe[2];


//Klok en EEPROM
RTC_DS1307 Clock;
struct voorkeuren{
  int aantalWekkers;
} voorkeuren;

struct wekker{
  byte dagen[7];
  byte uur;
  byte minuut;
  char *label;
  byte actief;
} wekker;

boolean wekkerGingAf=false;
int wekkerGingAfOp;

void setup(){

    Serial.begin(9600); //Serial begin
    Serial.println("Master setup");
  
    Serial1.begin(9600); //Communication between master and table begin
    trans.begin(details(data), &Serial1);
 
	pinMode(IN, INPUT); //Music
	pinMode(STROBE, OUTPUT);
	pinMode(RESET, OUTPUT);
	
	analogReference(DEFAULT);

	digitalWrite(RESET, LOW);
	digitalWrite(STROBE, HIGH);

	digitalWrite(RESET, HIGH);
	digitalWrite(RESET, LOW);

	pinMode(SWITCHPIN, INPUT); //Buttons
	pinMode(BUTTONSPIN, INPUT);
	pinMode(RELAYPIN, OUTPUT);
	digitalWrite(SWITCHPIN, LOW);
	  
	if(relayState) //Enable table according to relayState
	  digitalWrite(RELAYPIN, HIGH);
	else
	  digitalWrite(RELAYPIN, LOW);
	  	
	  	
	irrecv.enableIRIn(); //IR Remote
	
	Ethernet.begin(mac, ip); //Ethernet
	ethernetServer.begin();
	OSCServer.sockOpen(serverPortOSC);
	Serial.print("server is at ");
	Serial.println(Ethernet.localIP());
	  
	Wire.begin(); //Clock
	Clock.begin();
	if(!Clock.isrunning())
    	Serial.println("RTC is not running.");
	  
	DateTime now=Clock.now(); //Get clock and print clock
	String time="";
	time+=now.day();
	time+="/";
	time+=now.month();
	time+="/";
	time+=now.year();
	time+=" ";
	time+=now.hour();
	time+=":";
	time+=now.minute();
	Serial.println(time);
	  
	  
	wekker.dagen[1]=1;
	wekker.dagen[2]=1;
	wekker.dagen[3]=1;
	wekker.dagen[4]=1;
	wekker.dagen[5]=1;
    wekker.dagen[6]=1;
    wekker.dagen[7]=1;
    wekker.uur=now.hour();
    wekker.minuut=now.minute()+1;
    wekker.actief=1;
    
    voorkeuren.aantalWekkers=1;
  
  	Serial.println("Proberen opslaan:");
    Serial.print("uur ");
    Serial.println(wekker.uur);
    Serial.print("minuut ");
    Serial.println(wekker.minuut);
    
    //eeWrite(0,voorkeuren);
    eeWrite(1,wekker);
  
    delay(30);
   
    wekker.uur=0;
    wekker.minuut=0;
  	
  	eeRead(1,wekker);
  	
    Serial.println("De opgeslagen wekker:");
    Serial.print("uur ");
    Serial.println(wekker.uur);
    Serial.print("minuut ");
    Serial.println(wekker.minuut);
  
	color[0]=0; //Set current color on black, with full alpha
	color[1]=0;
	color[2]=0;
	color[3]=100;
}



void loop(){
  //Resets---------------------------------------
  readyToSend=0;
  previousMode=data.mode; //Current data mode is previous data mode, but will not be sent out except for continuous modes
  dataReset(); //Reset all data
  s=""; //Serial string reset
  
  
  //Alarm Clock (wakeUp)
  eeRead(0,voorkeuren);
  /* Serial.print("aantal ");
  Serial.println(voorkeuren.aantalWekkers);*/
  
  for(int nummerWekker=1; nummerWekker<=voorkeuren.aantalWekkers; nummerWekker++){ 
    eeRead(nummerWekker,wekker);
    DateTime now= Clock.now();
    
    byte minuut=now.minute();
    byte uur=now.hour();
    byte dag=now.dayOfWeek();
    
    wekker.dagen[0]=wekker.dagen[7];
    
    if(minuut>wekkerGingAfOp) wekkerGingAf=false;
    
    if(wekker.dagen[dag]>0 && wekker.uur==uur && wekker.minuut==minuut && !wekkerGingAf && wekker.actief>0){
        Serial.println("Radio speelt");
        wekkerGingAf = true;
        wekkerGingAfOp = wekker.minuut;
    }
    delay(50);
  }
  
  //Buttons & switch Control--------------------
  if(digitalRead(SWITCHPIN))
  	switchState=true;
  else 
  	switchState=false;

  if(switchState != previousSwitchState){
  	if(relayState){
  		digitalWrite(RELAYPIN, LOW);
  		relayState = false;
  	}else{
  		digitalWrite(RELAYPIN, HIGH);
  		relayState = true;
  	}
  	
  }
  previousSwitchState = switchState;
  
  	
  int buttonValue = analogRead(BUTTONSPIN); //Analog to 'digital' convert
  byte buttonChanged = 0;
  
  if(buttonValue>400 && buttonValue<700){
  	buttonChanged=2;
  }else if(buttonValue>700 && buttonValue<1000){
  	 buttonChanged=1;
  }
  
  if(buttonChanged != lastButtonState) //If button changed, reset timer
  	lastDebounceButtonTime = millis();
  
  if((millis() - lastDebounceButtonTime) > debounceDelay){ //Debounce
  
  	if(buttonChanged != buttonState){
  		Serial.println("button changed");
  		buttonState = buttonChanged;
		boolean found=false;
		byte m=0;
		while(!found & m<(sizeof(buttonModes)/sizeof(byte))){ //Search current mode in sequence array
			if(lastMode==buttonModes[m])
				found=true;
			m++;
		}
		m--;
		
		if(found){
			if(buttonChanged==1){ //Button up
				m++; 
				if(m>=(sizeof(buttonModes)/sizeof(byte))){
					m=0; //Cycle back to beginning
				}
			} else if(buttonChanged==2){ //Button down
				if(m>0){
					m--; 
				} else{
					m=(sizeof(buttonModes)/sizeof(byte))-1; //Cycle to end
				}
			}
		} else{
			m=0; //Mode not found, start at beginning
		}
		
		data.mode=buttonModes[m]; //Set mode
		readyToSend=1;
	}
	
  }
  lastButtonState=buttonChanged;
  
  //IR Remote
  if (irrecv.decode(&results) & readyToSend<1) {
  
  	if(results.value == 0x10C8E11E){ //ON OFF 
  		switchChange();
  	}
  	
  	else if(results.value == 0x10C8718E){ //Rainbow
  		data.mode = 13;
  		readyToSend=1;
  	}
  	
  	else if(results.value == 0x10C8F10E){ //RainbowScroll
  		data.mode = 14;
  		readyToSend = 1;
  	}
  	
  	else if(results.value == 0x10C806F9){ //Random
  		data.mode = 12;
  		readyToSend = 1;
  	}
  	
  	else if(results.value == 0x10C8B14E){ //Stars
  		data.mode = 15;
  		readyToSend = 1;
  	}
  	
  	else if(results.value == 0x10C831CE){ //Fireplace
  		data.mode = 21;
  		readyToSend = 1;
  	}
  	
  	else if(results.value == 0x10C832CD){ //Firework
  		data.mode = 31;
  		readyToSend = 1;
  	}
  	
  	else if(results.value == 0x10C841BE){ //Game of life
  		data.mode = 32;
  		readyToSend = 1;
  	}
  	
  	else if(results.value == 0x10C8B24D){ //Empty
  		
  	}
  	
  	else if(results.value == 0x10C8728D){ //EQ
  		data.mode = 16;
  		readyToSend = 1;
  	}
  	
  	else if(results.value == 0x10C821DE){ //Beats
  		data.mode = 17;
  		readyToSend = 1;
  	}
  	
  	else if(results.value == 0x10C8F20D){ //Snake
  		data.mode = 18;
  		game = 18;
  		readyToSend = 1;
  	}
  	
  	else if(results.value == 0x10C87A85){ //Pong
  		data.mode = 23;
  		game = 23;
  		readyToSend = 1;
  	}
  	
  	else if(results.value == 0x10C8D12E){ //Tetris
  		data.mode = 25;
  		game=25;
  		readyToSend = 1;
  	}
  	
  	else if(results.value == 0x10C8A15E){ //up
  		if(game==18) data.mode=19;
        else if(game==23) data.mode=24;
        else if(game==25) data.mode=26;
        data.value0=1;
  		readyToSend = 1;
  	}
  	
  	else if(results.value == 0x10C8C639){ //right
  		if(game==18) data.mode=19;
        else if(game==23) data.mode=24;
        else if(game==25) data.mode=26;
        data.value0=2;
  		readyToSend = 1;
  	}
  	
  	else if(results.value == 0x10C826D9){ //down
  		if(game==18) data.mode=19;
        else if(game==23) data.mode=24;
        else if(game==25) data.mode=26;
        data.value0=3;
  		readyToSend = 1;
  	}
  	
  	else if(results.value == 0x10C801FE){ //left
  		if(game==18) data.mode=19;
        else if(game==23) data.mode=24;
        else if(game==25) data.mode=26;
        data.value0=4;
  		readyToSend = 1;
  	}
  	
    Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
  }
  
  //OSC Control----------------------------------
  String getOSC="";
  if(OSCServer.available() && readyToSend<1){ //Check if OSCServer is available and if there is no other connection 
    rcvMes=OSCServer.getMessage();
    //byte destIPOSC[]={*rcvMes->getIpAddress(), *(rcvMes->getIpAddress()+1), *(rcvMes->getIpAddress()+2), *(rcvMes->getIpAddress()+3)};
    //message.setAddress(destIPOSC, destPortOSC);
    
    
    //ON OFF
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/onoff")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
      	switchChange();
      }
    }
    
    //COLORS
    boolean colorChanged=false;
    //Red
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/red")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,255);
      color[0]=number;
      colorChanged=true;
    }
    
    //Green
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/green")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,255);
      color[1]=number;
      colorChanged=true;
    }
    
    //Blue
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/blue")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,255);
      color[2]=number;
      colorChanged=true;
    }
    
    //opacity
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/opacity")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,100);
      color[3]=number;
      colorChanged=true;
    }
    
    if(colorChanged){ //Send received colors all at once
      data.mode=5;
      data.value0=color[0]*color[3]/100;
      data.value1=color[1]*color[3]/100;
      data.value2=color[2]*color[3]/100;
      readyToSend=1;
    }
    
    //Clear
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/clear")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=2;
        readyToSend=1;
        /*message.setZ_OSCMessage( "/1/cleared" ,"s" , "cleared" );
        OSCClient.send(&message);*/
      }
    }
    
    //setAll
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/setAll")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=7;
        readyToSend=1;
      }
    }
    
    //push
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/push")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=7;
        readyToSend=1;
      }else{
        data.mode=2;
        readyToSend=1;
      }
    }

    //Strobe
    boolean strobeChanged=false;
    
    //1
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/strobe/1")){
      unsigned int number=rcvMes->getFloat(0);
      strobe[0]=number;
      strobeChanged=true;
    }
    
    //2
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/strobe/2")){
      unsigned int number=rcvMes->getFloat(0);
      strobe[1]=number;
      strobeChanged=true;
    }
    
    if(strobeChanged){ //Send received strobe data all at once
      data.mode=22;
      data.value7=strobe[0];
      data.value8=strobe[1];
      readyToSend=1;
    }
    
    //rainbow
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/rainbow")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=13;
        readyToSend=1;
      }
    }
    
    //rainbowScroll
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/rainbowScroll")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=14;
        readyToSend=1;
      }
    }
    
    //random
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/random")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=12;
        readyToSend=1;
      }
    }
    
    //stars
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/stars")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=15;
        readyToSend=1;
      }
    }
    
    //fireplace
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/fireplace")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=21;
        readyToSend=1;
      }
    }
    
    //eq
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/eq")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=16;
        readyToSend=1;
      }
    }
    
    //beats
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/beats")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=17;
        readyToSend=1;
      }
    }
    
    //Snake initialize
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/snake")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=18;
        game=18;
        readyToSend=1;
      }
    }
    
    //Game Move UP
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/up")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        if(game==18) data.mode=19;
        else if(game==23) data.mode=24;
        else if(game==25) data.mode=26;
        data.value0=1;
        readyToSend=1;
      }
    }
    
    //Game Move RIGHT
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/right")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        if(game==18) data.mode=19;
        else if(game==23) data.mode=24;
        else if(game==25) data.mode=26;
        data.value0=2;
        readyToSend=1;
      }
    }
    
    //Game Move DOWN
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/down")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        if(game==18) data.mode=19;
        else if(game==23) data.mode=24;
        else if(game==25) data.mode=26;
        data.value0=3;
        readyToSend=1;
      }
    }
    
    //Game Move LEFT
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/left")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        if(game==18) data.mode=19;
        else if(game==23) data.mode=24;
        else if(game==25) data.mode=26;
        data.value0=4;
        readyToSend=1;
      }
    }

    //Pong 1 initialize
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/pong")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=23;
        game=23;
        readyToSend=1;
      }
    }
    
    //Tetris initialize
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/tetris")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=25;
        game=25;
        readyToSend=1;
      }
    }
    
    //digitalClock
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/digitalClock")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=27;
        readyToSend=1;
      }
    }
    
    //binaryClock
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/binaryClock")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=28;
        readyToSend=1;
      }
    }
    
    //Temperature
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/temperature")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=29;
        readyToSend=1;
      }
    }
    
    //Fireworks
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/fireworks")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=31;
        readyToSend=1;
      }
    }
    
    //Game of Life
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/gameOfLife")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=32;
        readyToSend=1;
      }
    }
    
    //Particle System - Flame
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/particleFlame")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=33;
        readyToSend=1;
      }
    }
    
    //Particle System - Smoker
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/particleSmoker")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=34;
        readyToSend=1;
      }
    }
    
    //Particle System - Spin
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/particleSpin")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=35;
        readyToSend=1;
      }
    }
    
    //Radial Rainbow
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/rainbowRadial")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=36;
        readyToSend=1;
      }
    }
    
    //WakeUp TEST
    if (!strcmp( rcvMes->getZ_OSCAddress(),"/1/wakeUpTest")){
      int number=rcvMes->getFloat(0);
      number=constrain(number,0,1);
      if(number>0){
        data.mode=37;
        readyToSend=1;
      }
    }
    
  }
  
  
  
  //Web control-------------------------------------
  
  String getEthernet="";
  EthernetClient client = ethernetServer.available();
  
  if (client && readyToSend<1) { // Check if client is available and if there is no other connection 
  
    boolean currentLineIsBlank = true;
    ethernetbuffer="";
    
    while (client.connected()) {
      if (client.available()) {
      
        char c = client.read();
        
        if (ethernetbuffer.length() < 50) {
            ethernetbuffer.concat(c);
         }
         
        if (c == '\n' && currentLineIsBlank) {
        
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connnection: close");
          client.println();
          
          //Serial.println(ethernetbuffer);
          
          if(ethernetbuffer.indexOf("mode=clear")>0)
            data.mode=2;
          else if(ethernetbuffer.indexOf("mode=fadeOut")>0)
            data.mode=4;
          else if(ethernetbuffer.indexOf("mode=setAll")>0)
            data.mode=7;
          else if(ethernetbuffer.indexOf("mode=rainbowScroll")>0)
            data.mode=14;
          else if(ethernetbuffer.indexOf("mode=rainbow")>0)
            data.mode=13;
          else if(ethernetbuffer.indexOf("mode=stars")>0)
            data.mode=15;
          else if(ethernetbuffer.indexOf("mode=random")>0)
            data.mode=12;
          else if(ethernetbuffer.indexOf("mode=eq")>0)
            data.mode=16;
          else if(ethernetbuffer.indexOf("mode=beats")>0)
            data.mode=17;
          else if(ethernetbuffer.indexOf("mode=fireplace")>0)
            data.mode=21;
          else if(ethernetbuffer.indexOf("mode=setColor")>0){
            data.mode=5;
            getEthernet="";
            byte p=ethernetbuffer.indexOf("mode=setColor")+13;
            while(ethernetbuffer.charAt(p)!=')'){
              getEthernet+=ethernetbuffer.charAt(p);
              p++;
            }
            getEthernet+=")";
            byte res[10];
            splitValues(getEthernet,res);
            data.value0=res[0];
            data.value1=res[1];
            data.value2=res[2];
          }
          else if(ethernetbuffer.indexOf("mode=setPixel")>0){
            data.mode=6;
            byte p=ethernetbuffer.indexOf("mode=setPixel")+13;
            while(ethernetbuffer.charAt(p)!=')'){
              getEthernet+=ethernetbuffer.charAt(p);
              p++;
            }
            getEthernet+=')';
            
            String c=s.substring(8);
            byte res[10];
            splitValues(c,res);
            data.value0=res[0];
            data.value1=res[1];
            readyToSend=1;
          }
          else if(ethernetbuffer.indexOf("mode=write(")>0){
            data.mode=11;
            byte p=ethernetbuffer.indexOf("mode=write(")+12;
            while(ethernetbuffer.charAt(p)!=')'){
              getEthernet+=ethernetbuffer.charAt(p);
              p++;
            }
            str=getEthernet;
          }
          /*--REMOTE COMMANDS FOR STEREO--*/
		  boolean remote = false;
		  if(ethernetbuffer.indexOf("cmd=power")>0){
		  	irsend.sendNEC(0xA55A38C7, 32);
		  	remote = true;
		  	Serial.println("power");
		  }
		  if(ethernetbuffer.indexOf("cmd=tuner")>0){
		  	irsend.sendNEC(0xA55AE21D, 32);
		  	remote = true;
		  }
		  if(ethernetbuffer.indexOf("cmd=phone")>0){
		  	irsend.sendNEC(0xA55AB14D, 32);
		  	remote = true;
		  }
		  if(ethernetbuffer.indexOf("cmd=volumed")>0){
		  	irsend.sendNEC(0xA55AD02F, 32);
		  	remote = true;
		  }
		  if(ethernetbuffer.indexOf("cmd=volumep")>0){
		  	irsend.sendNEC(0xA55A50AF, 32);
		  	remote = true;
		  }
		  if(ethernetbuffer.indexOf("cmd=cd")>0){
		  	irsend.sendNEC(0xA55A32CD, 32);
		  	remote = true;
		  }
		  if(ethernetbuffer.indexOf("cmd=vcr")>0){
		  	irsend.sendNEC(0xA55AF00F, 32);
		  	remote = true;
		  }
		  if(ethernetbuffer.indexOf("cmd=tape1")>0){
		  	irsend.sendNEC(0xA55A728D, 32);
		  	remote = true;
		  }
		  if(ethernetbuffer.indexOf("cmd=tape2")>0){
		  	irsend.sendNEC(0xA55AB847, 32);
		  	remote = true;
		  }
		  if(ethernetbuffer.indexOf("cmd=sleep")>0){
		  	irsend.sendNEC(0xA55A12ED, 32);
		  	remote = true;
		  }
		  if(remote)
		  	irrecv.enableIRIn();
          if(!remote) readyToSend=1;
          
          client.println("ok");
          client.println(getEthernet);
          client.println(s);
          break;
        }
        if (c == '\n') 
          currentLineIsBlank = true;
        else if (c != '\r') 
          currentLineIsBlank = false;
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }

  
  
  //Serial control-----------------------------------
  s=checkSerial();
  delay(10);
  
  if(readyToSend<1){ //Check if no other connection was made
    if(s.equals("reset")){
      data.mode=1;
      readyToSend=1;
    }
    
    else if(s.equals("clear")){
      data.mode=2;
      readyToSend=1;
    }
    
    else if(s.equals("fadeOut")){
      data.mode=4;
      readyToSend=1;
    }
    
    else if(s.substring(0,8).equals("setColor")){
      String c=s.substring(8);
      byte res[10];
      splitValues(c,res);
      data.mode=5;
      data.value0=res[0];
      data.value1=res[1];
      data.value2=res[2];
      readyToSend=1;
    }
    
    else if(s.substring(0,8)=="setPixel"){
      String c=s.substring(8);
      byte res[10];
      splitValues(c,res);
      data.mode=6;
      data.value0=res[0];
      data.value1=res[1];
      readyToSend=1;
    }
    
    else if(s.equals("setAll")){
      data.mode=7;
      readyToSend=1;
    }
    
    else if(s.substring(0,10)=="drawCircle"){
      String c=s.substring(10);
      byte res[10];
      splitValues(c,res);
      data.mode=8;
      data.value0=res[0];
      data.value1=res[1];
      data.value2=res[2];
      readyToSend=1;
    }
    
    else if(s.substring(0,13)=="drawRectangle"){
      String c=s.substring(13);
      byte res[10];
      splitValues(c,res);
      data.mode=9;
      data.value0=res[0];
      data.value1=res[1];
      data.value2=res[2];
      data.value3=res[3];
      readyToSend=1;
    }
    
    else if(s.substring(0,5).equals("write")){
      data.mode=11;
      str= s.substring(6);
      str= str.substring(0, str.length() - 1);
      readyToSend=1;
    }
    
    else if(s=="random"){
      data.mode=12;
      readyToSend=1;
    }
    
    else if(s=="rainbow"){
      data.mode=13;
      readyToSend=1;
    }
    
    else if(s=="rainbowScroll"){
      data.mode=14;
      readyToSend=1;
    }
    
    else if(s=="stars"){
      data.mode=15;
      readyToSend=1;
    }
  
    else if(s=="eq"){
      data.mode=16;
    }
    
    else if(s=="beats"){
      data.mode=17;
    }
    
    else if(s.substring(0,9)=="snakeMove"){
      String c=s.substring(10);
      data.mode=19;
      str= c.substring(0, c.length() - 1);
      readyToSend=1;
    }
    else if(s.substring(0,8)=="snakeEat"){
      data.mode=20;
      readyToSend=1;
    }
    else if(s.substring(0,5)=="snake"){
      data.mode=18;
      readyToSend=1;
    }
    
    else if(s.substring(0,9)=="fireplace"){
      data.mode=21;
      readyToSend=1;
    }
    
    else if(s.substring(0,6)=="strobe"){
      data.mode=22;
      String c=s.substring(6);
      unsigned int resInts[10];
      splitValuesInInts(c,resInts);
      data.value7=resInts[0]; //in mHZ
      data.value8=resInts[1]; //in mHZ
      Serial.println(data.value7);
      Serial.println(data.value8);
      readyToSend=1;
    }
  }
  
  
  //Continuous effects (music, clock,...): keep sending
  if(data.mode==16 || (previousMode == 16 && data.mode == 0)){
  	//EQ
    if(data.mode==0) data.mode=16;
    
    readMusic(spectrumValue); //Read music
    
    for(byte c=0; c<7; c++){
      byte b=constrain(map(spectrumValue[c],60,1000,0,9),0,8);
      
      switch(c){
        case 0: data.value0=b;
        case 1: data.value1=b;
        case 2: data.value2=b;
        case 3: data.value3=b;
        case 4: data.value4=b;
        case 5: data.value5=b;
        case 6: data.value6=b;
      }
    }
    
    readyToSend=1; //Keep sending
    
  }else if(data.mode==17 || (previousMode==17 && data.mode==0)){ //If the mode in this loop is 17, or in the previous loop and if no mode has been selected in this loop.
    //BEATS
    if(data.mode==0) data.mode=17;
    
    readMusic(spectrumValue); //Read music
    
    int som;
    som+=spectrumValue[0]*2;
    som+=spectrumValue[1];
    
    int g=som/3;
    int gemiddeld=0;
    if(g>0){
      gemiddeld=constrain(map(g-300,0,1000,0,255),0,255);
    }
    data.value0=gemiddeld;
    vorigGemiddeld=gemiddeld;

    readyToSend=1; //Keep sending
    
    delay(20);
  }else if(data.mode==27 || (previousMode == 27 && data.mode == 0 )){ //Digital clock
	DateTime now=Clock.now(); //Read clock
	
	if(data.mode==0) data.mode=27; //Set mode
	
	data.value0=now.hour();
	data.value1=now.minute();
	
	readyToSend=1; //Keep sending
  }else if(data.mode==28 || (previousMode == 28 && data.mode == 0 )){ //Binary clock
	DateTime now=Clock.now(); // Read clock
	
	if(data.mode==0) data.mode=28; //Set mode
	
	data.value0=now.hour();
	data.value1=now.minute();
	data.value2=now.second();
	
	readyToSend=1; //Keep sending
  }
  
  if(readyToSend>0 && relayState){ //If there was a connection made turing the loop, send it to the table
    sendToTable();
  }
}


//Functions----------------------------

String checkSerial(){ //Check the serial and copy it in a String
  for(byte j=0; j<100; j++){
    buffer[j]=0;
  }
  i=0;
  j=1;
  while(Serial.available()){
    buffer[i]=Serial.read();
    i++;
    s=String(buffer);
    delay(10);
    j=0;
  }
  return s;
}

void sendToTable(){ //Function to send to table, using easyTransfer
    data.strLength=str.length();
    for(byte p=0; p<10; p++)
      data.str[p]='a';
    for(byte p=0; p<str.length(); p++)
      data.str[p]=str.charAt(p);
    
    trans.sendData();
    Serial.println(data.mode);
    lastMode=data.mode;
}

void readMusic(int spec[7]){ //Read the music of the MSGEQ7 in an array
    digitalWrite(RESET, HIGH);
    digitalWrite(RESET, LOW);
    for(byte c=0; c<7; c++){
      digitalWrite(STROBE, LOW);
      delayMicroseconds(30);

      spec[c] =  analogRead(IN);

      digitalWrite(STROBE, HIGH);
    }

}


void splitValues(String c, byte res[10]){ //Split comma seperated values in an array of bytes
    for(int k=0; k<0; k++){
      res[k]=0;
    }
    char b[50];
    int j=0;
    byte k=0;
    c.replace('(',' ');
    c.replace(')',' ');
    while(c.indexOf(',')!=-1){
      k=c.indexOf(",");
      c.substring(0,k).toCharArray(b,k+1);
      res[j]=(byte)atoi(b);
      c=c.substring(k+1);
      j++;
    }
    c.toCharArray(b,c.length());
    res[j]=(byte)atoi(b);
}

void splitValuesInInts(String c, unsigned int res[10]){ //Split comma seperated values in an array of unsigned ints
    for(int k=0; k<0; k++){
      res[k]=0;
    }
    char b[50];
    int j=0;
    int k=0;
    c.replace('(',' ');
    c.replace(')',' ');
    while(c.indexOf(',')!=-1){
      k=c.indexOf(",");
      c.substring(0,k).toCharArray(b,k+1);
      
      res[j]=(unsigned int)atoi(b);
    
      c=c.substring(k+1);
      j++;
    }
    c.toCharArray(b,c.length());
    res[j]=(unsigned int)atoi(b);
}

void dataReset(){ //Set all data back to 0
  for(byte p=0; p<10; p++)
      data.str[p]='a';
  data.strLength=0;
  str="";
  data.mode=0;
  data.value0=0;
  data.value1=0;
  data.value2=0;
  data.value3=0;
  data.value4=0;
  data.value5=0;
  data.value6=0;
  data.value7=0;
  data.value8=0;
}

void switchChange(){ //Switch state 
	if(relayState){
		relayState=false;
		digitalWrite(RELAYPIN, LOW);
	}else{
		relayState=true;
		digitalWrite(RELAYPIN, HIGH);
	}
}
