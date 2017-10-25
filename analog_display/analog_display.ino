#include <TM1637.h>

#define DIO_PIN 6
#define CLK_PIN 7
#define BRIGHTNESS 3

//----------------------------------------------------------------------------//
void setup()
{
    TM1637::setup(CLK_PIN, DIO_PIN, BRIGHTNESS);
}
//----------------------------------------------------------------------------//
void loop()
{
    int val = long(analogRead(A0)) * 100 / 1024;

    char a, b, c, d;
    a = val / 1000 % 10 + 48;
    b = val / 100 % 10 + 48;
    c = val / 10 % 10 + 48;
    d = val % 10 + 48;
    TM1637::write(a, b, c, d);

    uint32_t start = millis();
    while (millis() - start < 100)
        yield();
}
//----------------------------------------------------------------------------//
