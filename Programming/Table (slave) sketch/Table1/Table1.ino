//Basic
#include <EasyTransfer.h>
#include <FastSPI_LED.h>
#include <avr/pgmspace.h>
#include <Display.h>

//Temperature
#include <OneWire.h>
#include <DallasTemperature.h>

//Particle System: https://github.com/giladaya/arduino-particle-sys
#include <ParticleSys.h>
#include <Particle_Std.h>
#include <Particle_Bounce.h>
#include <Particle_Fixed.h>
#include <Emitter_Fountain.h>
#include <Particle_Attractor.h>
#include <PartMatrix.h>
#include <Emitter_Spin.h>

//IR for touch support
#include <IRremote.h>


//IR for touch support
#define PIN_IR 9 //9 for Mega, 3 for Uno
IRsend irsend;

//Temperature
#define ONE_WIRE_BUS 7
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


int r=8;
Display display(r);
char buffer[100];
int i=0;
byte j=0;
byte red=0;
byte green=0;
byte blue=0;
String s;
String str;
boolean firstTime=false;
byte mode;
boolean justReset;
byte previousMode;
byte prevValue1=0;
unsigned long timer1=0;

//create object
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

//give a name to the group of data
RECEIVE_DATA_STRUCTURE data;

//Particle system
const byte numParticles = 60;
boolean pulseOn = false;


Particle_Std particlesStd[numParticles];
Particle_Bounce particlesBounce[numParticles];

Particle_Fixed sourceFixed;
Emitter_Fountain emitterFixed(0, 0, 5, &sourceFixed);
ParticleSys pSysFixed(numParticles, particlesStd, &emitterFixed);

Particle_Attractor sourceAttractor;
Emitter_Fountain emitterAttractor(0, 0, 5, &sourceAttractor);
ParticleSys pSysAttractor(numParticles, particlesStd, &emitterAttractor);

Emitter_Spin emitterSpin(112, 112, 5, 7);
ParticleSys pSysSpin(numParticles, particlesBounce, &emitterSpin);

PartMatrix pMatrix;

int lastFog; 
int fogI=1;

void setup(){
    Serial.begin(9600);
    Serial1.begin(9600);
    trans.begin(details(data), &Serial1);
    
    sensors.begin();
    
    //Particle System 
    pMatrix.reset();
    
    display.enableTouch();
    
    Serial.println("Waiting on input");
    
    display.setRotation(1);
    
    
    //Enable Touch IR LED
    pinMode(9,OUTPUT);
  	digitalWrite(9, LOW);
    irsend.enableIROut(36);
    irsend.mark(0);
    
    pinMode(dataTouchPins[0],OUTPUT);
    pinMode(dataTouchPins[1],OUTPUT);
    pinMode(dataTouchPins[2],OUTPUT);
    justReset=true;
}

