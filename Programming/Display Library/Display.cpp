#include "Arduino.h"
#include "Display.h"
#include <avr/pgmspace.h>

#include <PartMatrix.h>

//-CONVERT HSV VALUE TO RGB //credit: https://github.com/andymortland/msp430_ws2811_energia_routines/blob/master/rainbox_strip
void HSVtoRGB(int hue, byte sat, byte val, byte colors[3]) {
	// hue: 0-359, sat: 0-255, val (lightness): 0-255
	int r, g, b, base;

	if (sat == 0) { // Achromatic color (gray).
		colors[0]=val;
		colors[1]=val;
		colors[2]=val;
	} else  {
		base = ((255 - sat) * val)>>8;
		switch(hue/60) {
			case 0: r = val; g = (((val-base)*hue)/60)+base; b = base; break;
			case 1: r = (((val-base)*(60-(hue%60)))/60)+base; g = val; b = base; break;
			case 2: r = base; g = val; b = (((val-base)*(hue%60))/60)+base; break;
			case 3: r = base; g = (((val-base)*(60-(hue%60)))/60)+base; b = val; break;
			case 4: r = (((val-base)*(hue%60))/60)+base; g = base; b = val; break;
			case 5: r = val; g = base; b = (((val-base)*(60-(hue%60)))/60)+base; break;
		}
		colors[0]=r;
		colors[1]=g;
		colors[2]=b;
	}
}

void RGBtoHSV( byte inRed, byte inGreen, byte inBlue, int hsv[3] ){ //http://www.instructables.com/id/RGB-Led-game-shield-for-arduino/step5/The-code/
  double vals[3]; 
  unsigned char maxc=0, minc=0; 
  double hue, sat, val;

  vals[0]=inRed;
  vals[1]=inGreen;
  vals[2]=inBlue;
  //red is set as maximum and minimum
  if(vals[1]>vals[maxc]) maxc=1; 
  if(vals[2]>vals[maxc]) maxc=2; 
  if(vals[1]<vals[minc]) minc=1; 
  if(vals[2]<vals[minc]) minc=2; 
  val = vals[maxc]; 
  if(vals[maxc]==0) 
    sat = hue = 0; 
  else { 
    sat=255*(1-(vals[minc]/vals[maxc])); 
    hue = 60 * ((maxc*2) + (vals[(maxc+1)%3] - vals[(maxc+2)%3])/(vals[maxc] - vals[minc])); 
  }
  if(hue < 0) hue += 360; //corrector for hues in -60 to 0 range
  hsv[0] = hue; //map(hue,0,360,0,255);
  hsv[1] = sat;
  hsv[2] = val;
}



Display::Display(byte r){
	n=r*r;
	ribbe=r;
	FastSPI_LED.setLeds(n);
	FastSPI_LED.setChipset(CFastSPI_LED::SPI_WS2801);
	FastSPI_LED.setDataRate(2);
	FastSPI_LED.init();
	FastSPI_LED.start();
	leds=(struct CRGB*)FastSPI_LED.getRGBData();
	ihue=0;
	index=0;
	firstTime=true;
	randomSeed(analogRead(0));
	touchEnabled=false;
	state1=false;
	state2=false;
	timer1=0;
	timer2=0;
	busy=false;
	a=0;
	p1=0;
	p2=0;
	p3=0;
	fireworksState=0;
	value=255;
	gameOfLifeI=0;
	rotation=0;
}

void Display::setFirstTime(boolean first){
	firstTime=first;
}

void Display::reset(){
	firstTime=true;
	index=0;
	ihue=0;
}
void Display::show(){
	FastSPI_LED.show();
}

void Display::clear(){
	reset();
	memset(leds, 0, n*3);
	FastSPI_LED.show();
}

void Display::clear(boolean show){
	reset();
	memset(leds, 0, n*3);
	if(show) FastSPI_LED.show();
}

void Display::fadeOut(){
	fadeOut(500);
}

void Display::fadeOut(int duration){
	reset();
	for(int i=1; i<=255; i++){
		for(int j=0; j<n; j++){
			if(leds[j].r>0)leds[j].r=leds[j].r-1;
			if(leds[j].g>0)leds[j].g=leds[j].g-1;
			if(leds[j].b>0)leds[j].b=leds[j].b-1;
		}
		show();
		delay(duration/255);
	}
}

void Display::setPixel(byte p, byte red, byte green, byte blue){
	leds[p].r=red;
	leds[p].g=green;
	leds[p].b=blue;
}

void Display::setPixel(byte p, byte red, byte green, byte blue, float alpha){
	setPixel(p,red*alpha, green*alpha, blue*alpha);
}

void Display::setRotation(byte rot){
	rotation = rot;
}
byte Display::getRotation(){
	return rotation;
}

int Display::getPin(byte x, byte y){
	byte x1 = x;
	byte y1 = y;
	for(byte r=0; r<rotation; r++){
		byte temp = x1;
		x1 = y1;
		y1 = 7 - temp;
	}
	
	byte pin;
	if(x1 < ribbe && y1 < ribbe && x1 >= 0 && y1 >= 0){
		if(y1 % 2 == 0) pin = y1 * ribbe + (ribbe- x1 -1);
		else pin = y1 * ribbe + x1;
		return pin;
	}
	return -1;
}

