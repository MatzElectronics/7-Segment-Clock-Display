#include <Wire.h>

#define I2C_BASE_ADDRESS  0x2C

#define PIN_I2C_ADDR0     18  // PA1
#define PIN_I2C_ADDR1     19  // PA2
#define PIN_I2C_ADDR2     20  // PA3

#define PIN_AUX           17  // PC5

#define PIN_SEGA          7   // PB4
#define PIN_SEGB          5   // PB6
#define PIN_SEGC          6   // PB5
#define PIN_SEGD          2   // PA6
#define PIN_SEGE          1   // PA5
#define PIN_SEGF          3   // PA7
#define PIN_SEGG          4   // PB7

#define PIN_DIG1          15  // PC3
#define PIN_DIG2          14  // PC2
#define PIN_DIG3          12  // PC0
#define PIN_DIG4          13  // PC1
#define PIN_DIGP          16  // PC4

#define PIN_PWM           0   // PA4



volatile unsigned char registers[13] = {
  0,          // 0  digit 1 (left-most)
  0,          // 1  digit 2
  0,          // 2  digit 3
  0,          // 3  digit 4 (right-most)
  0b0011000,  // 4  dot map (default center colon on, others off)
  0b0011000,  // 5  blink mask (1 = blinking) (default center colon blinking)
  3,          // 6  blink rate (default ~0.5 seconds)
  0,          // 7  all digits numeric value MSB (signed 16-bit int)
  0,          // 8  all digits numeric value LSB
  0,          // 9  digit 1-2 (left) numeric value (signed 8-bit int)
  0,          // 10 digit 3-4 (right) numeric value (signed 8-bit int)
  0b100,      // 11 leading zeros on numeric values (boolean) (0 = no) (1 = yes).  
              //    First (LSB) bit is a leading zero for 4 digit writes.  (default no)
              //    Second bit is a leading zero for left 2 digit writes.  (default no)
              //    Third bit is a leading zero for right 2 digit writes.  (default yes)
  127,        // 12 brightness - (default ~50%)
};

int segment_pins[7] = {
  PIN_SEGA,
  PIN_SEGB,
  PIN_SEGC,
  PIN_SEGD,
  PIN_SEGE,
  PIN_SEGF,
  PIN_SEGG
};

int digit_pins[5] = {
  PIN_DIG1,
  PIN_DIG2,
  PIN_DIG3,
  PIN_DIG4,
  PIN_DIGP
};

unsigned char current_register = 0;