void loop(){
  if(justReset)
    data.mode=30;
  justReset=false;
  
  //Get serial
  if(Serial1.available()){
    if(trans.receiveData()){
      
      str="";
      for(byte p=0; p<data.strLength; p++)
        str+=data.str[p];
      firstTime=true;
    }
  }
  
  delay(10);
  /*--PREVIOUS--*/
  if(data.mode==0){
    data.mode=mode;
  }
  
  /*--RESET--*/
  else if(data.mode==1){
    display.reset();
   
  }
 
  /*--CLEAR--*/
  else if(data.mode==2){
    display.clear();
    
    display.show();
  }
  
  /*--FADEOUT--*/
  else if(data.mode==4){
    display.fadeOut(1000);
    
  }
  
  /*--SETCOLOR--*/
  else if(data.mode==5){
    red=data.value0;
    green=data.value1;
    blue=data.value2;
    data.mode=previousMode;
  }
  
  /*--SETPIXEL--*/
  else if(data.mode==6){
    display.setPixel(data.value0,data.value1,red, green, blue);
    display.show();
    
  }
  
  /*--SETALL--*/
  else if(data.mode==7){
    display.setAll(red, green, blue);
    display.show();
    delay(10); 
    
  }
  
  /*--DRAWRECTANGLE--*/
  else if(data.mode==8){
    
    display.drawCircle(data.value0,data.value1,data.value2,red, green, blue);
    display.show();
    
  }
  
  /*--DRAWRECTANGLE--*/
  else if(data.mode==9){
    
    display.drawRectangle(data.value0,data.value1,data.value2,data.value3,red, green, blue);
    display.show();
    
  }
  
  /*--WRITE--*/
  else if(data.mode==11){
    
    byte d;
    if(data.value0==0) d=200;
    else d=data.value0;
    
    display.scrollLeft(str,d, red, green, blue);
    delay(300);
    
  }
  
  /*--RANDOM--*/
  else if(data.mode==12){
    display.clear(false);
    
    byte d;
    if(data.value0==0) d=50;
    else d=data.value0;
    
    float fill;
    if(data.value1==0) fill=0.9;
    else fill=data.value1/100;
    
    display.fillRandom(fill);
    display.show();
    delay(d);
    
  }
  
  /*--RAINBOW--*/
  else if(data.mode==13){
    
    byte d;
    if(data.value0==0) d=5;
    else d=data.value0;

    display.rainbow(d);
  }
  
  /*--RAINBOWSCROLL--*/
  else if(data.mode==14){
    display.setFirstTime(firstTime);
    
    byte d;
    if(data.value0==0) d=10;
    else d=data.value0;
    
    byte step1;
    if(data.value1==0) step1=4;
    else step1=data.value1;
    
    byte step2;
    if(data.value2==0) step2=2;
    else step2=data.value2;
    
    display.rainbowScroll(d, step1, step2);
  }
  
  /*--STARS--*/
  else if(data.mode==15){
    if(firstTime) display.clear();
    
    byte d;
    if(data.value0==0) d=10;
    else d=data.value0;
    
    display.stars(d);
       
  }
  
  /*--EQ--*/
  else if(data.mode==16){
    if(red==0&&green==0&&blue==0)
      display.eq(data.value0,data.value1,data.value2,data.value3,data.value4,data.value5,data.value6);
    else
      display.eq(data.value0,data.value1,data.value2,data.value3,data.value4,data.value5,data.value6,red,green,blue);
  
  }
  
  /*--BEATS--*/
  else if(data.mode==17){
    if(red==0&&green==0&&blue==0)
      display.beats(data.value0,(byte)255,(byte)255,(byte)255);
    else
      display.beats(data.value0,red,green,blue);

  }
  
  /*--SNAKE--*/
  else if(data.mode==18){
    display.snake();
  }
  
  /*--SNAKEMOVE--*/
  else if(data.mode==19){
    display.snakeMove(data.value0);
  }
  
  /*--SNAKEEAT--*/
  else if(data.mode==20){
    display.snakeEat();
    data.mode=19;
    data.value0=display.getSnakeDirection();
  }
  
  /*--FIREPLACE--*/
  else if(data.mode==21){
    display.fireplace();
  }
  
  /*--STROBE--*/
  else if(data.mode==22){
    display.strobe(data.value7,data.value8, red, green, blue); 
  }
  
  
  /*--PONG1--*/
  else if(data.mode==23){
    display.pong1();
  }
  
  /*--PONG1MOVEPLATFROM--*/
  else if(data.mode==24){
    if(firstTime){
      if(data.value1!=prevValue1){
        display.pong1MovePlatformTo(data.value1-1);
        prevValue1=data.value1;
      }else
        display.pong1MovePlatform(data.value0);
    }else
      display.pong1MovePlatform(0);
  }
  
  /*--TETRIS--*/
  else if(data.mode==25){
    display.tetris();
  }
  
  /*--TETRISMOVE--*/
  else if(data.mode==26){
    if(firstTime)
      display.tetrisMove(data.value0);
    else
      display.tetrisMove(0);
  }
  
  /*--DIGITALCLOCK--*/
  else if(data.mode==27){
    display.digitalClock(data.value0,data.value1);
  }
  
  /*--BINARYCLOCK--*/
  else if(data.mode==28){
    display.binaryClock(data.value0,data.value1,data.value2);
  }
  
  /*--TEMPERATURE--*/
  else if(data.mode==29){
  	sensors.requestTemperatures();
  	float temp=sensors.getTempCByIndex(0);
  	display.printTemperature(temp);
  }
  
  /*--LOADING--*/
  else if(data.mode==30){
  	display.loading();
  }
  
  /*--FIREWORKS--*/
  else if(data.mode==31){
  	display.fireworks();
  }
  
  /*--GAME OF LIFE--*/
  else if(data.mode==32){
  	if(firstTime){
  		display.clear();
  		display.gameOfLifeFill();
  	}
  	display.gameOfLife();
  }
  
  /*--FLAME--*/
  else if(data.mode==33){
  	if(firstTime){
		sourceFixed.x = 112; //Initialize vars
		sourceFixed.y = 1;
		Emitter_Fountain::minLife = 20;
		Emitter_Fountain::maxLife = 80;
		Particle_Std::ay = 1;
		pMatrix.reset();
    }
    pSysFixed.update();
    pMatrix.reset();
    pMatrix.render(particlesStd, numParticles);
    display.drawPartMatrix(pMatrix);
    delay(40);
  }
  
  /*--SMOKER--*/
  else if(data.mode==34){
  	if(firstTime){
		sourceAttractor.vx = 3;
		sourceAttractor.vy = 1;
		sourceAttractor.x = random(50)+100;
		sourceAttractor.y = random(10)+1;
		Particle_Std::ay = 1;
		PartMatrix::isOverflow = false;
		Emitter_Fountain::minLife = 100;
		Emitter_Fountain::maxLife = 200;
		ParticleSys::perCycle = 2;
		Particle_Attractor::atf = 2;
		pMatrix.reset();
	}
	pSysAttractor.update();
	pMatrix.reset();
    pMatrix.render(particlesStd, numParticles);
    display.drawPartMatrix(pMatrix);
    delay(50);
  }
  
  /*--SPIN--*/
  else if(data.mode==35){
	if(firstTime){
		PartMatrix::isOverflow = true;
		emitterSpin.oscilate = true;
	}
	pSysSpin.update();
	pMatrix.reset();
	pMatrix.render(particlesBounce, numParticles);
	display.drawPartMatrix(pMatrix);
	delay(30);
  }
  
  /*--RAINBOWSCRLOLL--*/
  else if(data.mode==36){
  	display.setFirstTime(firstTime);
  
    byte d;
    if(data.value0==0) d=10;
    else d=data.value0;
    
    byte step1;
    if(data.value1==0) step1=30;
    else step1=data.value1;
    
    byte step2;
    if(data.value2==0) step2=2;
    else step2=data.value2;
    
    display.rainbowRadial(d, step1, step2);
  }
  
  /*--WAKEUP--*/
  else if(data.mode==37){
  	display.setFirstTime(firstTime);
    display.wakeUp();
  }


  firstTime=false;
  previousMode=data.mode;
}

