#include <avr/interrupt.h>
#include <avr/io.h>
#include <helperfunctions.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

volatile uint8_t watchdog;
volatile uint8_t sekunde;
volatile uint8_t prell =0;
volatile uint8_t minute;
volatile uint8_t stunde;
volatile uint8_t tag;
uint8_t monate[] = { 31,28,31,30,31,30, 31, 31, 30, 31, 30, 31 };
uint8_t monate_schalt[] = { 31,29,31,30,31,30, 31, 31, 30, 31, 30, 31 };
uint8_t jahr = 2024;
uint8_t aktuellermonat = 3;

void main(){

    TCCR2B |=(1<<CS22, 1<<CS21) //ps=256, Timer 2, (32,768 kHz = 32768/128*256 = 1 1/s == 1s)
    OCR2A = 124; // 125-1
    TCCR2A |= (1<<WGM21); //enable CTC // 0 falls overflow
    TIFR2 |= (1<<OCF2A, 1<<TOV2); //flag register timer interrupt for ocie0b
    TIMSK |= (1<<OCIE2A); //enable compare interrupt
    TIMSK |= (1<<TOIE2); //enable overflow 
    EIMSK |= (1<<INT0, 1<<INT1); // enable interupt0
    // Initialisierung des Watchdog-Timers
    MCUSR &= ~(1 << WDRF); // Watchdog-Reset löschen
    WDTCSR |= (1 << WDCE) | (1 << WDE); // WDCE setzen, WDE setzen
    WDTCSR = (1 << WDE) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0); // WDT auf 2 Sekunden einstellen
    // Watchdog-Interrupt aktivieren
    WDTCR |= (1 << WDIE);
    sei();

    while(1){
    
    void entprellen();
    void monate();

}

ISR(TIMER2_COMPA_vect){

    minute++;
    if(minute>=60)minute=0;
    stunde++;    
}

ISR(INT0_vect){
    if (prell == 0) {
        // Aktionen ausführen, wenn der Taster gedrückt wurde und Prellen beendet ist
        // Zum Beispiel: Taster ist gedrückt
    }
    prell = 3; // Setze Prellzeit auf 3 Taktzyklen
}

ISR(INT1_vect){
    if (prell == 0) {
        // Aktionen ausführen, wenn der Taster gedrückt wurde und Prellen beendet ist
        // Zum Beispiel: Taster ist gedrückt
    }
    prell = 3; // Setze Prellzeit auf 3 Taktzyklen entspricht 3sek
}

ISR(WDT_vect) {
    cli(); // Interrupts deaktivieren
    __watchdog_reset();
    if (watchdog >= 30) {
        watchdog = 0;
    } else {
        watchdog++;
    }
    sei(); // Interrupts aktivieren
}

ISR(EE_READY_vect){


}
