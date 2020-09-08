/*
  Oscilloscope

  This Oscilloscope was writtern for the ATmega328P.
*/

volatile unsigned short int buffer_pointer = 0;
volatile unsigned short int buffer_index = 0;
volatile unsigned short int buffer_0_full = 0;
volatile unsigned int buffer_0[256] = {};
volatile unsigned short int buffer_1_full = 0;
volatile unsigned int buffer_1[256] = {};
volatile unsigned short int buffer_2_full = 0;
volatile unsigned int buffer_2[256] = {};

void setup() {
  /*
    ADMUX - Analog to Digital Multiplexer Selection Register

    |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
    | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- |
    | REFS1 | REFS0 | ADLAR |       |  MUX3 |  MUX2 |  MUX1 |  MUX0 |

    Bits 7-6: High Voltage Reference Source

    | REFS1 | REFS0 | Source       |
    | ----- | ----- | ------------ |
    |   0   |   0   | AREF         |
    |   0   |   1   | AVCC         |
    |   1   |   1   | Internal 1v1 |

    Bits 5: Left Adjustment vs Right Adjustment

    The reason you might want right adjustment is because right adjustment allows
    for 10 bit presicion whereas left adjustment only allows 8 bit presicion.

    | ADLAR | Adjustment |
    | ----- | ---------- |
    |   0   | Right      |
    |   1   | Left       |

    Bits 3-0: Input Selection

    | MUX3 | MUX2 | MUX1 | MUX0 | Input    |
    | ---- | ---- | ---- | ---- | -------- |
    |    0 |    0 |    0 |    0 | Analog 0 |
    |    0 |    0 |    0 |    1 | Analog 1 |
    |    0 |    0 |    1 |    0 | Analog 2 |
    |    0 |    0 |    1 |    1 | Analog 3 |
    |    0 |    1 |    0 |    0 | Analog 4 |
    |    0 |    1 |    0 |    1 | Analog 5 |
  */
  ADMUX = (0 << REFS1) | (1 << REFS0) | (0 << ADLAR) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);

  /*
    ADCSRA - Analog to Digital Converter Control Status Register A

    |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
    | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- |
    |  ADEN |  ADSC | ADATE |  ADIF |  ADIE | ADPS2 | ADPS1 | ADPS0 |

    Bit 7: Analog to Digital Conversion Enable

    Enable Analog to Digital Conversion

    | ADEN | Enabled |
    | ---- | ------- |
    |    0 | No      |
    |    1 | Yes     |

    Bit 6: Analog to Digital Conversion Start

    Start the Analog to Digital Conversion

    | ADSC | Started |
    | ---- | ------- |
    |    0 | No      |
    |    1 | Yes     |

    Bit 3: Analog to Digital Conversion Interupt Enable

    Enable the ADC interupt.

    | ADIE | Enabled |
    | ---- | ------- |
    |    0 | No      |
    |    1 | Yes     |

    Bits 2-0: Analog to Digital Conversion Prescaler Select

    Determine the division factor between the system clock frequency and the input
    clock to the ADC.

    | ADPS2 | ADPS1 | ADPS0 | Division Factor |
    | ----- | ----- | ----- | --------------- |
    |     0 |     0 |     0 |               2 |
    |     0 |     0 |     1 |               2 |
    |     0 |     1 |     0 |               4 |
    |     0 |     1 |     1 |               8 |
    |     1 |     0 |     0 |              16 |
    |     1 |     0 |     1 |              32 |
    |     1 |     1 |     0 |              64 |
    |     1 |     1 |     1 |             125 |
  */
  ADCSRA = (1 << ADEN) | (1 << ADSC) | (0 << ADATE) | (0 << ADIF) | (1 << ADIE) | (0 << ADPS2) | (0 << ADPS1) | (0 << ADPS0);
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
