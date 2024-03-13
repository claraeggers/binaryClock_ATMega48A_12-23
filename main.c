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
volatile boolean countingHour = 0;
volatile boolean countingMin = 0;

//isr core-functionality, isr wird 1x pro sekunde ausgelöst
ISR(TIMER2_OVF_vect){
    sekunde++
    if(sekunde>=60){
    minute++;
    sekunde=0;
    countingMin = 0;
    countingHour = 0;
    }
    if(minute>=60){
    minute=0;
    stunde++;    
    }
}

ISR(INT0_vect){
    if (prell == 0) {
        // Aktionen ausführen, wenn der Taster gedrückt wurde und Prellen beendet ist
        // Zum Beispiel: Taster ist gedrückt
    }
    prell = 180; // Setze Prellzeit auf 180 Taktzyklen entspricht 2/3 sek
}

ISR(INT1_vect){
    if (prell == 0 && PIN == 0) {
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
    prell = 180; // Setze Prellzeit auf 180 Taktzyklen entspricht 2/3 sek
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
    TCCR2A = (1 << WGM21); 
    TCCR2B = (1 << CS22) | (1 << CS21);     //ps=256, Timer 2, (32,768 kHz = 32768/128*256 = 1 1/s == 1s)
    OCR2A = 124; // 125-1
    TIFR2 |= (1<<OCF2A) | (1<<TOV2); //flag register timer interrupt for ocie0b
    TIMSK |= (1<<OCIE2A); //enable compare interrupt
    TIMSK |= (1<<TOIE2); //enable overflow 
    EIMSK |= (1<<INT0) | (1<<INT1); // enable interupt0
    // Initialisierung des Watchdog-Timers
    MCUSR &= ~(1 << WDRF); // Watchdog-Reset löschen
    wdt_disable();
    wdt_enable(WDT0_20S);
    WDTCSR = (1 << WDIE) | (1 << WDE) | (1 << WDP3) | (1 << WDP0); // WDT auf 20 Sekunden einstellen
    // Watchdog-Interrupt aktivieren
    WDTCR |= (1 << WDIE);
    sei();

    while(1){
    
    wdt_reset();
    void entprellen();
    void tage();

    }
}
