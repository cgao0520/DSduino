// Written by Vincent Gao (c_gao)
//BLOG: http://blog.congao.net
//EMAIL: dr.c.gao@gmail.com
// Sep. 2014

#include "pins_arduino.h"
//#include "SPI.h"
#define SS 10                 // PB2
#define MOSI 11               // PB3
#define MISO 12               // PB4
#define SCK 13                // PB5

// what to do with incoming data
byte command = 0;
byte led_pin = 6;
byte led_status = 1;

void setup()
{
  byte clr;
  
  Serial.begin(9600);
  
  // setup SPI interface
  pinMode(SS, INPUT);
  pinMode(MOSI, INPUT);
  pinMode(MISO, OUTPUT);
  pinMode(SCK, INPUT);
  
  // enable SPI interface, CPOL=1, CHPA=1
  SPCR = (1<<6)|(1<<3)|(1<<2);
  // dummy read
  clr = SPSR;
  clr = SPDR;
  
  //attachInterrupt (0, ss_falling, FALLING);
  //SPI.attachInterrupt();
}

byte spi_trans(volatile byte out)
{
  // send and receive a character, blocking
  SPDR = out;
  while (!(SPSR & (1<<7)));
  return SPDR;
}

#define SPI_COMMAND_DWRITE	'~'
#define SPI_COMMAND_AWRITE	'!'
#define SPI_COMMAND_DREAD	'@'
#define SPI_COMMAND_AREAD	'#'
boolean is_recvdata = false;
boolean is_command = false;
boolean wait_command_info = false;
int wait_num_byte = 0;
char buf[4];
int index = 0;
int pin;
int value;

byte do_spi(volatile byte out)
{
  SPDR = out;
  while (!(SPSR & (1<<7)));
  byte d = SPDR;
  //Serial.write(d);

  if(d == '\\') // it is a command
  {
    is_command = true;
    index = 0;
    wait_command_info = false;
    return d;
  }
  
  if(is_command)
  {
    is_command = false;
    buf[index++] = d;
    wait_command_info = true;
    switch(d)
    {
      case SPI_COMMAND_DWRITE:
      case SPI_COMMAND_AWRITE:
        wait_num_byte = 2;
        break;
      case SPI_COMMAND_DREAD:
      case SPI_COMMAND_AREAD:
        wait_num_byte = 1;
        break;
      default:
        wait_num_byte = 0;
        break;
    }
    return d;
  }
  
  if(wait_command_info && (wait_num_byte > 0))
  {
    buf[index++]=d;
    wait_num_byte--;
    //Serial.print(wait_num_byte);
  }
  
  if(wait_command_info && (wait_num_byte == 0))
  {
      //deal with command
      index = 0;
      wait_command_info = false;
      //Serial.println("AAA");
      switch(buf[0])
      {
        case SPI_COMMAND_DWRITE:
          pin = buf[1];
          value = buf[2];
          //buf[3]=0;
          //Serial.println(buf);
          pinMode(pin, OUTPUT);
          digitalWrite(pin, value);
          break;
        case SPI_COMMAND_AWRITE:
          pin = buf[1];
          value = buf[2];
          pinMode(pin, OUTPUT);
          analogWrite(pin, value);
          break;
        case SPI_COMMAND_DREAD:
          pin = buf[1];
          pinMode(pin, INPUT);
          value = digitalRead(pin);
          //SPDR = 0xff & value;
          spi_trans(0xff & value);
          //Serial.println(value);
          break;
        case SPI_COMMAND_AREAD:
          pin = buf[1];
          pinMode(pin, INPUT);
          value = analogRead(pin);
          spi_trans(value>>8);
          spi_trans(0xff & value);
          break;
        default:
          break;
      }
    return d;
  }
    
  return d;
}

byte spi_transfer(volatile byte out)
//ISR (SPI_STC_vect)
{
  static byte sss=LOW;
  
  SPDR = out;
  while (!(SPSR & (1<<7)));
  byte d = SPDR;
  
  Serial.write(d);

  if(d == 'A')
  {
    if(sss==LOW)
    {
      digitalWrite(led_pin, HIGH);
      sss=HIGH;
    }
    else
    {
      digitalWrite(led_pin, LOW);
      sss=LOW;
    }
  }    
  return 1;
}
/*
// SPI interrupt routine
ISR (SPI_STC_vect)
{
  byte c = SPDR;  // grab byte from SPI Data Register
  
}  // end of interrupt routine SPI_STC_vect
*/

void loop (void)
{
  //spi_transfer(0xee);
  do_spi(0xee);
  //Serial.println("A");
  //pinMode(7,INPUT);
  //Serial.println(digitalRead(7));
}  // end of loop

