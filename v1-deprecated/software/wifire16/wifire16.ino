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
const unsigned int SIGNAL_HOLD = 500;

byte serialData = 0;
byte r1 = 0, r2 = 0;
unsigned int timers[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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
        serialData = Serial.read();

        if( (serialData >= 0) && (serialData <= 0x0f) ) 
        {
            activateChannel(serialData);
        }
        else if(serialData == '?')
        {
            // load power on = low, off = high
            digitalRead(ARMEDPIN) ? Serial.print("-") : Serial.print("+");
        }
    }

    timer.run();
}

bool isChannelActive(const byte channel)
{
	if (channel > 7)
	{
		return bitRead(r2, channel - 8) > 0;
	}
	else
	{
		return bitRead(r1, channel) > 0;
	}
}

void activateChannel(const byte channel)
{
	if (!isChannelActive(channel))
	{
		if (channel > 7)
		{
			bitSet(r2, channel - 8);
		}
		else
		{
			bitSet(r1, channel);
		}

		transmit();
		setTimer(channel);
	}
	else
	{
		timer.restartTimer(timers[channel]);
	}
}

void deactivateChannel()
{
	for (int channel = 0; channel <= 15; ++channel)
	{
		unsigned int timerId = timers[channel];
		if (isChannelActive(channel) && !timer.isEnabled(timerId))
		{
			if (channel > 7)
			{
				bitClear(r2, channel - 8);
			}
			else
			{
				bitClear(r1, channel);
			}

			transmit();
		}
	}
}

void setTimer(byte channel)
{
    int timerId = timer.setTimeout(SIGNAL_HOLD, deactivateChannel);
	timers[channel] = timerId;
}

void transmit()
{
    digitalWrite(LATCHPIN, LOW);
    digitalWrite(OEPIN, HIGH);
    SPI.transfer(r1);
    SPI.transfer(r2);
    digitalWrite(LATCHPIN, HIGH);
    digitalWrite(OEPIN, LOW);    
}
