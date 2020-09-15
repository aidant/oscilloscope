/*
  Oscilloscope

  This Oscilloscope was writtern for the ATmega328P.
*/

volatile unsigned short int buffer_pointer = 0;
volatile unsigned short int buffer_index   = 0;
volatile unsigned short int buffer_0_full  = 0;
volatile unsigned int       buffer_0[256]  = {};
volatile unsigned short int buffer_1_full  = 0;
volatile unsigned int       buffer_1[256]  = {};
volatile unsigned short int buffer_2_full  = 0;
volatile unsigned int       buffer_2[256]  = {};

void setup() {
  // Configure the ADC input
  ADMUX =   (0b01 << REFS1);    // Rreference voltage = AVcc
  ADMUX |=  (0b0000 << MUX3);   // Select ADC0. Analog pin 0 on the board

  // More setup
  ADCSRA = (1 << ADEN);         // Turn on the ADC
  ADCSRA |= (1 << ADIE);        // Enable interrupting, with I of SREG.
  ADCSRA |= (0b000 << ADPS2);   // Division factor of 2
}

void loop() {

}

/*
  ISR - Interrupt Service Routine

  The Interrupt Service Routine is a handler which is executed on an event, in
  this case the Interrupt Service Routine handler is executed once the Analog to
  Digital Conversion (ADC) has completed; this is because the Analog to Digital
  Conversion vector (ADC_vect) is specified bellow.
*/
ISR(ADC_vect) {
  /*
    Analog to Digital Conversion

    The Analog to Digital Conversion has upto 10 bits of accuracy whilst the
    microcontroller operates on 8 bits (1 byte). A 10 bit reading is possible by
    splitting the reading into two sections, a high containing the first 2 bits
    and a low containing the remaining 8 bits - these two can later be joined to
    create a 10 bit number. The low reading is avaliable on a register by the
    name ADCL (Analog to Digital Conversion Low) whist the high reading is
    avaliable by the name ADCH (Analog to Digital Conversion High). It is
    crucial that ADCL is accessed before ADCH to achieve 10 bit precision, this
    is because once the ADCL register is accessed the Analog to Digital
    Conversion will not be updated until ADCH has been accessed.
  */

  /*
    Analog to Digital Conversion Low
  */
  unsigned short int adcl = ADCL;
  /*
    Analog to Digital Conversion High
  */
  unsigned short int adch = ADCH;
  /*
    Analog to Digital Conversion (10 bit precision)

    ADCH      ADCL
    0000 0011 1111 1111
  */
  unsigned int adc = (adch << 8) | adcl;

  /*
    Update our buffer_index and buffer_pointer to point at the correct position
    in the correct buffer.
  */
  buffer_index += 1;
  if (buffer_index == 0) {
    buffer_pointer = (buffer_pointer + 1) % 3;
  }

  /*
    Insert our reading in our buffer and mark it as full if necessary.
  */
  switch (buffer_pointer) {
    case 0:
      buffer_0[buffer_index] = adc;
      if (buffer_index == 255) buffer_0_full = 1;
      break;
    case 1:
      buffer_1[buffer_index] = adc;
      if (buffer_index == 255) buffer_1_full = 1;
      break;
    case 2:
      buffer_2[buffer_index] = adc;
      if (buffer_index == 255) buffer_2_full = 1;
      break;
  }

  /*
    Start the Analog to Digital Conversion
  */
  ADCSRA = ADCSRA | (1 << ADSC);
}
