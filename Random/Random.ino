//Random MiniFrank
//random modul for MiniFrank, outputs 2 different random note Values, between -5V - +5V.
//Speed depends on the potentiometer, clock signal input

#include <SPI.h>
#include <Wire.h>

#define POT A1      //Potentiometer pin
#define CLK_IN 0   // Clock interrupt

//SPI
#define DAC_1  A2   //CS DAC_1
#define MOSI  4
#define SCLK  6

long timer = 0;
bool trigger = 0;

void setup() {
  SPI.begin();  //Initialize SPI

  //Setup Pins
  pinMode(POT, INPUT);
  pinMode(DAC_1, OUTPUT);

  //new random seed from current potentiometer value
  randomSeed(analogRead(POT));

  //Clock interrupt
  attachInterrupt(CLK_IN, clkInterrupt, FALLING);
}

void loop() {

  long potVal = analogRead(POT) ;     //read potentiometer value
  long wait =  potVal * potVal >> 9;  //non linear value from potentiometer value

  if (wait < 1950) {   //disable auto trigger, if potentiometer value is under 1950
    if (millis() - timer > wait) trigger = 1; //test timer reached
  }

  //send new random Signal
  if (trigger) {
    timer = millis();

    setVoltage(DAC_1, 0, random(4096)); //random output 1
    setVoltage(DAC_1, 1, random(4096)); //random output 2
    trigger = 0; //reset trigger
  }
}

//Send voltage value to MCP4822   //from https://github.com/elkayem/midi2cv

void setVoltage(uint8_t dacpin, bool channel, unsigned int mV)  //select dac, select dac channel, mv Value
{
   //16bit register
  // |channel|-|gain(0)|shutdown(1)|....12bit Value....|

  //build command
  unsigned int command = channel ? 0x9000 : 0x1000;  //select channel 0 or 1 and activate DAC
  command |= (mV & 0x0FFF);                          //set 12bit value

  SPI.beginTransaction(SPISettings(24000000, MSBFIRST, SPI_MODE0));
  digitalWrite(dacpin, LOW);    //activate transfer
  SPI.transfer(command >> 8);   //send first byte..
  SPI.transfer(command & 0xFF); //.. and the second
  digitalWrite(dacpin, HIGH);   //disable transfer
  SPI.endTransaction();         //end SPI transaction
}

//clock signal
void clkInterrupt() {
  trigger = 1;
}