void Display::setPixel(byte x, byte y, byte red, byte green, byte blue){
	byte x1 = x;
	byte y1 = y;
	for(byte r=0; r<rotation; r++){
		byte temp = x1;
		x1 = y1;
		y1 = 7 - temp;
	}
	
	byte pin;
	if(x1<ribbe&&y1<ribbe&&x1>=0&&y1>=0){
		if(y1%2==0) pin=y1*ribbe+(ribbe-x1-1);
		else pin=y1*ribbe+x1;
		setPixel(pin,red,green,blue);
	}
}

void Display::setPixel(byte x, byte y, byte red, byte green, byte blue, float alpha){
	setPixel(x, y,red*alpha, green*alpha, blue*alpha);
}

void Display::setAll(byte red, byte green, byte blue, float alpha){
	reset();
	for(int i=0; i<n; i++){
		setPixel(i, red, green, blue, alpha);
	}
}

void Display::setAll(byte red, byte green, byte blue){
	setAll(red, green, blue, 1);
}

void Display::drawCircle(byte x0, byte y0, byte r, byte red, byte green, byte blue){
	reset();
	int y;
	for(byte x=0; x<ribbe; x++){
		if(r*r-(x-x0)*(x-x0)>=0){
			y=sqrt(r*r-(x-x0)*(x-x0))+y0;
			
			if(sqrt(r*r-(x-x0)*(x-x0))+y0-y>=0.4) y=ceil(sqrt(r*r-(x-x0)*(x-x0))+y0);
			else if(sqrt(r*r-(x-x0)*(x-x0))+y0-y<=0.2) y=floor(sqrt(r*r-(x-x0)*(x-x0))+y0)+1;
			else y=floor(sqrt(r*r-(x-x0)*(x-x0))+y0);
		
			setPixel(x,y,red,green,blue);
			setPixel(x,2*y0-y+1,red,green,blue);
		}
	}
}

void Display::drawRectangle(byte x0, byte y0, byte h, byte b, byte red, byte green, byte blue){
	reset();
	for(byte x=x0; x<x0+b; x++){
		setPixel(x,y0,red,green,blue);
		setPixel(x,y0+h-1,red,green,blue);
	}
	for(byte y=y0; y<y0+h-1; y++){
		setPixel(x0,y,red,green,blue);
		setPixel(x0+b-1,y,red,green,blue);
	}
}

void Display::fillRandom(float p){
	reset();
	for(byte i=0; i<n*p; i++){
		setPixel(random(n),random(255),random(255),random(255));
	}
}

void Display::drawCharacter(char c, byte red, byte green, byte blue){
	drawCharacter(c, 0, 0, red, green, blue);
}
void Display::drawCharacter(char c, byte x0, byte y0, byte red, byte green, byte blue){
	reset();
	byte b=(byte) c;
	if(b>='A'&&b<='Z')
		b=b-65;
	else if(b>='a'&&b<='z')
		b=b-97;
	else if(b>='0'&&b<='9')
		b=b-48+26;
	else if(b==' ')
		b=36;
	
	byte x;
	byte y;
	
	for(int i=0; i<24; i++){
		byte h=pgm_read_byte(&table[b][i]);
		if(h<99){
			if(h<10){
				x=0;
				y=h;
			}else{
				x=String(pgm_read_byte(&table[b][i])).charAt(0)-48;
				y=String(pgm_read_byte(&table[b][i])).charAt(1)-48;
			}
			setPixel(x+x0,y+y0,red,green,blue);
		}
	}
	
}

void Display::scrollLeft(String s, int d, byte red, byte green, byte blue){
	reset();
	for(byte y0=0; y0<s.length()*5+8; y0++){
		clear(false);
		for(byte i=0; i<s.length(); i++){
			drawCharacter(s.charAt(i), i*5-y0+8, 0, red, green, blue);
		}
		delay(d);
		show();		
	}
}

void Display::rainbow(int d){
	ihue++;
	if(ihue>=359) ihue=0;
	byte color[3];
	HSVtoRGB(ihue,255,255, color);
	Serial.println(touchEnabled);
	for(byte i=0; i<n; i++){	
		if(i<8 && getTouch(i)==1 && touchEnabled)
			setPixel(i,255-color[0],255-color[1],255-color[2]);
		else
			setPixel(i,color[0],color[1],color[2]);
	}
	show();
	delay(d);
}

void Display::rainbow(){
	rainbow(20);
}

