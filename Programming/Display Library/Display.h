#ifndef Display_h
#define Display_h

#define TOUCHPIN 2

#include "Arduino.h"
#include <FastSPI_LED.h>
#include <avr/pgmspace.h>

#include <PartMatrix.h>

const byte table[37][24]PROGMEM={ //Large letters decleration
	{00, 01, 02, 03, 04, 05, 16, 12, 22, 26, 30, 31, 32, 33, 34, 35, 99, 99, 99, 99, 99, 99, 99, 99}, //A
	{00, 01, 02, 03, 04, 05, 06, 16, 13, 10, 20, 23, 26, 35, 34, 32, 31, 99, 99, 99, 99, 99, 99, 99}, //B
	{00, 01, 02, 03, 04, 05, 06, 10, 20, 30, 16, 26, 36, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //C
	{00, 01, 02, 03, 04, 05, 06, 10, 20, 16, 26, 31, 32, 33, 34, 35, 99, 99, 99, 99, 99, 99, 99, 99}, //D
	{00, 01, 02, 03, 04, 05, 06, 10, 20, 30, 13, 23, 33, 16, 26, 36, 99, 99, 99, 99, 99, 99, 99, 99}, //E
	{00, 01, 02, 03, 04, 05, 06, 13, 23, 33, 16, 26, 36, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //F
	{01, 02, 03, 04, 05, 10, 20, 16, 26, 31, 32, 33, 23, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //G
	{00, 01, 02, 03, 04, 05, 06, 13, 23, 30, 31, 32, 33, 34, 35, 36, 99, 99, 99, 99, 99, 99, 99, 99}, //H
	{00, 10, 20, 11, 12, 13, 14, 15, 16, 06, 26, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //I
	{01, 00, 10, 20, 21, 22, 23, 24, 25, 26, 16, 36, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //J
	{00, 01, 02, 03, 04, 05, 06, 13, 30, 21, 22, 24, 25, 36, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //K
	{00, 01, 02, 03, 04, 05, 06, 10, 20, 30, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //L
	{00, 01, 02, 03, 04, 05, 06, 15, 25, 30, 31, 32, 33, 34, 35, 36, 99, 99, 99, 99, 99, 99, 99, 99}, //M
	{00, 01, 02, 03, 04, 05, 06, 15, 24, 30, 31, 32, 33, 34, 35, 36, 99, 99, 99, 99, 99, 99, 99, 99}, //N
	{01, 02, 03, 04, 05, 31, 32, 33, 34, 35, 10, 20, 16, 26, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //O
	{00, 01, 02, 03, 04, 05, 06, 13, 23, 34, 35, 26, 16, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //P
	{01, 02, 03, 04, 05, 31, 32, 33, 34, 35, 10, 20, 16, 26, 21, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //Q
	{00, 01, 02, 03, 04, 05, 06, 16, 22, 13, 21, 30, 23, 34, 35, 26, 99, 99, 99, 99, 99, 99, 99, 99}, //R
	{00, 10, 20, 31, 32, 23, 13, 04, 05, 16, 26, 36, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //S
	{10, 11, 12, 13, 14, 15, 16, 06, 26, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //T
	{01, 02, 03, 04, 05, 06, 31, 32, 33, 34, 35, 36, 10, 20, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //U
	{01, 02, 03, 04, 05, 06, 31, 32, 33, 34, 35, 36, 10, 20, 11, 21, 99, 99, 99, 99, 99, 99, 99, 99}, //V
	{00, 01, 02, 03, 04, 05, 06, 30, 31, 32, 33, 34, 35, 36, 11, 21, 99, 99, 99, 99, 99, 99, 99, 99}, //W
	{00, 01, 30, 31, 02, 32, 13, 23, 34, 35, 36, 04, 05, 06, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //X
	{10, 11, 12, 13, 03, 04, 05, 06, 23, 24, 25, 26, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //Y
	{00, 10, 20, 30, 01, 02, 13, 24, 35, 36, 26, 16, 06, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //Z
	{01, 02, 03, 04, 05, 31, 32, 33, 34, 35, 10, 20, 16, 26, 06, 36, 30, 00, 99, 99, 99, 99, 99, 99}, //0
	{10, 20, 30, 21, 22, 23, 24, 25, 26, 15, 04, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //1
	{30, 20, 10, 00, 01, 02, 03, 13, 23, 33, 34, 35, 36, 26, 16, 06, 99, 99, 99, 99, 99, 99, 99, 99}, //2
	{00, 31, 32, 03, 34, 35, 06, 10, 20, 30, 13, 23, 33, 16, 26, 36, 99, 99, 99, 99, 99, 99, 99, 99}, //3
	{30, 31, 32, 33, 34, 23, 13, 03, 04, 05, 06, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //4
	{00, 10, 20, 31, 32, 23, 13, 04, 05, 16, 26, 36, 06, 03, 33, 30, 99, 99, 99, 99, 99, 99, 99, 99}, //5
	{00, 01, 02, 03, 04, 05, 06, 16, 13, 10, 20, 23, 26, 32, 31, 30, 33, 36, 99, 99, 99, 99, 99, 99}, //6
	{30, 31, 32, 33, 34, 35, 36, 26, 16, 06, 05, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, //7
	{00, 01, 02, 03, 04, 05, 06, 16, 13, 10, 20, 23, 26, 35, 34, 32, 31, 33, 36, 30, 99, 99, 99, 99}, //8
	{00, 03, 04, 05, 06, 16, 13, 10, 20, 23, 26, 35, 34, 32, 31, 33, 36, 30, 99, 99, 99, 99, 99, 99}, //9
	{99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99} //Space
};
const byte tetrisBlocks[7][4][2]PROGMEM={ //Tetris Blocks
	{{0,0},{1,0},{2,0},{3,0}}, //I
	{{0,1},{1,1},{1,0},{2,0}}, //Z
	{{0,0},{1,0},{2,1},{1,1}}, //S
	{{0,0},{1,0},{2,0},{1,1}}, //T
	{{0,0},{1,0},{1,1},{0,1}}, //O
	{{0,0},{0,1},{1,1},{2,1}}, //L
	{{0,0},{1,0},{2,0},{0,1}}  //J
};
const byte smallNumbers[10][8][2]PROGMEM={ //Small numbers
	{{0,0},{1,0},{0,1},{1,1},{9,9},{9,9},{9,9},{9,9}}, //0
	{{0,2},{1,3},{1,1},{1,0},{1,2},{9,9},{9,9},{9,9}}, //1
	{{0,0},{1,0},{0,1},{1,2},{1,3},{0,3},{9,9},{9,9}}, //2
	{{0,0},{1,0},{0,3},{1,1},{1,2},{0,2},{1,3},{9,9}}, //3
	{{0,1},{1,0},{1,1},{0,2},{0,3},{9,9},{9,9},{9,9}}, //4
	{{0,0},{1,1},{1,0},{0,2},{1,3},{0,3},{9,9},{9,9}}, //5
	{{0,0},{0,1},{1,0},{1,1},{0,2},{0,3},{1,3},{9,9}}, //6
	{{0,0},{0,1},{1,2},{1,3},{0,3},{9,9},{9,9},{9,9}}, //7
	{{0,0},{0,1},{0,2},{0,3},{1,0},{1,1},{1,2},{1,3}}, //8
	{{1,0},{1,1},{1,2},{0,2},{0,3},{1,3},{9,9},{9,9}}  //9
};

const byte dataTouchPins[] = {4,5,6}; //Touch pins

class Display{
	public:
		Display(byte r);
		
		//Basics
		void setFirstTime(boolean first);
		void reset();
		void clear();
		void clear(boolean show);
		void show();
		void fadeOut();
		void fadeOut(int duration);
		
		//Draw
		void setPixel(byte p, byte red, byte green, byte blue);
		void setPixel(byte p, byte red, byte green, byte blue, float alpha);
		void setRotation(byte rot);
		byte getRotation();
		int getPin(byte x, byte y);
		void setPixel(byte x, byte y, byte red, byte green, byte blue);
		void setPixel(byte x, byte y, byte red, byte green, byte blue, float alpha);
		void setAll(byte red, byte green, byte blue);
		void setAll(byte red, byte green, byte blue, float alpha);
		void drawCircle(byte x0, byte y0, byte r, byte red, byte green, byte blue);
		void drawRectangle(byte x0, byte y0, byte h, byte b, byte red, byte green, byte blue);
		
		//Text
		void drawCharacter(char c, byte red, byte green, byte blue);
		void drawCharacter(char c, byte x0, byte y0, byte red, byte green, byte blue);
		void fillRandom(float p);
		void scrollLeft(String s, int d, byte red, byte green, byte blue);
		
		//Effects
		void rainbow(int d);
		void rainbow();
		void rainbowScroll(int d, byte step1, byte step2);
		void rainbowScroll(int d);
		void rainbowRadial(int d, byte step1, byte step2);
		void stars(int d);
		void fireplace();
		void strobe(unsigned int freq1, unsigned int del, byte red, byte green, byte blue);
		void fireworks();
		
		//Music
		void eq(byte h1,byte h2, byte h3, byte h4, byte h5, byte h6, byte h7, byte red, byte green, byte blue);
		void eq(byte h1,byte h2, byte h3, byte h4, byte h5, byte h6, byte h7);
		void beats(byte h1, byte red, byte green, byte blue);
		
		//Touch
		void enableTouch();
		void disableTouch();
		byte getTouch(byte p);
		byte getTouch(byte x, byte y);
		
		//Snake
		void snake();
		void snakeMove(byte move);
		void snakeShow();
		void snakeEat();
		byte getSnakeDirection();
		boolean findInBody(byte x, byte y, byte start);
		void snakeGameOver();
		void snakeNewFood();
		
		//Pong1
		void pong1();
		void pong1Move();
		void pong1Show();
		void pong1MovePlatform(byte direction);
		void pong1GameOver();
		void pong1MovePlatformTo(byte position);
		
		//Tetris
		void tetris();
		void tetrisMove(byte direction);
		void tetrisShow();
		boolean tetrisCheckCollision();
		void tetrisSet();
		void tetrisClearBlock();
		void tetrisNewBlock();
		boolean tetrisCheckSet();
		void tetrisGameOver();
		void tetrisLines();
		void tetrisRemoveLine(byte line);
		
		//Clock
		void digitalClock(byte hour, byte minute);
		void printClockNumber(byte hour, byte startY, byte red, byte green, byte blue);
		void binaryClock(byte hour, byte minute, byte second);
		void printBinaryClock(byte hour, byte startX, byte red, byte green, byte blue);
		void printTemperature(float temp);
		void loading();
		void wakeUp();
		
		//GOL
		void gameOfLife();
		void gameOfLifeFill();
		
		//Particle System
		void drawPartMatrix(PartMatrix pMatrix);
		
		
	private:
		//Basics
		byte ribbe;
		struct CRGB{unsigned char r; unsigned char g; unsigned char b;};
		byte n;
		int ihue;
		int index;
		boolean firstTime;
		boolean matrix[8][8];
		byte rotation; 
		
		//Timers
		unsigned long timer1;
		unsigned long timer2;
		
		//Touch
		boolean touchEnabled;
		
		//Snake
		int snakeBody[64][2];
		byte snakeBodyLength;
		byte direction;
		byte foodX;
		byte foodY;
		
		//Stroboscope
		boolean state1;
		boolean state2;
		boolean busy;
		byte p1;
		byte p2;
		byte p3;
		int a;
		
		//Pong
		int pongDeltaX;
		int pongDeltaY;
		int pongPosX;
		int pongPosY;
		int pong1Platform;
		
		//Tetris
		byte tetrisActiveBlock;
		int tetrisActiveBlocks[4][2];
		byte tetrisActiveBlockRotation;
		int tetrisActiveBlockPosX;
		int tetrisActiveBlockPosY;
		
		//Fireworks
		byte fireworksState;
		byte fireworksActiveCenterX;
		byte fireworksActiveCenterY;
		byte fireworksColorRed;
		byte fireworksColorGreen;
		byte fireworksColorBlue;
		int fireworksHue;
		int value;
		
		//Game of Life
		byte gameOfLifeI;
		byte gameOfLifeGeneration;
		
		//WakeUp
		byte wakeUpRed; 
		byte wakeUpGreen;
		byte wakeUpBlue;
		byte wakeUpState;
		byte wakeUpStep;		
	public: 
		struct CRGB *leds;
		
};

#endif