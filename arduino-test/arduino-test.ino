#include <SPI.h>

#include "mx7219.h"
#include "pins.h"

byte spi_communicate(byte mxreg, byte mxval, byte key_out = 0) {
    byte key_state;

    SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    digitalWrite(DPYSS_BAR, LOW);
    key_state = SPI.transfer(0);
    SPI.transfer(key_out);
    SPI.transfer(mxreg);
    SPI.transfer(mxval);
    digitalWrite(DPYSS_BAR, HIGH);
    SPI.endTransaction();
    return key_state;
}

void setup() {
    pinMode(PHI2, INPUT);
    pinMode(RW_BAR, INPUT);
    pinMode(RST_BAR, INPUT);
    pinMode(BE, INPUT);

    digitalWrite(HALT, LOW);
    pinMode(HALT, OUTPUT);

    digitalWrite(STEP, LOW);
    pinMode(STEP, OUTPUT);

    digitalWrite(ADLOAD, LOW);
    pinMode(ADLOAD, OUTPUT);

    digitalWrite(DOE_BAR, HIGH);
    digitalWrite(AOE_BAR, HIGH);
    pinMode(DOE_BAR, OUTPUT);
    pinMode(AOE_BAR, OUTPUT);

    digitalWrite(DPYSS_BAR, HIGH);
    pinMode(DPYSS_BAR, OUTPUT);

    digitalWrite(BUSS_BAR, HIGH);
    pinMode(BUSS_BAR, OUTPUT);

    digitalWrite(SCK, LOW);
    digitalWrite(MOSI, LOW);
    pinMode(MISO, INPUT);
    pinMode(MOSI, OUTPUT);
    pinMode(SCK, OUTPUT);

    for(int digit=0; digit<8; ++digit) {
        spi_communicate(MX7219_DIGIT_0 + digit, 0);
    }
    spi_communicate(MX7219_DECODE_MODE, 0x00); // No decode
    spi_communicate(MX7219_INTENSITY, 0xFF);   // Maximum intensity
    spi_communicate(MX7219_SCN_LIMIT, 0x05);   // Scan digits 0-5
    spi_communicate(MX7219_DPLY_TEST, 0x00);   // Display test on
    spi_communicate(MX7219_SHUTDOWN, 0x01);    // Normal operation
}

unsigned long addr = 1;
unsigned long chase = 1;
byte state;

#define TICK_DELAY 30
unsigned long next_tick = millis() + TICK_DELAY;

#define HALT_MASK (1<<6)

uint_least32_t blinken_lights_a = 0, blinken_lights_b = 0;;

void loop() {
    byte a0_7, a8_15, a16_24, ctrl;

    spi_communicate(MX7219_NOP, 0, 0x1);
    a0_7 = spi_communicate(MX7219_NOP, 0);
    spi_communicate(MX7219_NOP, 0, 0x2);
    a8_15 = spi_communicate(MX7219_NOP, 0);
    spi_communicate(MX7219_NOP, 0, 0x4);
    a16_24 = spi_communicate(MX7219_NOP, 0);
    spi_communicate(MX7219_NOP, 0, 0x8);
    ctrl = spi_communicate(MX7219_NOP, 0);

    if(ctrl & HALT_MASK) {
        spi_communicate(MX7219_DIGIT_0 + 0, ctrl);
        spi_communicate(MX7219_DIGIT_0 + 1, a0_7);
        spi_communicate(MX7219_DIGIT_0 + 2, a8_15);
        spi_communicate(MX7219_DIGIT_0 + 3, a16_24);
        spi_communicate(MX7219_DIGIT_0 + 4, chase & 0xff);
        spi_communicate(MX7219_DIGIT_0 + 5, (chase>>8) & 0xff);
    } else {
        for(int i=0; i<4; ++i) {
            spi_communicate(
                MX7219_DIGIT_0 + i,
                (blinken_lights_a >> (i<<3)) & 0xff
            );
            spi_communicate(
                MX7219_DIGIT_0 + 4 + i,
                (blinken_lights_b >> (i<<3)) & 0xff
            );
        }
    }

    unsigned long now = millis();
    if(now > next_tick) {
        next_tick = now + TICK_DELAY;

        addr<<=1;
        if(addr >= 1UL<<24) {
            addr = 1;
        }

        chase<<=1;
        if(chase >= 1UL<<16) {
            chase = 1;
        }

        blinken_lights_a ^= 1UL << random(32);
        blinken_lights_b ^= 1UL << random(32);
    }
}