void Display::rainbowScroll(int d, byte step1, byte step2){
	if(firstTime){
		clear(false);
		for(byte y=0; y<ribbe; y++){
			for(byte x=0; x<ribbe; x++){
				ihue=ihue+step1;
				if(ihue>=359) ihue=0;
				byte color[3];
				HSVtoRGB(ihue,255,255,color);
				setPixel(x,y,color[0],color[1],color[2]);
			}
		}
		firstTime=false;
		
	}
	for(byte i=0; i<n; i++){
		int colorHSV[3]; //Convert back to HSV
		RGBtoHSV(leds[i].r,leds[i].g,leds[i].b, colorHSV); 
		int hue=colorHSV[0]+step2;//Progress in HSV space
		byte colorRGB[3];//Convert back to RGB
		if(hue>=359) hue=0;
		HSVtoRGB(hue,255,255,colorRGB);
		setPixel(i,colorRGB[0],colorRGB[1],colorRGB[2]);
	}
	show();
	delay(d);
}

void Display::rainbowScroll(int d){
	rainbowScroll(d,4,2);
}

void Display::rainbowRadial(int d, byte step1, byte step2){
	if(firstTime){
		clear(false);
		for(byte x=0; x<ribbe; x++){
			for(byte y=0; y<ribbe; y++){
				float d = sqrt(pow((x-3.5),2)+pow((y-3.5),2));
				byte color[3];
				HSVtoRGB(d*step1,255,255,color);
				setPixel(x,y,color[0],color[1],color[2]);
			}
		}
		firstTime=false;
		
	}
	for(byte i=0; i<n; i++){
		int colorHSV[3]; //Convert back to HSV
		RGBtoHSV(leds[i].r,leds[i].g,leds[i].b, colorHSV);
		int hue=colorHSV[0]+step2; //Progress in HSV space
		byte colorRGB[3]; //Convert back to RGB
		if(hue>=359) hue=0;
		HSVtoRGB(hue,255,255,colorRGB);
		
		setPixel(i,colorRGB[0],colorRGB[1],colorRGB[2]);
	}
	show();
	delay(d);
}
void Display::stars(int d){
	reset();
	
	if(!busy){
		p1=random(0,n);
		p2=random(0,n);
		p3=random(0,n);
	}
	
	unsigned long now=millis();
	if(p1!=p2 && p1!=p3 && p2!=p3){
		
		if(now - timer1 > d){
			timer1=now;
			if(leds[p1].r!=255) setPixel(p1,a,a,a);
			if(leds[p2].r!=0) setPixel(p2,255-a,255-a,255-a);
			if(leds[p3].r!=0) setPixel(p3,255-a,255-a,255-a);
			show();
			
			a++;
			busy=true;
		}
		if(a>=255){
			busy=false;
			a=0;
		}
	}
	
}

void Display::eq(byte h1, byte h2, byte h3, byte h4, byte h5, byte h6, byte h7, byte red, byte green, byte blue){
	reset();
	byte h0;
	clear(false);
	for(byte b=0; b<8; b++){
		switch(b){
			case 0: h0=h1;break;
			case 1: h0=h2;break;
			case 2: h0=h3;break;
			case 3: h0=h4;break;
			case 4: h0=h5;break;
			case 5: h0=h6;break;
			case 6: h0=h7;break;
			case 7: h0=(h1+h2+h3+h4+h5+h6+h7)/7;break;
		}
		for(byte h=0; h<=h0; h++){
			setPixel(b,h-1,red,green,blue);
		}
	}
	show();
}

void Display::eq(byte h1, byte h2, byte h3, byte h4, byte h5, byte h6, byte h7){
	reset();
	byte h0;
	clear(false);
	for(byte b=0; b<8; b++){
		switch(b){
			case 0: h0=h1;break;
			case 1: h0=h2;break;
			case 2: h0=h3;break;
			case 3: h0=h4;break;
			case 4: h0=h5;break;
			case 5: h0=h6;break;
			case 6: h0=h7;break;
			case 7: h0=(h1+h2+h3+h4+h5+h6+h7)/7;break;
		}
		for(byte h=0; h<=h0; h++){
			byte color[3];
			HSVtoRGB(40+40*(h-1),255,255, color);
			setPixel(b,h-1,color[0],color[1],color[2]);
		}
	}
	show();
}

void Display::beats(byte h1, byte red, byte green, byte blue){
	reset();
	clear(false);
	Serial.println(h1);
	float val = pow(h1,2) / pow(180,2);
	Serial.println(val);
	setAll((byte)(val*red), (byte)(val*green), (byte)(val*blue));
	show();
}

void Display::enableTouch(){
	touchEnabled=true;
}

void Display::disableTouch(){
	touchEnabled=false;
}

byte Display::getTouch(byte p){
	for(byte bit=0; bit<3; bit++){
		byte isBitSet=bitRead(p,bit);
		digitalWrite(dataTouchPins[bit],isBitSet);
		Serial.print(isBitSet);
	}
	Serial.println("");
	Serial.println(digitalRead(TOUCHPIN));
	return !digitalRead(TOUCHPIN);
}

byte Display::getTouch(byte x, byte y){
	byte pin;
	if(x<ribbe&&y<ribbe&&x>=0&&y>=0){
		if(y%2==0) pin=y*ribbe+(ribbe-x-1);
		else pin=y*ribbe+x;
	}
	return getTouch(pin); 
}