unsigned char char_map[128] = {
  0b1111110,   // 0 -> 0
  0b0110000,   // 1 -> 1
  0b1101101,   // 2 -> 2
  0b1111001,   // 3 -> 3
  0b0110011,   // 4 -> 4
  0b1011011,   // 5 -> 5
  0b1011111,   // 6 -> 6
  0b1110000,   // 7 -> 7
  0b1111111,   // 8 -> 8
  0b1110011,   // 9 -> 9
  0b1111110,   // 10 -> 0
  0b0110000,   // 11 -> 1
  0b1101101,   // 12 -> 2
  0b1111001,   // 13 -> 3
  0b0110011,   // 14 -> 4
  0b1011011,   // 15 -> 5
  0b1011111,   // 16 -> 6
  0b1110000,   // 17 -> 7
  0b1111111,   // 18 -> 8
  0b1110011,   // 19 -> 9
  0b1111110,   // 20 -> 0
  0b0110000,   // 21 -> 1
  0b1101101,   // 22 -> 2
  0b1111001,   // 23 -> 3
  0b0110011,   // 24 -> 4
  0b1011011,   // 25 -> 5
  0b1011111,   // 26 -> 6
  0b1110000,   // 27 -> 7
  0b1111111,   // 28 -> 8
  0b1110011,   // 29 -> 9
  0b1111110,   // 30 -> 0
  0b0110000,   // 31 -> 1
  0b0000000,   // 32 ->  
  0b0000001,   // 33 -> -
  0b0100000,   // 34 -> "
  0b0000001,   // 35 -> -
  0b0000001,   // 36 -> -
  0b0000001,   // 37 -> -
  0b0000001,   // 38 -> -
  0b0000010,   // 39 -> '
  0b1001110,   // 40 -> C
  0b1111000,   // 41 -> ]
  0b0000001,   // 42 -> -
  0b0000001,   // 43 -> -
  0b0000001,   // 44 -> -
  0b0000001,   // 45 -> -
  0b0000001,   // 46 -> -
  0b0000110,   // 47 -> I
  0b1111110,   // 48 -> 0
  0b0110000,   // 49 -> 1
  0b1101101,   // 50 -> 2
  0b1111001,   // 51 -> 3
  0b0110011,   // 52 -> 4
  0b1011011,   // 53 -> 5
  0b1011111,   // 54 -> 6
  0b1110000,   // 55 -> 7
  0b1111111,   // 56 -> 8
  0b1110011,   // 57 -> 9
  0b0000001,   // 58 -> -
  0b0000001,   // 59 -> -
  0b0001100,   // 60 -> <
  0b0001001,   // 61 -> =
  0b0011000,   // 62 -> >
  0b0000001,   // 63 -> -
  0b0011101,   // 64 -> o
  0b1110111,   // 65 -> A
  0b0011111,   // 66 -> B
  0b1001110,   // 67 -> C
  0b0111101,   // 68 -> D
  0b1001111,   // 69 -> E
  0b1000111,   // 70 -> F
  0b1111011,   // 71 -> G
  0b0010111,   // 72 -> H
  0b0000110,   // 73 -> I
  0b0111100,   // 74 -> J
  0b0110111,   // 75 -> K
  0b0001110,   // 76 -> L
  0b0010101,   // 77 -> M
  0b0010101,   // 78 -> N
  0b0011101,   // 79 -> O
  0b1100111,   // 80 -> P
  0b1110011,   // 81 -> Q
  0b0000101,   // 82 -> R
  0b1011011,   // 83 -> S
  0b0001111,   // 84 -> T
  0b0011100,   // 85 -> U
  0b0111110,   // 86 -> V
  0b0011100,   // 87 -> W
  0b0110111,   // 88 -> X
  0b0110011,   // 89 -> Y
  0b0110000,   // 90 -> Z
  0b1001110,   // 91 -> C
  0b0000110,   // 92 -> I
  0b1111000,   // 93 -> ]
  0b0000001,   // 94 -> -
  0b0001000,   // 95 -> _
  0b0000010,   // 96 -> '
  0b1110111,   // 97 -> A
  0b0011111,   // 98 -> B
  0b1001110,   // 99 -> C
  0b0111101,   // 100 -> D
  0b1001111,   // 101 -> E
  0b1000111,   // 102 -> F
  0b1111011,   // 103 -> G
  0b0010111,   // 104 -> H
  0b0000110,   // 105 -> I
  0b0111100,   // 106 -> J
  0b0110111,   // 107 -> K
  0b0001110,   // 108 -> L
  0b0010101,   // 109 -> M
  0b0010101,   // 110 -> N
  0b0011101,   // 111 -> O
  0b1100111,   // 112 -> P
  0b1110011,   // 113 -> Q
  0b0000101,   // 114 -> R
  0b1011011,   // 115 -> S
  0b0001111,   // 116 -> T
  0b0011100,   // 117 -> U
  0b0111110,   // 118 -> V
  0b0011100,   // 119 -> W
  0b0110111,   // 120 -> X
  0b0110011,   // 121 -> Y
  0b0110000,   // 122 -> Z
  0b1001110,   // 123 -> C
  0b0000110,   // 124 -> I
  0b1111000,   // 125 -> ]
  0b0000001,   // 126 -> -
  0b0000000,   // 127 ->  
};

volatile unsigned char digit_values[4] = {69, 32, 116, 32};

unsigned char dot_map = registers[4];
unsigned char blink_shift = 12 - registers[6];

int get_place_value(int value, int place) {
  int place_powers[] = {0, 1, 10, 100, 1000, 10000};
  return (int)((value / place_powers[place]) % 10);
}

void set_segments(unsigned char c) {
  if (c < 128) {
    digitalWrite(PIN_SEGA, !!(char_map[c] & 0b1000000));
    digitalWrite(PIN_SEGB, !!(char_map[c] & 0b0100000));
    digitalWrite(PIN_SEGC, !!(char_map[c] & 0b0010000));
    digitalWrite(PIN_SEGD, !!(char_map[c] & 0b0001000));
    digitalWrite(PIN_SEGE, !!(char_map[c] & 0b0000100));
    digitalWrite(PIN_SEGF, !!(char_map[c] & 0b0000010));
    digitalWrite(PIN_SEGG, !!(char_map[c] & 0b0000001));
  } else {
    digitalWrite(PIN_SEGA, !!(c & 0b1000000));
    digitalWrite(PIN_SEGB, !!(c & 0b0100000));
    digitalWrite(PIN_SEGC, !!(c & 0b0010000));
    digitalWrite(PIN_SEGD, !!(c & 0b0001000));
    digitalWrite(PIN_SEGE, !!(c & 0b0000100));
    digitalWrite(PIN_SEGF, !!(c & 0b0000010));
    digitalWrite(PIN_SEGG, !!(c & 0b0000001));
  }
};

void set_dots() {
  digitalWrite(PIN_SEGA, !!(dot_map & 0b1000000));
  digitalWrite(PIN_SEGB, !!(dot_map & 0b0100000));
  digitalWrite(PIN_SEGC, !!(dot_map & 0b0010000));
  digitalWrite(PIN_SEGD, !!(dot_map & 0b0001000));
  digitalWrite(PIN_SEGE, !!(dot_map & 0b0000100));
  digitalWrite(PIN_SEGF, !!(dot_map & 0b0000010));
  digitalWrite(PIN_SEGG, !!(dot_map & 0b0000001));
}

