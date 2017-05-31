#include <SPI.h>
#include "lib\SimpleTimer.h"

// pin definitions
#define CLEARPIN 4    // master clear for 74HC595 shift registers
#define LATCHPIN 5    // latch for 74HC595 shift registers
#define OEPIN    6    // output enable for 74HC595 shift registers
#define ARMEDPIN 7    // optoisolator connected to load power
#define DATAPIN  11   // data for 74HC595 shift registers
#define CLOCKPIN 13   // clock for 74HC595 shift registers 

// the timer object
SimpleTimer timer;

char serialIn;
byte r1 = 0, r2 = 0;

// setup
void setup() {
  
  // set all output pins
  SPI.begin(); // handles DATAPIN and CLOCKPIN
  pinMode(LATCHPIN, OUTPUT);
  pinMode(OEPIN, OUTPUT);
  pinMode(CLEARPIN, OUTPUT);

  // make sure no lines go active until data is shifted out
  digitalWrite(CLEARPIN, HIGH);
  digitalWrite(OEPIN, LOW);

  // clear any lines that were left active
  digitalWrite(LATCHPIN, LOW);
  digitalWrite(OEPIN, HIGH);
  SPI.transfer(0);
  SPI.transfer(0);
  digitalWrite(LATCHPIN, HIGH);
  digitalWrite(OEPIN, LOW);
  
  // activate built-in pull-up resistor 
  digitalWrite(ARMEDPIN, HIGH);

  // start the serial communication with the xbee
  Serial.begin(115200);
}


// main loop
void loop() 
{
    if(Serial.available()) 
    {
        c = Serial.read();

        if( (c >= 0) && (c <= 0x0f) ) 
        {
            activateChannel();
        }
        else if(c == '?')
        {
            // load power on = low, off = high
            digitalRead(ARMEDPIN) ? Serial.print("-") : Serial.print("+");
        }
    }

    timer.run();
}

void activateChannel(byte channel)
{
    if(channel > 7) 
    {
      bitSet(r2,channel-8);
    }
    else 
    {
      bitSet(r1,channel);
    }
}

void deactivateChannel(byte channel)
{
    if(c > 7) 
    {
      bitClear(r2,c-8);
    }
    else 
    {
      bitClear(r1,c);
    }
}

void setTimer(byte r, byte channel)
{
    timer.setTimeout(200, deactivateChannel);
}

void transmit()
{
    digitalWrite(LATCHPIN, LOW);
    digitalWrite(OEPIN, HIGH);
    c = SPI.transfer(r1);
    c = SPI.transfer(r2);
    digitalWrite(LATCHPIN, HIGH);
    digitalWrite(OEPIN, LOW);    
}
