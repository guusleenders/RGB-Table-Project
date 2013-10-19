//Basic
#include <FastSPI_LED.h>
#include <avr/pgmspace.h>
#include <Display.h>

//Particle System: https://github.com/giladaya/arduino-particle-sys
#include <ParticleSys.h>
#include <Particle_Std.h>
#include <Particle_Bounce.h>
#include <Particle_Fixed.h>
#include <Emitter_Fountain.h>
#include <Particle_Attractor.h>
#include <PartMatrix.h>
#include <Emitter_Spin.h>

int r=8;
Display display(r);

void setup(){

    Serial1.begin(9600);
    trans.begin(details(data), &Serial1);   
    display.setRotation(1);
    
}

void loop(){
  
    display.rainbow(5);
    
}