void Display::snake(){
	clear();
	snakeBody[0][0]=4;
	snakeBody[0][1]=4;
	snakeBodyLength=1;
	snakeNewFood();
	snakeShow();
}
void Display::snakeMove(byte move){ 

	/*   1
	   4   2
	   	 3
	*/
	
	int interval;
	if(500-snakeBodyLength*20>=100)
		interval=500-snakeBodyLength*20;
	else
		interval=100;
		
	unsigned long now=millis();
	if(now - timer1 > interval){ //Grote intervallen
		timer1=now;
		
		
		for(byte l=0; l<snakeBodyLength; l++){
		
			if(!((direction==4&&move==2)||(direction==2&&move==4)||(direction==1&&move==3)||(direction==3&&move==1))){
				direction=move;
			}
			int vorig[2];
			int tmp[2];

			if(l==0){
				vorig[0]=snakeBody[l][0]; vorig[1]=snakeBody[l][1];
				if(direction==4) snakeBody[l][0]--;
				else if(direction==2) snakeBody[l][0]++;
				else if(direction==1) snakeBody[l][1]++;
				else if(direction==3) snakeBody[l][1]--;
			
				if(snakeBody[l][0]<0) snakeBody[l][0]=7;
				if(snakeBody[l][1]<0) snakeBody[l][1]=7;
				if(snakeBody[l][0]>7) snakeBody[l][0]=0;
				if(snakeBody[l][1]>7) snakeBody[l][1]=0;
			}else{
				tmp[0]=snakeBody[l][0]; tmp[1]=snakeBody[l][1];
				snakeBody[l][0]=vorig[0]; snakeBody[l][1]=vorig[1];
				vorig[0]=tmp[0]; vorig[1]=tmp[1];
			}
		
		}
		if(snakeBody[0][0]==foodX && snakeBody[0][1]==foodY){
			snakeEat();
			snakeNewFood();
			
		}
		if(findInBody(snakeBody[0][0],snakeBody[0][1],(byte)1))
			snakeGameOver();
			
		snakeShow();
	}
	
	//delay(500);
}
void Display::snakeShow(){ 
	clear(false);
	for(byte l=0; l<snakeBodyLength; l++){
		setPixel((byte)snakeBody[l][0],(byte)snakeBody[l][1],(byte)150,(byte)255,(byte)0);
	}
	setPixel(foodX,foodY,(byte)245,(byte)20,(byte)5);
	show();
}
void Display::snakeEat(){
	int vorigX=snakeBody[snakeBodyLength-1][0];
	int vorigY=snakeBody[snakeBodyLength-1][1];
	
	if(direction==4){
		snakeBody[snakeBodyLength][0]=vorigX+1;
		snakeBody[snakeBodyLength][1]=vorigY;
	}
	else if(direction==2){
		snakeBody[snakeBodyLength][0]=vorigX-1;
		snakeBody[snakeBodyLength][1]=vorigY;
	}
	else if(direction==1){
		snakeBody[snakeBodyLength][0]=vorigX;
		snakeBody[snakeBodyLength][1]=vorigY-1;
	}
	else if(direction==3){
		snakeBody[snakeBodyLength][0]=vorigX;
		snakeBody[snakeBodyLength][1]=vorigY+1;
	}
	snakeBodyLength=snakeBodyLength+1;
}
byte Display::getSnakeDirection(){
	return direction;
}
boolean Display::findInBody(byte x, byte y, byte start){
	for(byte l=start; l<snakeBodyLength; l++){
		if(snakeBody[l][0]==x && snakeBody[l][1]==y)
			return true;
	}
	return false;
}
void Display::snakeGameOver(){
	scrollLeft("Game over", 30, (byte)150,(byte)255,(byte)0);
	snake();
}
void Display::snakeNewFood(){
	do{
		foodX=random(0,ribbe);
		foodY=random(0,ribbe);
	}while(findInBody(foodX,foodY,(byte)0));
}
void Display::fireplace(){ //Modified from: http://www.instructables.com/id/8x8-LED-Matrix-with-flames/
	byte scn[ribbe][ribbe];
	byte r, c, t;
	//advance fire
	for(r = 0; r < ribbe - 1; r++) 
		for(c = 0; c < ribbe; c++){
			t = scn[r + 1][c] << 1;
			t += scn[r][c] >> 1;
			t += (c ? scn[r + 1][c - 1] : scn[r + 1][ribbe - 1]) >> 1;
			t += (c == ribbe - 1 ? scn[r + 1][0] : scn[r + 1][c + 1]) >> 1;
			t >>= 2;
			scn[r][c] = t;
			setPixel(c, abs(r-7), (byte)constrain(2*t+pow(t,2.5),0,255),  (byte)constrain(2*t+pow(t,2.5),0,255)*0.4, (byte)0);
		}
		
	//generate random seeds on the bottom
	for(c = 0; c < ribbe; c++) {
			
		t = (rand() > 0xB0) ? rand() & 0x0F : (scn[ribbe - 1][c] ? scn[ribbe - 1][c] - 1 : 0);
			
		scn[ribbe - 1][c] = t;

		setPixel(c, abs((ribbe - 1)-7), (byte)constrain(3*t+pow(t,2.5),0,255),  (byte)constrain(2*t+pow(t,2.5),0,255)*0.4, (byte)0);
	}
	show();
	delay(80);
}

