// Rainbow Spectrum Analyzer code
// last edited 11/14/2017
// Joey Conenna

// AVR @ internal 8MHz
// avrdude -p atmega2560 -c usbtiny -U lfuse:w:0xc2:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m
// AVR @ external 16MHz
// avrdude -p atmega2560 -c usbtiny -U lfuse:w:0xff:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define R PORTB
#define O PORTK
#define Y PORTA
#define G PORTJ
#define B PORTC
#define V PORTD
#define PWMROY PORTE
#define PWMGBV PORTH
#define RST 2
#define STR 1

// frequency intensity values
int16_t level[7] = {0, 0, 0, 0, 0, 0, 0};

// storage for 4 time averaged frequency intensities per band
uint16_t filter_levels [7][4];

// potentiometer value
uint16_t pot_val = 0;

// scaled boundary values used to adjust output sensitivity
uint16_t scale_vals [5];

// function prototypes
void reset_eq(void);
void read_eq(void);
void pot_scale(void);

int main(){

  // set data direction registers for all ports
  DDRB = 0xFF; 
  DDRK = 0xFF; 
  DDRA = 0xFF; 
  DDRJ = 0xFF; 
  DDRC = 0xFF; 
  DDRD = 0xFF; 
  DDRE = 0xFF; 
  DDRH = 0xFF;
  DDRF = 0b00000110; 
 
  // turn ON pwm mosfets
  PWMROY = 0xFF;
  PWMGBV = 0xFF;

  
  // pwm green, blue, and violet, to visually match overall intensity levels of warmer colors
  TCCR4A |= (1 << COM4A1) | (1 << COM4B1) | (1 << COM4C1) | (1 << WGM42) | (1 << WGM40);
  TCCR4B |= (1 << CS41);
  OCR4A = 0x0096;
  OCR4B = 0x0096;
  OCR4C = 0x00C8;
  
  // set up ADC for sampling pot and IC output
  ADCSRA |= (1 << ADPS1); // Set ADC prescalar to 128 - 125KHz sample rate @ 16MHz
  ADMUX  |= (1 << REFS0); // Set ADC reference to AVCC
  ADCSRA |= (1 << ADEN);  // Enable ADC
  ADCSRA |= (1 << ADSC);  // Start A2D Conversions
   
   
  // reset MSGEQ7
  _delay_us(10);
  PORTF &= ~(1 << RST);
  PORTF |= (1 << STR);
   
  // initialize all filter levels to 0
  for(uint8_t i = 0; i < 7; i++)
	  for(uint8_t j = 0; j < 4; j++)
		 filter_levels[i][j] = 0;

 while(1)
 {  

  reset_eq();  // reset IC
  read_eq();   // sample and read in IC outputs for all bands
  pot_scale(); // scale output level boundaries
  
  // reset level values
  for(uint8_t i = 0; i < 7; i++)
	  level[i] = 0;
  
  // Set output led levels for:
  // Red
  for(uint8_t i = 0; i < 4; i++)
	  level[0] += filter_levels[0][i];
  
  level[0] = level[0] >> 2;
  
  if(level[0] > 50 && level[0] <= scale_vals[0])
	  R = 0b00000001;
  else if(level[0] > scale_vals[0] && level[0] <= scale_vals[1])
	  R = 0b00000011;
  else if(level[0] > scale_vals[1] && level[0] <= scale_vals[2])
	  R = 0b00000111;
  else if(level[0] > scale_vals[2] && level[0] <= scale_vals[3])
	  R = 0b00001111;
  else if(level[0] > scale_vals[3] && level[0] <= scale_vals[4])
	  R = 0b00011111;
  else if(level[0] > scale_vals[4])
	  R = 0b00111111;
  else
	  R = 0b00000000;
  
  
  // Orange
  for(uint8_t i = 0; i < 4; i++)
	  level[1] += filter_levels[1][i];
  
  level[1] = level[1] >> 2;
  
  if(level[1] > 100 && level[1] <= scale_vals[0])
	  O = 0b00000001;
  else if(level[1] > scale_vals[0] && level[1] <= scale_vals[1])
	  O = 0b00000011;
  else if(level[1] > scale_vals[1] && level[1] <= scale_vals[2])
	  O = 0b00000111;
  else if(level[1] > scale_vals[2] && level[1] <= scale_vals[3])
	  O = 0b00001111;
  else if(level[1] > scale_vals[3] && level[1] <= scale_vals[4])
	  O = 0b00011111;
  else if(level[1] > scale_vals[4])
	  O = 0b00111111;
  else
	  O = 0b00000000;

  
  // Yellow
  for(uint8_t i = 0; i < 4; i++)
	  level[2] += filter_levels[2][i];
  
  level[2] = level[2] >> 2;
  
  if(level[2] > 100 && level[2] <= scale_vals[0])
	  Y = 0b00000001;
  else if(level[2] > scale_vals[0] && level[2] <= scale_vals[1])
	  Y = 0b00000011;
  else if(level[2] > scale_vals[1] && level[2] <= scale_vals[2])
	  Y = 0b00000111;
  else if(level[2] > scale_vals[2] && level[2] <= scale_vals[3])
	  Y = 0b00001111;
  else if(level[2] > scale_vals[3] && level[2] <= scale_vals[4])
	  Y = 0b00011111;
  else if(level[2] > scale_vals[4])
	  Y = 0b00111111;
  else
	  Y = 0b00000000;


  // Green
  for(uint8_t i = 0; i < 4; i++)
	  level[3] += filter_levels[3][i];
  
  level[3] = level[3] >> 2;
  
  if(level[3] > 100 && level[3] <= scale_vals[0])
	  G = 0b00100000;
  else if(level[3] > scale_vals[0] && level[3] <= scale_vals[1])
	  G = 0b00110000;
  else if(level[3] > scale_vals[1] && level[3] <= scale_vals[2])
	  G = 0b00111000;
  else if(level[3] > scale_vals[2] && level[3] <= scale_vals[3])
	  G = 0b00111100;
  else if(level[3] > scale_vals[3] && level[3] <= scale_vals[4])
	  G = 0b00111110;
  else if(level[3] > scale_vals[4])
	  G = 0b00111111;
  else
	  G = 0b00000000;
	

  // Blue
  for(uint8_t i = 0; i < 4; i++)
	  level[4] += filter_levels[4][i];
  
  level[4] = level[4] >> 2;
  
  if(level[4] > 100 && level[4] <= scale_vals[0])
	  B = 0b00100000;
  else if(level[4] > scale_vals[0] && level[4] <= scale_vals[1])
	  B = 0b00110000;
  else if(level[4] > scale_vals[1] && level[4] <= scale_vals[2])
	  B = 0b00111000;
  else if(level[4] > scale_vals[2] && level[4] <= scale_vals[3])
	  B = 0b00111100;
  else if(level[4] > scale_vals[3] && level[4] <= scale_vals[4])
	  B = 0b00111110;
  else if(level[4] > scale_vals[4])
	  B = 0b00111111;
  else
	  B = 0b00000000;
	
	
  // Violet
  for(uint8_t i = 0; i < 4; i++)
	  level[5] += filter_levels[5][i];
  
  level[5] = level[5] >> 2;
  
  if(level[5] > 100 && level[5] <= scale_vals[0])
	  V = 0b00000001;
  else if(level[5] > scale_vals[0] && level[5] <= scale_vals[1])
	  V = 0b00000011;
  else if(level[5] > scale_vals[1] && level[5] <= scale_vals[2])
	  V = 0b00000111;
  else if(level[5] > scale_vals[2] && level[5] <= scale_vals[3])
	  V = 0b00001111;
  else if(level[5] > scale_vals[3] && level[5] <= scale_vals[4])
	  V = 0b00011111;
  else if(level[5] > scale_vals[4])
	  V = 0b00111111;
  else
	  V = 0b00000000;
	

  }
}
	
