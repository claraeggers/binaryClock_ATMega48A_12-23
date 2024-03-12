#include <avr/interrupt.h>
#include <avr/io.h>
#include <helperfunctions.h>


volatile uint8_t sekunde;
volatile uint8_t prell;

void main(){

    TCCR2B |=(1<<CS22, 1<<CS21) //ps=256, Timer 2, (32,768 kHz = 32768/128*256 = 1 1/s == 1s)
    OCR2A = 124; // 125-1
    TCCR2A |= (1<<WGM21); //enable CTC // 0 falls overflow
    TIFR2 |= (1<<OCF2A, 1<<TOV2); //flag register timer interrupt for ocie0b
    TIMSK |= (1<<OCIE2A); //enable compare interrupt
    TIMSK |= (1<<TOIE2); //enable overflow 
    EIMSK |= (1<<INT0, 1<<INT1); // enable interupt0
    sei();

    while(1){
    if(prell>=0)prell--;


    }



}

ISR(TIMER2_COMPA_vect){

    sekunde++;
    if(sekunde>=60)sekunde=0;    
}

ISR(INT0_vect){
    if(prell==0){
    }
    prell=200;
    //interrupt routine
}

ISR(INT1_vect){
    if(prell==0){
    }

    //interrupt routine
    prell=200;

}

ISR(WDT_vect){

__disable_interrupt();
__watchdog_reset();
WDTCSR|= (1<<WDCE) | (1<<WDE);
WDTCSR|= (1<<WDE) | (1<<WDP2) | (1<<WDP1)| (1<<WDP0); //WDP prescaler 256 2 sec at 5v
__enable_interrupt();

}

ISR(EE_READY_vect){


}