void Display::strobe(unsigned int freq1, unsigned int del, byte red, byte green, byte blue){
	unsigned int interval1;
	unsigned int interval2;
	if(freq1!=0) interval1 = 1000000/freq1;
	else interval1 = 0;
	interval2 = del;
	
	unsigned long now=millis();
	if(now - timer2 > interval2){ //Grote intervallen
		timer2=now;
		if(!state2){
			state2 = true;
		}else{
			if(interval2 != 0){
				state2 = false;
				state1 = false;
				clear(1);
			}
		}
	}
	if((now - timer1 > interval1) && state2){ //Kleine intervallen
		timer1=now;
		if(!state1){
			setAll(red, green, blue);
			show();
			state1 = true;
		}else{
			clear(1);
			state1 = false;
		}
	}
}

void Display::pong1(){
	clear();
	pongPosX = random(0,8);
	pongPosY = random(2,8);
	pong1Platform = 4;
	pongDeltaX = 1;
	pongDeltaY = 1;
	pong1Show();
}

void Display::pong1Move(){
	unsigned long now = millis();
	int interval = 200;
	if(now - timer1 > interval){ //Grote intervallen
		timer1 = now;
		if(pongPosX == 7 && pongPosY == 7 || pongPosX == 0 && pongPosY == 7){
			pongDeltaY=-pongDeltaY;
			pongDeltaX=-pongDeltaX;
		}else if(pongPosX == 7 && pongDeltaX == 1 || pongPosX == 0 && pongDeltaX == -1){
			pongDeltaX = -pongDeltaX;
		}else if(pongPosY == 7 && pongDeltaY == 1){
			pongDeltaY =- pongDeltaY;
		} 
		pongPosX = pongPosX+pongDeltaX;
		pongPosY = pongPosY+pongDeltaY;
	
		if(pongPosY == 1 && ( pong1Platform == pongPosX || pong1Platform == pongPosX-1 || pong1Platform == pongPosX+1)){
			pongDeltaY =- pongDeltaY;
			if(pong1Platform == pongPosX-1)
				if(pongDeltaX == 0)
					pongDeltaY =- 1;
				else if(pongDeltaY == 1)
					pongDeltaX == 0;
			else if(pong1Platform == pongPosX - 1)
				if(pongDeltaX == 0)
					pongDeltaY = 1;
				else if(pongDeltaY == -1)
					pongDeltaX == 0;
		}
	
		if(pongPosY == -1)
			pong1GameOver();
	}
}

void Display::pong1MovePlatform(byte direction){
	pong1Move();
	if(direction == 2 && pong1Platform < 8) pong1Platform += 1;
	else if(direction == 4 && pong1Platform > -1) pong1Platform -= 1;
	pong1Show();
}
void Display::pong1MovePlatformTo(byte position){
	pong1Move();
	pong1Platform = position;
	pong1Show();
}
void Display::pong1Show(){
	clear(false);
	setPixel(pongPosX, pongPosY, (byte)255, (byte)255, (byte)255);
	
	setPixel(pong1Platform-1, 0, (byte)255, (byte)255, (byte)255);
	setPixel(pong1Platform, 0, (byte)255, (byte)255, (byte)255);
	setPixel(pong1Platform+1, 0, (byte)255, (byte)255, (byte)255);
	
	show();
}
void Display::pong1GameOver(){
	scrollLeft("Game over", 30, (byte)255,(byte)255,(byte)255);
	pong1();
}

