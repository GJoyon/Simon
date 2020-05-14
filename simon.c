/**
 * a program that runs simon program using atmega328p
 * @author Sua "Joshua" Lee
 * @version 29 Apr 2020
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <math.h>
#include "delay.h"

// maximum possible sequence
#define MAX_SEQ_NUM 100

// external variable declarations
volatile uint8_t overflow = 0;
volatile uint8_t waiting = 0;
volatile uint8_t count = 0;
volatile uint8_t pressed = 0;
volatile uint8_t read = 0;
volatile uint8_t seq_set = 0;
volatile uint8_t idle_count = 0;
volatile uint8_t idle_overflow = 0;
volatile uint8_t pinc = 0x00;

// helper function declaration
uint8_t convert_input(uint8_t input);

void main(void)
{
  // set PD0~3 and PD5~6 as output pins
  DDRD = 0x6f;
  PORTD = 0x00;

  // set all pins (specifically PC2~5) as input pins
  DDRC = 0x00;

  // clear interrupts before initial settings
  cli();

  // sound generation setting
  TCCR0A = (1 << WGM01) + (1 << WGM00) + (1 << COM0A1) + (0 << COM0A0)
    + (1 << COM0B1) + (0 << COM0B0);
  TCCR0B = (1 << WGM02) + (1 << CS02) + (0 << CS01) + (1 << CS00);
  OCR0A = 0;
  OCR0B = 0;

  // stand-by time setting
  TIMSK1 = 1 << TOIE1;
  TCCR1A = 0x00;
  TCCR1B = (1 << CS12) + (0 << CS11) + (1 << CS10);

  // button debounce (using software) setting
  TIMSK2 = 1 << TOIE2;
  TCCR2A = 0x00;
  TCCR2B = (1 << CS22) + (1 << CS21) + (1 << CS20);

  // random sequence generator setting
  PRR &= ~(1 << PRADC);
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  ADCSRB = 0x00;
  ADMUX = (1 << REFS1) | (1 << REFS0) | (0 << ADLAR) |
    (1 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);

  /*
   * pin change interrupt setting for waking up the program
   * from idle mode
   */
  PCICR = (1 << PCIE0);
  PCMSK0 = (1 << PCINT0);

  // initialization finished; set interrupts
  sei();

  // declaration of variables to be used in this code only
  volatile uint8_t seq[MAX_SEQ_NUM];
  volatile uint8_t current_seq = 1;
  volatile uint8_t index;
  volatile uint8_t ans_index = 0;
  volatile uint8_t input;
  volatile uint8_t truncate;
  volatile uint8_t seq_shown = 0;

  while (1)
  {
    // process input
    cli();
    PORTD = pinc;
    if (!pressed) // button not pressed; don't generate any sound
    {
      OCR0A = 0;
      OCR0B = 0;
    }
    else // button pressed
    {
      // generate appropriate sound
      input = convert_input(pinc - 0xf0);
      OCR0A = ((1 + input) * 64) - 1;
      OCR0B = OCR0A / 2;

      // read the input only once per press
      if (!read)
      {
	read = 1;
	if (seq_shown) // check the input only after the sequence is shown
	{
	  if (seq[ans_index] == input) // correct input
	  {
	    ans_index++;
	    if (ans_index == current_seq) // correct sequence
	    {
	      current_seq++;
	      if (current_seq == MAX_SEQ_NUM)
	      {
		current_seq = 1;
	      }
	      
	      ans_index = 0;
	      seq_set = 0;
	      seq_shown = 0;
	    }
	    else if (ans_index > current_seq) // input number over the sequence
	    {
	      ans_index = 0;
	      current_seq = 1;
	      seq_set = 0;
	      seq_shown = 0;
	    }
	  }
	  else // incorrect sequence
	  {
	    ans_index = 0;
	    current_seq = 1;
	    seq_set = 0;
	    seq_shown = 0;
	  }
	}
      }
    }
    sei();

    // set the sequence
    if (!seq_set)
    {
      for (index = 0; index < current_seq; index++)
      {
	ADCSRA |= 1 << ADSC;
	while ((ADCSRA & (1 << ADIF)) == 0);

	ADCSRA &= ~(1 << ADIF);
	seq[index] = ADCL % 4;
	truncate = ADCH;
      }

      seq_set = 1;
    }
    
    // wait for input
    cli();
    while (waiting)
    {
      set_sleep_mode(SLEEP_MODE_PWR_SAVE);
      sleep_enable();
      
      sei();
      sleep_cpu();
      sleep_disable();

      // no input for a long time
      if (idle_overflow > 4)
      {
	DDRC = 0xff;
	PORTC = 0x00;
	
	idle_overflow = 5;
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();

	sei();
	sleep_cpu();
	sleep_disable();

	DDRC = 0x00;
      }
    }
    sei();

    // time to show the sequence
    if (overflow)
    {
      ans_index = 0; // timeout; show the sequence again
      
      for (index = 0; index < current_seq; index++)
      {
	OCR0A = ((1 + seq[index]) * 64) - 1;
	OCR0B = OCR0A / 2;
	PORTD = 1 << seq[index];
	delay1ms(300);

	OCR0A = 0;
	OCR0B = 0;
	PORTD = 0x00;
	delay1ms(100);	
      }
      
      overflow = 0;
      waiting = 1;
      seq_shown = 1;
    }
  }
}

/**
 * convert a given input to an appropriate index
 * for the purpose of generating sound
 * @param input the given input
 * @returns the index used for generating sound
 */
uint8_t convert_input(uint8_t input)
{
  uint8_t result;
  
  if (input < 2)
  {
    result = 0;
  }
  else if (input < 3)
  {
    result = 1;
  }
  else if (input < 5)
  {
    result = 2;
  }
  else
  {
    result = 3;
  }

  return result;
}
