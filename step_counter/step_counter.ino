#include <TM1637.h>

#define STEP_PIN 2
#define DIR_PIN  3
#define ENA_PIN   4

#define STEP_OUT_PIN 5

#define DIO_PIN 6
#define CLK_PIN 7
#define BRIGHTNESS 3

int counter = 0;
//----------------------------------------------------------------------------//
void setup()
{
    TM1637::setup(CLK_PIN, DIO_PIN, BRIGHTNESS);

    pinMode(STEP_PIN, INPUT_PULLUP);
    pinMode(DIR_PIN,  INPUT_PULLUP);
    pinMode(ENA_PIN,  INPUT_PULLUP);

    pinMode(STEP_OUT_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(STEP_PIN), step, RISING);
}
//----------------------------------------------------------------------------//
void step()
{
    int ena = digitalRead(ENA_PIN);
    int dir = digitalRead(DIR_PIN);

    if(ena == LOW)
    {
        digitalWrite(STEP_OUT_PIN, HIGH);

        if(dir == HIGH)
            counter++;
        else
            counter--;

        digitalWrite(STEP_OUT_PIN, LOW);
    }
}
//----------------------------------------------------------------------------//
void loop()
{
    int tmp = counter;

    digitalWrite(LED_BUILTIN, (digitalRead(ENA_PIN) == LOW) ? HIGH : LOW);

    char a, b, c, d;
    if(counter < -999)
    {
        a = 'L';
        b = 'E';
        c = 'S';
        d = 'S';
    }
    else if(counter > 9999)
    {
        a = 'M';
        b = 'O';
        c = 'R';
        d = 'E';
    }
    else
    {
        a = counter < 0 ? '-': tmp / 1000 % 10 + 48;
        int absval = abs(tmp);
        b = absval / 100 % 10 + 48;
        c = absval / 10 % 10 + 48;
        d = absval % 10 + 48;
    }

    TM1637::write(a, b, c, d);

    uint32_t start = millis();
    while (millis() - start < 10)
        yield();
}
//----------------------------------------------------------------------------//