void Display::tetris(){
	clear();
	for(byte x = 0; x < 8; x++){
		for(byte y = 0; y < 8; y++){
			matrix[x][y] = false;
		}
	}
	tetrisNewBlock();
	tetrisShow();
	tetrisActiveBlockRotation = 1;
}
void Display::tetrisMove(byte direction){
	tetrisClearBlock();
	tetrisLines();
	int prevTetrisActiveBlockPosX = tetrisActiveBlockPosX;
	int prevTetrisActiveBlockPosY = tetrisActiveBlockPosY;
	if(direction == 1){
		for(byte i=0; i<4; i++){
			int x = tetrisActiveBlocks[i][0];
			int y = tetrisActiveBlocks[i][1];
			
			tetrisActiveBlocks[i][0]=y;
			tetrisActiveBlocks[i][1]=-x;
		}
	}else if(direction == 2) 
		tetrisActiveBlockPosX += 1;
	else if(direction == 3)
		tetrisActiveBlockPosY -= 1;
	else if(direction == 4)
		tetrisActiveBlockPosX -= 1;
	else if(direction == 0){
		unsigned long now = millis();
		int interval = 1000;
		if(now - timer1 > interval){
			timer1 = now;
			tetrisActiveBlockPosY -= 1;
		}
	}
	
	if(tetrisCheckSet()){
		tetrisActiveBlockPosX=prevTetrisActiveBlockPosX;
		tetrisActiveBlockPosY=prevTetrisActiveBlockPosY;
		tetrisSet();
	}else if(tetrisCheckCollision()){
		tetrisActiveBlockPosX=prevTetrisActiveBlockPosX;
		tetrisActiveBlockPosY=prevTetrisActiveBlockPosY;
	}
	
	tetrisShow();
}
void Display::tetrisShow(){
	byte blockRed;
	byte blockGreen;
	byte blockBlue;
	switch(tetrisActiveBlock){
		case 0: blockRed=3; blockGreen=226; blockBlue=252; break; //I
		case 1: blockRed=252; blockGreen=10; blockBlue=3; break;  //Z
		case 2: blockRed=30; blockGreen=252; blockBlue=3; break;  //S
		case 3: blockRed=190; blockGreen=3; blockBlue=252; break; //T
		case 4: blockRed=252; blockGreen=230; blockBlue=0; break; //O
		case 5: blockRed=252; blockGreen=70; blockBlue=3; break;  //L
		case 6: blockRed=3; blockGreen=30; blockBlue=255; break;  //J
	}
	for(byte i=0; i<4; i++){
		byte x = tetrisActiveBlockPosX + tetrisActiveBlocks[i][0];
		byte y = tetrisActiveBlockPosY + tetrisActiveBlocks[i][1];
		setPixel(x,y,blockRed,blockGreen,blockBlue);
	}
	show();
}
boolean Display::tetrisCheckCollision(){
	for(byte i=0; i<4; i++){
		int x = tetrisActiveBlockPosX + tetrisActiveBlocks[i][0];
		int y = tetrisActiveBlockPosY + tetrisActiveBlocks[i][1];
		
		if(matrix[x][y] || x<0 || y<0 || x>7)
			return true;
	}
	return false;
}
boolean Display::tetrisCheckSet(){
	for(byte i=0; i<4; i++){
		int x = tetrisActiveBlockPosX + tetrisActiveBlocks[i][0];
		int y = tetrisActiveBlockPosY + tetrisActiveBlocks[i][1];
		if(x >= 0 && x<=7 && y>=0 && y<=7){
			if(matrix[x][y]){
				Serial.println(matrix[x][y]);
				return true;
			}
		} else if(y<0){
			return true;
		}
	}
	return false;
}
void Display::tetrisSet(){
	for(byte i=0; i<4; i++){
		byte x = tetrisActiveBlockPosX+tetrisActiveBlocks[i][0];
		byte y = tetrisActiveBlockPosY+tetrisActiveBlocks[i][1];
		matrix[x][y]=true;
	}
	tetrisShow();
	tetrisNewBlock();
}
void Display::tetrisClearBlock(){
	for(byte i=0; i<4; i++){
		byte x = tetrisActiveBlockPosX+tetrisActiveBlocks[i][0];
		byte y = tetrisActiveBlockPosY+tetrisActiveBlocks[i][1];
		setPixel(x,y,(byte)0,(byte)0,(byte)0);
	}
}
void Display::tetrisNewBlock(){
	tetrisActiveBlock = random(0,7);
	for(byte i=0; i<4; i++){
		byte x = pgm_read_byte(&tetrisBlocks[tetrisActiveBlock][i][0]);
		byte y = pgm_read_byte(&tetrisBlocks[tetrisActiveBlock][i][1]);
		tetrisActiveBlocks[i][0]=x;
		tetrisActiveBlocks[i][1]=y;
	}
	tetrisActiveBlockPosX = 2;
	tetrisActiveBlockPosY = 6;
	if(tetrisCheckCollision())
		tetrisGameOver();
}
void Display::tetrisGameOver(){
	delay(300);
	scrollLeft("Game over", 30, (byte)200,(byte)30,(byte)0);
	tetris();
}
void Display::tetrisLines(){
	for(byte line = 0; line < 8; line++){
		boolean completeLine = true;
		byte block = 0; 
		while(block < 8 && completeLine){
			completeLine = matrix[block][line];
			block++;
		}
		if(completeLine){
			tetrisRemoveLine(line);
		}
	}
}
void Display::tetrisRemoveLine(byte line){
	for(byte y=line; y<7; y++){
		for(byte x=0; x<8; x++){
			matrix[x][y]=matrix[x][y+1];
			setPixel(x,y,leds[getPin(x,y+1)].r,leds[getPin(x,y+1)].g,leds[getPin(x,y+1)].b);
		}
	} 
	for(byte x=0; x<8; x++){
		matrix[x][7]=0;
		setPixel(x,7,(byte)0,(byte)0,(byte)0);
	}
	
	
}

void Display::digitalClock(byte hour, byte minute){
	clear(false);
	printClockNumber(hour,4,(byte)255,(byte)247,(byte)201);
	printClockNumber(minute,0,(byte)255,(byte)222,(byte)85);
	show();
}