void reset_eq(void)
{

  PORTF |= (1 << RST);
  _delay_us(10);
  PORTF &= ~(1 << RST);
  _delay_us(80);
  
}

void read_eq(void){
for (uint8_t i = 0; i < 7; i++)
 {
    
	// trigger next value
	PORTF &= ~(1 << STR);
	_delay_us(40);
 
	// start ADC conversion
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
    while(ADCSRA & (1<<ADSC));

    // add level to filter
	for(uint8_t j = 0; j < 3; j++)
		filter_levels[i][j] = filter_levels[i][j+1];
	
    filter_levels[i][3] = ADC >> 2;
	
	PORTF |= (1 << STR);
	_delay_us(40);
 }
}

void pot_scale(void)
{
  // read pot value for output scaling
  
  // set ADCMUX to pot pin
  ADMUX |= (1 << MUX2) | (1 << MUX1);
  
  ADCSRA |= (1<<ADSC);
  
  // wait for conversion to complete
  while(ADCSRA & (1<<ADSC));
  
  pot_val = ADC;
  
  // set ADCMUX back to IC output pin
  ADMUX &= ~(1 << MUX2);
  ADMUX &= ~(1 << MUX1);
  
  // use pot_val to set output level boundaries
  pot_val = pot_val >> 3;
  
  scale_vals[0] = 100 + 10 + pot_val;
  scale_vals[1] = 100 + 20 + pot_val;
  scale_vals[2] = 100 + 30 + pot_val;
  scale_vals[3] = 100 + 40 + pot_val;
  scale_vals[4] = 100 + 50 + pot_val;
}
	