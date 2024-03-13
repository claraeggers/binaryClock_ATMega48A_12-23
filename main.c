#include <avr/interrupt.h>
#include <avr/io.h>
#include <helperfunctions.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

volatile uint8_t watchdog = 0;
volatile uint8_t prell = 0;
volatile uint8_t sekunde = 0;
volatile uint8_t minute = 0;
volatile uint8_t stunde = 0;
volatile bool countingHour = 0;
volatile bool countingMin = 0;
volatile bool sleepMode = 0;


#define sleepB PORTD2; //int0
#define hButton PORTD3;  //int1
#define mButton PORTB0; //pinchange interrupt 1
#define mLED PORTC;
#define hLED0 PORTB1;
#define hLED1 PORTB2;
#define hLED2 PORTD5;
#define hLED3 PORTD6;
#define hLED4 PORTD7;

//isr core-functionality, isr wird 1x pro sekunde ausgelöst
ISR(TIMER2_OVF_vect){

    sekunde++
    if(sekunde>=60){
    minute++;
    sekunde = 0;
    }
    if(minute>=60){
    minute = 0;
    stunde++; 
    }

}

ISR(INT0_vect){

    if (prell == 0) {
       if(sleepMode == 0){
        SMCR |= (1<<SE) | (1<<SM1) | (1<<SM0); // SMCR sleepmodecontrolregister, SM1&SM0 == Power-Save
        PRR |= (1<<PRSPI) | (1<<PRADC); // extra power reduction throur PRR, adc and spi turnoff in power save mode
        sleepMode = 1;
       }
       else(sleepMode == 1){
        SMCR |= (0<<SE) | (0<<SM1) | (0<<SM0); // SMCR sleepmodecontrolregister, SM1&SM0 == Power-Save
        PRR |= (0<<PRSPI) | (0<<PRADC); // extra power reduction throur PRR, adc and spi turn on again
        sleepMode = 0;
       }
    }
    prell = 255; // Setze Prellzeit auf 255 Taktzyklen entspricht 1 sek

}

ISR(INT1_vect){

    if (prell == 0) {
        if(isCounting == 1){
            hour++;
            if(hour>=24){
                hour = 0;
            }
        }
        else{
            isCounting = 1;
            hour = 0;
        }
    }
    prell = 90; // Setze Prellzeit auf 90 Taktzyklen entspricht 1/3 sek

}


ISR(PCINT0_vect){

    if (prell == 0) {
        if(isCounting == 1){
            minute++;
            if(minute>=60){
                minute = 0;
            }
        }
        else{
            isCounting = 1;
            minute = 0;
        }
    }
    prell = 90; // Setze Prellzeit auf 90 Taktzyklen entspricht 1/3 sek
    
}

ISR(WDT_vect) {
    cli(); // Interrupts deaktivieren
    __watchdog_reset();
    //31*2 sek entspricht 1 min 2 sek
    if (watchdog >= 31) {
        watchdog = 0;
    } else {
        watchdog++;
    }
    sei(); // Interrupts aktivieren
}

ISR(EE_READY_vect){


}


void main(){

    void initialisieren(){

    //EXT CLOCK SOURCE


    //TIMER2 OVF CTC
    TCCR2A = (1 << WGM21); 
    TCCR2B = (1 << CS22) | (1 << CS21);     //ps=256, Timer 2, (32,768 kHz = 32768/128*256 = 1 1/s == 1s)
    OCR2A = 124; // 125-1
    TIFR2 |= (1<<OCF2A) | (1<<TOV2); //flag register timer interrupt for ocie0b
    TIMSK |= (1<<OCIE2A); //enable compare interrupt
    TIMSK |= (1<<TOIE2); //enable overflow 

    //INTERRUPTS
    EIMSK |= (1<<INT0) | (1<<INT1); // enable interupt0 and interrupt 1
    EICRA |= (1<<ISC11) | (1<<ISC01); // enable external interrupt 0/1 auf fallende flanke
    PCICR |= (1<<PCIE0)// pin change interrupt enable for hourcounter in pcint 0-7

    //WATCHDOG
    MCUSR &= ~(1 << WDRF); // Watchdog-Reset löschen
    wdt_disable();
    wdt_enable(WDT0_20S);
    WDTCSR = (1 << WDIE) | (1 << WDE) | (1 << WDP3) | (1 << WDP0); // WDT auf 20 Sekunden einstellen
    WDTCR |= (1 << WDIE);     // Watchdog-Interrupt aktivieren

    //POWER-REDUCTION
    PRR |= (1<<TWI) | (1<<PRUSART0) | (1<<PRTIM0) | (1<<PRTIM1); // Power Reduction Register turns of TWI,timer0/1,usart by initialisation

    //SET-DDR and pull-up
    DDRC = 0b00111111; //PC0-5 als led output hour
    DDRB = 0b00010110; //PB4 as XTAL1 output, pb1 undßb2 als led output min
    DDRD = 0b11100000; //pd5-7 als led output hour
    sleepB = 1;
    hButton = 1;
    mButton = 1



    sei();

    }

    while(1){
    
    wdt_reset();
    void entprellen();
    void tage();
    void displayTime();
  

    }
}