void Display::printClockNumber(byte hour,byte startY, byte red, byte green, byte blue){
	byte numberCount;
	if(hour>9) numberCount=2;
	else numberCount=1;
	for(byte j=0; j<numberCount; j++){
		byte startX=j*4+abs(j-1);
		byte number=hour;
		if(numberCount==2){
			if(j==0) number=hour/10;
			else if(j==1) number=hour%10;
		}else if(numberCount==1){
			number=hour;
		}
		for(byte i=0; i<8; i++){
			if(pgm_read_byte(&smallNumbers[number][i][0]) < 8){
				byte x = pgm_read_byte(&smallNumbers[number][i][0]);
				byte y = pgm_read_byte(&smallNumbers[number][i][1]);
				setPixel(startX+x,startY+y,red,green,blue);
			}
		}
	}
}

void Display::binaryClock(byte hour, byte minute, byte second){
	clear(false);
	printBinaryClock(hour,(byte)0,(byte)255,(byte)247,(byte)201);
	printBinaryClock(minute,(byte)3,(byte)255,(byte)222,(byte)85);
	printBinaryClock(second,(byte)6,(byte)255,(byte)180,(byte)40);
	show();
}

void Display::printBinaryClock(byte hour, byte startX, byte red, byte green, byte blue){
	byte rest=hour;
	if(rest/40>=1){
		setPixel(startX+0,2*2,red, green, blue);
		setPixel(startX+0,2*2+1,red, green, blue);
		rest=rest-40;
	}
	if(rest/20>=1){
		setPixel(startX+0,1*2,red, green, blue);
		setPixel(startX+0,1*2+1,red, green, blue);
		rest=rest-20;
	}
	if(rest/10>=1){
		setPixel(startX+0,0*2,red, green, blue);
		setPixel(startX+0,0*2+1,red, green, blue);
		rest=rest-10;
	}
	if(rest/8>=1){
		setPixel(startX+1,3*2,red, green, blue);
		setPixel(startX+1,3*2+1,red, green, blue);
		rest=rest-8;
	}
	if(rest/4>=1){
		setPixel(startX+1,2*2,red, green, blue);
		setPixel(startX+1,2*2+1,red, green, blue);
		rest=rest-4;
	}
	if(rest/2>=1){
		setPixel(startX+1,1*2,red, green, blue);
		setPixel(startX+1,1*2+1,red, green, blue);
		rest=rest-2;
	}
	if(rest/1>=1){
		setPixel(startX+1,0*2,red, green, blue);
		setPixel(startX+1,0*2+1,red, green, blue);
		rest=rest-1;
	}
}

void Display::printTemperature(float temp){
	clear(false);
	int t = (int)floor(temp);
	int r = constrain(map(t,0,30,0,255),0,255);
	if(t>0 && t<99){
		String ts=""; 
		ts+=t;
		for(byte i; i<ts.length(); i++){
			drawCharacter(ts.charAt(i),i*4,0,r,r/3,255-r);
		}
	}
	show();
}

void Display::loading(){
	int interval1=500;
	unsigned long now = millis();
	if(now - timer1 > interval1){ //Kleine intervallen
		timer1=now;
		if(!state1){
			ihue=random(0,360);
			byte color[3];
			HSVtoRGB(ihue,200,200, color);
			setPixel((byte)3, (byte)3, color[0],color[1],color[2]);
			setPixel((byte)4, (byte)3, color[0],color[1],color[2]);
			setPixel((byte)3, (byte)4, color[0],color[1],color[2]);
			setPixel((byte)4, (byte)4, color[0],color[1],color[2]);
			show();
			state1 = true;
		}else{
			clear(1);
			state1 = false;
		}
	}
}

void Display::fireworks(){
	clear(false);
	int interval1=10;
	unsigned long now = millis();
	if(now - timer1 > interval1){
		byte color[3];
		timer1=now;
		if(fireworksState==0&&value==255){
			show();
			delay(400);
			fireworksActiveCenterX=random(0,ribbe);
			fireworksActiveCenterY=random(0,ribbe);
			fireworksHue=random(0,360);
		}
		
		HSVtoRGB(fireworksHue,255,value, color);
		for(byte r=0; r<=fireworksState; r++){
			drawCircle(fireworksActiveCenterX, fireworksActiveCenterY, r, color[0], color[1], color[2]);
		}
		if(value%50==0&&value!=0){
			fireworksState++;
		}
		value-=5;
		if(value<=0){
			value=255;
			fireworksState=0;
		}
		show();
	}
}

