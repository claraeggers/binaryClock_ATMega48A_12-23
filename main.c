#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stdbool.h>
#include <stdio.h>

volatile uint8_t watchdog = 0;
volatile uint8_t pwm = 0;
volatile uint8_t sekunde = 0;
volatile uint8_t minute = 3;
volatile uint8_t stunde = 3;
volatile uint8_t prellS = 0;
volatile uint8_t prellM = 0;
volatile uint8_t prellH = 0;
volatile bool sleepMode = false;
volatile bool countingHour = false;
volatile bool countingMin = false;
uint8_t hourBitShiftDown = 0;
uint8_t hourBitShiftUp = 0;
uint8_t monate[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
uint8_t monate_schalt[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
typedef struct {
    uint8_t tag;
    uint8_t monat;
    bool isSchalt;
    uint16_t jahr;
} Datum;

Datum datum = {22, 3, true, 2024};


//isr core-functionality, isr wird 1x pro sekunde ausgelöst
ISR(TIMER2_OVF_vect){

    sekunde++;
    if(sekunde>59){
    minute++;
    sekunde = 0;
    countingMin = 0;     //bei einem minutenwechsel wird die Zählvariable für Button-Einstellung der Uhrzeit genullt. Beim Drücken des Minute-Buttons wird die Variable auf 1 gesetzt, bei jedem weiteren drücken wird die minute hochgezählt, es gibt keine bestätigung der eingabe, um eine eingabe zu bestätigen hört man auf zu drücken und bei einer vergangenen minute wird die variable wieder auf false gesetzt und die uhr kann von 0 auf gestellt werden
    countingHour = 0;
    }
    if(minute>59){
    minute = 0;
    stunde++; 
    if(stunde>23){
    stunde = 0;
    }
    }

}

//ISR für Sleep Button
ISR(INT0_vect) {
    if (prellS == 0) {
        if (sleepMode == false) {
            SMCR |= (1 << SE) | (1 << SM1) | (1 << SM0); // SMCR sleepmodecontrolregister, SM1&SM0 == Power-Save
            PRR |= (1 << PRSPI) | (1 << PRADC); // extra power reduction throur PRR, adc and spi turnoff in power save mode
            sleepMode = true;
        } else {
            SMCR |= (0 << SE) | (0 << SM1) | (0 << SM0); // SMCR sleepmodecontrolregister, SM1&SM0 == Power-Save
            PRR |= (0 << PRSPI) | (0 << PRADC); // extra power reduction throur PRR, adc and spi turn on again
            sleepMode = false;
        }
    }
    prellS = 255; // Setze Prellzeit auf 255 Taktzyklen entspricht 1 sek
}


//ISr für StundeneinstellungsButton
ISR(INT1_vect){

    if (prellH == 0) {
        if(countingHour == true){
            stunde++;
            if(stunde>=24){
                stunde = 0;
            }
        }
        else{
            countingHour = true;
            stunde = 0;
        }
    }
    prellH = 90; // Setze Prellzeit auf 90 Taktzyklen entspricht 1/3 sek

}

//ISR für Minuteneinstellung
ISR(PCINT0_vect){

    if (prellM == 0) {
        if(countingMin == true){
            minute++;
            if(minute>=60){
                minute = 0;
            }
        }
        else{
            countingMin = true;
            minute = 0;
        }
    }
    prellM = 90; // Setze Prellzeit auf 90 Taktzyklen entspricht 1/3 sek
    
}

//Watchdog ISR
ISR(WDT_vect) {
    cli(); // Interrupts deaktivieren
    wdt_reset();
    //31*2 sek entspricht 1 min 2 sek
    if (watchdog >= 31) {
        watchdog = 0;
    } else {
        watchdog++;
    }
    sei(); // Interrupts aktivieren
}


//PWM CTC
ISR(TIMER2_COMPA_vect) {

    if (pwm % 2 != 0) {
            
        if(sleepMode == false){

            hourBitShiftDown = ( stunde << 5); //Logik von 000xxxxx StundenByte für x = 0 oder 1 sollen die unteren 3 BITs auf PORTD5-7 angezeigt werden, shift um 5
            hourBitShiftUp = ( (stunde >> 2) && 0b00000110); //Logig von 000xxxxx Stundenbyte fpr x = 0 oder 1 sollen bit 3 und bit 4 alleine aif PB1 und PB" stehen, dafür linksshift um 2 und bitmaske fürpb0
            PORTD = (hourBitShiftDown && 0b11101100); //& mit Bitmaske, damit Pull-Up auf PD2&3 auf high bleibt
            PORTB = (hourBitShiftUp && 0b00000111); //& mit Bitmaske, damit Pull-Up auf PB0 auf high bleibt
            PORTC = (minute && 0b00111111);         // display minute and hour uint8_t in led mit bitmaske 
        }
    }
    else {

        PORTD = (0b00001100);
        PORTB = (0b00000001);
        PORTC = (0b00000000);    
    }
    pwm++;
}


void initialisieren(){

    //EXT CLOCK SOURCE
    ASSR |= (1 << AS2);

    //TIMER2 OVF CTC
    TCCR2A |= (1 << WGM21); 
    TCCR2B |= (1 << CS20) | (1 << CS22);     //ps=128, Timer 2, (32,768 kHz = 32768/128*256 = 1 1/s == 1s)
    OCR2A |= 255; //(256-1)
    TIFR2 |= (1<<OCF2A) | (1<<TOV2); //flag register timer interrupt for ocie0b
    TIMSK2 |= (1<<OCIE2A) | (1<<TOIE2) ; //enable compare interrupt, //enable overflow 

    //INTERRUPTS
    EIMSK |= (1<<INT0) | (1<<INT1); // enable interupt0 and interrupt 1
    EICRA |= (1<<ISC11) | (1<<ISC01); // enable external interrupt 0/1 auf fallende flanke
    PCICR |= (1<<PCIE0); // pin change interrupt enable for hourcounter in pcint 0-7

    //WATCHDOG
    MCUSR &= ~(1 << WDRF); // Watchdog-Reset löschen
    wdt_disable();
    WDTCSR |= (1 << WDIE) | (1 << WDE) | (1 << WDP3) | (1 << WDP0); // WDT auf 20 Sekunden einstellen
    wdt_enable(20);
  
    //POWER-REDUCTION
    PRR |= (1<<PRTWI) | (1<<PRUSART0) | (1<<PRTIM0) | (1<<PRTIM1); // Power Reduction Register turns of TWI,timer0/1,usart by initialisation

    //DDR and PULL-UP
    DDRC |= (1 << DDC0) | (1 << DDC1) | (1 << DDC2) | (1 << DDC3) | (1 << DDC4) | (1 << DDC5);  //PC0-5 als led output minLED
    DDRB |= (1 << DDB1) | (1 << DDB2) | (1 << DDB4); //PB4 als XTAL1 output, pb1 und b2 als led output minLED
    DDRD |= (1 << DDD5) | (1 << DDD6) | (1 << DDD7); //pd5-7 als led output hourLED
    PORTD |= (0b00000110); //SleepButton PD3, Stundenbutton PD2 pull-up
    PORTB |= (0b00000001); //MinutenButton PB0, pullup

    //GLOBAL
    sei();  
}

void entprellen(uint8_t prellM, uint8_t prellH, uint8_t prellS) {

    while(prellM > 0) {
        prellM--; // Dekrmentiere Prellvariable
    }
    while(prellH > 0) {
        prellH--; // Dekrementiere Prellvariable
    }
    while(prellS > 0) {
        prellS--; // Dekrementiere Prellvariable
    }
}

uint8_t tage(uint8_t tag, volatile uint8_t *stunde){

    if (*stunde>=24){
        tag++;
        //eeprom write stunde,  read tag, monat, jahr == nicht ändern, ansonsten write tag, monat, jahr, isSchalt
    }
    return tag;
}

void monatjahr(uint8_t tag, Datum *datum) {
    //Funktion für Monate und Jahre
    if (datum->jahr % 4 == 0) {
        datum->isSchalt = true;
        if (datum->tag > monate_schalt[datum->monat]) {
            datum->tag = 1;
            datum->monat++;
            if (datum->monat >= sizeof(monate_schalt)) {
                datum->monat = 0;
                datum->jahr++;
            }
        }
    } else {
        if (tag >= monate[datum->monat]) {
            datum->isSchalt = false;
            datum->tag = 0;
            datum->monat++;
            if (datum->monat >= sizeof(monate)) {
                datum->monat = 0;
                datum->jahr++;
            }
        }
    }
}




void main(){

    initialisieren();

    while(1){
    
        wdt_reset();
        entprellen(prellM, prellH, prellS);
        tage(datum.tag, &stunde);
        monatjahr(datum.tag, &datum);

    }
}
