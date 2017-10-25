// PORTF : A0 to A7
#define END_X_PIN (1 << 0)
#define END_Y_PIN (1 << 1)
#define END_Z_PIN (1 << 2)
#define TFR_PIN   (1 << 3)
#define OHM_PIN   (1 << 4)
#define TRC_PIN   (1 << 5)
#define CLK_PIN   (1 << 6)

// PORTK : A8 to A15
#define BUT_PIN   (1 << 0)
#define UP_PIN    (1 << 1)
#define DOWN_PIN  (1 << 2)
#define STOP_PIN  (1 << 3)

// PORTB : 10 to 13 ; 50 to 53
#define JOY_X_PIN 10
#define JOY_Y_PIN 11
#define THC_PIN   12

struct timestep
{
    uint16_t time;
    uint8_t data[5];
};

struct timestep timeline[512];
uint16_t timelength = 0;

//----------------------------------------------------------------------------//
void setup()
{
    Serial.begin(57600);

    //configure all concerned pins as output
    DDRB = 0xFF;
    DDRF = 0xFF;
    DDRK = 0xFF;

    //set all digital output to high
    PORTF = 0xBF;
    PORTK = 0xFF;

    //set all PWM output to zero
    analogWrite(JOY_X_PIN, 0);
    analogWrite(JOY_Y_PIN, 0);
    analogWrite(THC_PIN,   0);

    cli();

    //disable timer1 interuption
    TIMSK1 &= ~(1 << OCIE1A);

    TCCR1A = 0;// set entire TCCR1A register to 0
    TCCR1B = 0;// same for TCCR1B
    TCNT1  = 0;//initialize counter value to 0
    // set compare match register for 100hz increments
    OCR1A = 624;// = (16*10^6) / (100*256) - 1
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
        PORTF = ~timeline[current_idx].data[0] | CLK_PIN;
        PORTK = ~timeline[current_idx].data[1];
        current_idx++;
    }
    else
    {
        PORTF |= CLK_PIN;
    }

    delayMicroseconds(100);
    PORTF = PORTF & ~CLK_PIN;

    current_time++;
}
//----------------------------------------------------------------------------//
bool is_digit(char c)
{
    return c>=48 && c<=57;
}
//----------------------------------------------------------------------------//
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

    char *words[128] = {NULL};
    int nb_words = split(line, words);

    if(nb_words == 1)
    {
        if(strcmp(words[0], "reset") == 0)
        {
            TIMSK1 &= ~(1 << OCIE1A);
            running = false;
            timelength = 0;
            Serial.println("reset");
            return;
        }
        if(strcmp(words[0], "start") == 0)
        {
            running = true;
            current_idx = 0;
            current_time = 0;
            Serial.println("start");
            TCNT1 = 0;
            TIMSK1 |= (1 << OCIE1A);
            return;
        }
    }

    if(!running)
    {
        if(nb_words == 6)
        {
            bool all_int = true;
            for(int i=0;i<6;i++)
            {
                all_int = all_int && is_unsigned(words[i]);
            }
            if(all_int)
            {
                timeline[timelength].time = atoi(words[0]);
                for(int i=0;i<5;i++)
                    timeline[timelength].data[i] = atoi(words[i+1]);

                Serial.print("add timestep ");
                Serial.println(words[0]);
                timelength++;
            }
        }
    }
}
//----------------------------------------------------------------------------//
int nb_read = 0;
char line[256] = {0};
void loop()
{
    while(Serial.available()>0 && nb_read<256)
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

    if(nb_read == 256)
    {
        Serial.println("line should be smaller than 256 characters.");
        nb_read = 0;
    }

    delay(1);
}
//----------------------------------------------------------------------------//
