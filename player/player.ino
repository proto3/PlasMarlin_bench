// PORTB : D8 to D13
// #define BUT_PIN   (1 << 0)
// #define UP_PIN    (1 << 1)
// #define DOWN_PIN  (1 << 2)
// #define STOP_PIN  (1 << 3)
// #define RESET_PIN (1 << 4)
// #define LED_PIN   (1 << 5)

// PORTC : A0 to A5
// #define A0        (1 << 0)
// #define A1        (1 << 1)
// #define A2        (1 << 2)
// #define A3        (1 << 3)
// #define A4        (1 << 4)
#define CLK_PIN   (1 << 5)

// PORTD : D0 to D7
// #define D0        (1 << 0)
// #define D1        (1 << 1)
// #define OHM_PIN   (1 << 2)
// #define END_Z_PIN (1 << 3)
// #define END_Y_PIN (1 << 4)
// #define END_X_PIN (1 << 5)
// #define TRC_PIN   (1 << 6)
// #define TFR_PIN   (1 << 7)

struct timestep
{
    uint16_t time;
    uint8_t data[2];
};

struct timestep timeline[256];
uint16_t timelength = 0;

//----------------------------------------------------------------------------//
void setup()
{
    Serial.begin(115200);

    //configure all concerned pins as output
    DDRB = 0xFF;
    DDRC = 0xFF;
    DDRD = 0xFF;

    //set all digital outputs to high
    PORTB = 0xFF;
    PORTC = 0xFF;
    PORTD = 0xFF;

    //set clock to low
    PORTC &= ~CLK_PIN;

    cli();

    //disable timer1 interuption
    TIMSK1 &= ~(1 << OCIE1A);

    TCCR1A = 0;// set entire TCCR1A register to 0
    TCCR1B = 0;// same for TCCR1B
    TCNT1  = 0;//initialize counter value to 0
    // set compare match register for 100hz increments
    OCR1A = 624;// ~ (16*10^6) / (100*256) - 1 = 624
    // turn on CTC mode and set CS12 bits for 256 prescaler
    TCCR1B |= (1 << WGM12 | 1 << CS12);

    sei();

    Serial.println("Ready !");
}
//----------------------------------------------------------------------------//
int current_idx = 0;
int current_time = 0;
ISR(TIMER1_COMPA_vect)
{
    if(current_idx == timelength)
        current_idx--;

    if(timeline[current_idx].time == current_time)
    {
        PORTB = timeline[current_idx].data[0];
        PORTD = timeline[current_idx].data[1];
        PORTC |= CLK_PIN;
        current_idx++;
    }
    else
    {
        PORTC |= CLK_PIN;
    }

    delayMicroseconds(10);
    PORTC &= ~CLK_PIN;

    current_time++;
}
//----------------------------------------------------------------------------//
// true if c is in [0-9]
bool is_digit(char c)
{
    return c>=48 && c<=57;
}
//----------------------------------------------------------------------------//
// true if word is non empty and all characters are in [0-9]
bool is_unsigned(char *word)
{
    if(word == NULL)
        return false;

    int i = 0;
    while(word[i] != '\0')
    {
        if(!is_digit(word[i]))
            return false;
        i++;
    }

    return i > 0;
}
//----------------------------------------------------------------------------//
int split(char *line, char** words)
{
    int i = 0;
    char *token = strtok(line, " ");
    while(token != NULL)
    {
        words[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    return i;
}
//----------------------------------------------------------------------------//
void process_line(char *line)
{
    static bool running = false;

    char *words[8] = {NULL};
    int nb_words = split(line, words);

    if(nb_words == 1)
    {
        if(strcmp(words[0], "reset") == 0)
        {
            TIMSK1 &= ~(1 << OCIE1A);
            running = false;
            timelength = 0;
            PORTC &= ~CLK_PIN;
            Serial.println("reset");
            return;
        }
        if(strcmp(words[0], "start") == 0)
        {
            running = true;
            current_idx = 0;
            current_time = 0;
            Serial.println(timelength);
            TCNT1 = 0;
            TIMSK1 |= (1 << OCIE1A);
            return;
        }
    }

    if(!running)
    {
        if(nb_words == 3)
        {
            bool all_int = true;
            for(int i=0;i<3;i++)
            {
                all_int = all_int && is_unsigned(words[i]);
            }
            if(all_int)
            {
                timeline[timelength].time = atoi(words[0]);
                for(int i=0;i<2;i++)
                {
                    timeline[timelength].data[i] = atoi(words[i+1]);
                }
                Serial.println(words[0]);
                timelength++;
            }
        }
    }
}
//----------------------------------------------------------------------------//
int nb_read = 0;
char line[128] = {0};
void loop()
{
    while(Serial.available() > 0 && nb_read < 128)
    {
        line[nb_read] = Serial.read();
        if(line[nb_read] == '\0' || line[nb_read] == '\n')
        {
            line[nb_read] = '\0';
            process_line(line);
            nb_read = 0;
            break;
        }
        nb_read++;
    }

    if(nb_read == 128)
    {
        Serial.println("line should be smaller than 128 characters.");
        nb_read = 0;
    }

    delay(1);
}
//----------------------------------------------------------------------------//