void Display::gameOfLifeFill(){
	clear(false);
	//gameOfLifeGeneration=1;
	gameOfLifeI=0;
	byte color[3];
	HSVtoRGB(gameOfLifeGeneration*20,255,200, color);
	for(byte i=0; i<ribbe*ribbe; i++){
		setPixel(random(0,ribbe*ribbe), color[0],color[1],color[2]);
		gameOfLifeI++;
	}
	show();
	timer1=millis();
}
void Display::gameOfLife(){
	boolean c=true;
	int interval1=500;
	unsigned long now = millis();
	if(now - timer1 > interval1){
		timer1=now;
		if(gameOfLifeI<=0){
			gameOfLifeFill();
		}
		gameOfLifeI=0;
		for(byte x=0; x<ribbe; x++){
			for(byte y=0; y<ribbe; y++){
				if(leds[getPin(x,y)].r!=0 || leds[getPin(x,y)].g!=0 || leds[getPin(x,y)].b!=0){
					matrix[x][y]=true;
					gameOfLifeI++;
				}else{
					matrix[x][y]=false;
				}
			}
		}
		if(gameOfLifeI<=0){	
			gameOfLifeFill();
			c=false;
			for(byte x=0; x<ribbe; x++){
				for(byte y=0; y<ribbe; y++){
					if(leds[getPin(x,y)].r!=0 || leds[getPin(x,y)].g!=0 || leds[getPin(x,y)].b!=0){
						matrix[x][y]=true;
					}else{
						matrix[x][y]=false;
					}
				}
			}
		}
		if(c){
			byte color[3];
			HSVtoRGB(gameOfLifeGeneration*20,255,200, color);
			for(byte x=0; x<ribbe; x++){
				for(byte y=0; y<ribbe; y++){
					byte numberOfNeighbours=0;
					if(y+1<ribbe && matrix[x][y+1]){
						numberOfNeighbours++;
					}
					if(y+1<ribbe && x+1<ribbe && matrix[x+1][y+1]){
						numberOfNeighbours++;
					}
					if(x+1<ribbe && matrix[x+1][y]){
						numberOfNeighbours++;
					}
					if(y-1>=0 && x+1<ribbe && matrix[x+1][y-1]){
						numberOfNeighbours++;
					}
					if(y-1>=0 && matrix[x][y-1]){
						numberOfNeighbours++;
					}
					if(x-1>=0 && y-1>=0 && matrix[x-1][y-1]){
						numberOfNeighbours++;
					}
					if(x-1>=0 && matrix[x-1][y]){
						numberOfNeighbours++;
					}
					if(x-1>=0 && y+1<ribbe && matrix[x-1][y+1]){
						numberOfNeighbours++;
					}
					
					//if((numberOfNeighbours==2 || numberOfNeighbours==3) && matrix[x][y])
					if((numberOfNeighbours>=4 || numberOfNeighbours<2) && matrix[x][y] ){
						setPixel(x,y,(byte)0,(byte)0,(byte)0);
					}
					if(numberOfNeighbours==3 && !matrix[x][y]){
						setPixel(x,y,color[0],color[1],color[2]);
					}
			
				}
			}
		
			gameOfLifeGeneration++;
			if(gameOfLifeGeneration>=18)
				gameOfLifeGeneration=1;
		}
		show();
	}
}

void Display::drawPartMatrix(PartMatrix pMatrix){
	clear(false);
	//update the actual LED matrix
	for (byte y=0;y<PS_PIXELS_Y;y++) {
		for(byte x=0;x<PS_PIXELS_X;x++) {
			setPixel(x,y,pMatrix.matrix[x][y].r,pMatrix.matrix[x][y].g, pMatrix.matrix[x][y].b);
		}
	}
	show();
}

void Display::wakeUp(){
	int interval1=5000;
	unsigned long now = millis();
	
	byte steps = 30;
	
	if(firstTime){
		wakeUpRed = 0;
		wakeUpGreen = 0;
		wakeUpBlue = 0;
		wakeUpState = 0;
		wakeUpStep = 0;
	}
	
	if(now - timer1 > interval1){
		timer1=now;
		byte wakeUpRed1;
		byte wakeUpGreen1;
		byte wakeUpBlue1;
		//0->1
		if(wakeUpState==0){
			//Step1
			wakeUpRed1 = 6;
			wakeUpGreen1 = 20;
			wakeUpBlue1 = 40;
		}
		//1->2
		if(wakeUpState==1){
			//Step2
			wakeUpRed1 = 186;
			wakeUpGreen1 = 81;
			wakeUpBlue1 = 99;
		}
		
		//2->3
		if(wakeUpState==2){
			//Step3
			wakeUpRed1 = 172;
			wakeUpGreen1 = 132;
			wakeUpBlue1 = 42;
		}
		
		//3->4
		if(wakeUpState==3){
			//Step4
			wakeUpRed1 = 255;
			wakeUpGreen1 = 255;
			wakeUpBlue1 = 255;
		}
		
		wakeUpRed = wakeUpRed + ((wakeUpStep * (wakeUpRed1 - wakeUpRed)) / (steps - 1));
		wakeUpGreen = wakeUpGreen + ((wakeUpStep * (wakeUpGreen1 - wakeUpGreen)) / (steps - 1));
		wakeUpBlue = wakeUpBlue + ((wakeUpStep * (wakeUpBlue1 - wakeUpBlue)) / (steps - 1));
		setAll(wakeUpRed, wakeUpGreen, wakeUpBlue);
		show();
		
		wakeUpStep++;
		if(wakeUpStep >= steps && wakeUpState < 3){
			wakeUpStep=0;
			wakeUpState++;
		}
	}
}