void set_dots(unsigned char dots) {
  dot_map = dots;
  set_dots();
}

void set_digit(int d) {
  for (unsigned char i = 0; i < 5; i++) {
    digitalWrite(digit_pins[i], i == d ? HIGH : LOW);
  }
}

void set_digit(int digit, unsigned char value) {
  if (digit == 4) {
    set_dots(value);
  } else {
    set_segments(value);
  }
  set_digit(digit);
  delayMicroseconds(16);
  clear_digits();
  delayMicroseconds(8);
}

void clear_digits() {
  set_digit(6);
}



void setup() {


  pinMode(PIN_I2C_ADDR0, INPUT_PULLUP);
  pinMode(PIN_I2C_ADDR1, INPUT_PULLUP);
  pinMode(PIN_I2C_ADDR2, INPUT_PULLUP);

  unsigned char i2c_address_modifier = (!digitalRead(PIN_I2C_ADDR0) | !digitalRead(PIN_I2C_ADDR1) << 1 | !digitalRead(PIN_I2C_ADDR2) << 2);

  // unsigned char i2c_address_modifier = 0;

  Wire.onReceive(receiveDataWire);                            // give the Wire library the name of the function
  Wire.onRequest(transmitDataWire);                           // same as above, but master read event
  Wire.begin(I2C_BASE_ADDRESS + i2c_address_modifier);        // join i2c bus with specified address


  // put your setup code here, to run once:
  for (unsigned char i = 0; i < 7; i++) {
    pinMode(segment_pins[i], OUTPUT);
  }
  for (unsigned char i = 0; i < 5; i++) {
    pinMode(digit_pins[i], OUTPUT);
  }
  pinMode(PIN_PWM, OUTPUT);

  analogWrite(PIN_PWM, registers[12]);
}

void loop() {
  if (millis() & (1 << blink_shift)) {
    dot_map = registers[4];
  } else {
    dot_map &= (~registers[5]);
  }

  //delayMicroseconds(500);

  // put your main code here, to run repeatedly:
  set_digit(0, registers[0]);
  set_digit(1, registers[1]);
  set_digit(2, registers[2]);
  set_digit(3, registers[3]);
  set_digit(4, dot_map);

}


void receiveDataWire(int16_t num_bytes) {      // the Wire API tells us how many bytes
  current_register = Wire.read();
  num_bytes--;

  if (num_bytes > 13 - current_register) {
    num_bytes = 13 - current_register;
  }
  for (unsigned char i = current_register; i < (num_bytes + current_register); i++) {    // were received so we can for loop for that
    registers[i] = Wire.read();

    if (i == 6) {
      registers[6] = registers[6] % 7;
      blink_shift = 12 - registers[6];

    } else if (i == 7 || i == 8) {

      int16_t num_val = (int16_t) (registers[7] << 8 | registers[8]);
      if (num_val < -999) {
        num_val = -999;
      } else if (num_val > 9999) {
        num_val = 9999;
      }

      unsigned char places = 0;

      if (num_val >= 1000) {
        places = 3;
      } else if (num_val >= 100 || num_val <= -100) {
        places = 2;
      } else if (num_val >= 10 || num_val <= -10) {
        places = 1;
      }

      for (unsigned char i = 4; i > 0; i--) {
        if (i <= places + 1) {
          registers[4 - i] = get_place_value(num_val < 0 ? -num_val : num_val, i);
        } else {
          registers[4 - i] = (registers[11] & 1) ? 0 : 32;   // if leading zeros are enabled, place a zero, otherwise place a ' ' (space)
        }
      }

      if (num_val < 0) {
        registers[(registers[11] & 1) ? 0 : (2 - places)] = 45; // place the negative sign in front of the number
      }

    } else if (i == 9 || i == 10) {
      int8_t num_val = (int8_t) registers[i];

      if (num_val < -9) {
        num_val = -9;
      } else if (num_val > 99) {
        num_val = 99;
      }

      unsigned char digit_reg = i * 2 - 18;

      if (num_val >= 10) {
        registers[digit_reg] = get_place_value(num_val, 2);
        registers[digit_reg + 1] = get_place_value(num_val, 1);

      } else if (num_val >= 0) {
        registers[digit_reg] = (registers[11] & (i == 9 ? 0b10 : 0b100)) ? 0 : 32;
        registers[digit_reg + 1] = num_val;

      } else {
        registers[digit_reg] = 45;
        registers[digit_reg + 1] = -num_val;
      }

    } else if (i == 12) {
      analogWrite(PIN_PWM, registers[12]);
    }
  }
}

void transmitDataWire() {
  unsigned char num_bytes = 12 - current_register;
  for (unsigned char i = current_register; i < (num_bytes + current_register); i++) {
    Wire.write(registers[i]);
  }
}
