#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <helperfunctions.h>
#include <setup.h>

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

    void initialisieren();

    while(1){
    
    wdt_reset();
    void entprellen();
    void tage();
    void displayTime();
  

    }
}
