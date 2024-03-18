#define F_CPU 1000000UL

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stdbool.h>
#include <stdio.h>
#include <util/delay.h>

volatile uint8_t sekunde = 0;
volatile uint8_t minute = 0;
volatile uint8_t stunde = 0;
volatile uint8_t hourBitShiftDown;
volatile uint8_t hourBitShiftUp;
volatile uint8_t prell = 0;
volatile uint8_t pwm = 0;
volatile bool sleep_mode_on = false;

void pwm_fkt(volatile uint8_t pwm, volatile bool sleep_mod_on){

    if(sleep_mode_on == false){

        if(pwm%5){

         PORTD = 0b00001100;
         PORTB = 0b00000001;  
         PORTC = 0b00000000;
        }
        else{

         PORTC = (minute & 0b00111111);         // display minute and hour uint8_t in led mit bitmaske  
         hourBitShiftDown = ( stunde << 5); //Logik von 000xxxxx StundenByte für x = 0 oder 1 sollen die unteren 3 BITs auf PORTD5-7 angezeigt werden, shift um 5
         hourBitShiftUp = ( (stunde >> 2) & 0b00000110); //Logig von 000xxxxx Stundenbyte fpr x = 0 oder 1 sollen bit 3 und bit 4 alleine aif PB1 und PB" stehen, dafür linksshift um 2 und bitmaske fürpb0
         PORTD = (hourBitShiftDown & 0b11101100); //& mit Bitmaske, damit Pull-Up auf PD2&3 auf high bleibt
         PORTB = (hourBitShiftUp & 0b00000111); //& mit Bitmaske, damit Pull-Up auf PB0 auf high bleibt
        }
    }
    else{

     PORTD = 0b00001100;
     PORTB = 0b00000001;  
     PORTC = 0b00000000;
    }
}


int main (void){
    //minute = 0;

    //EXT CLOCK SOURCE
    ASSR |= (1 << AS2);

    //TIMER2 OVF CTC
    TIMSK2 |= (1<<TOIE2) ; //enable compare interrupt, //enable overflow 
    TCCR2B |= (1 << CS20) | (1 << CS22);     //ps=128, Timer 2, (32,768 kHz = 32768/128*256 = 1 1/s == 1s)
    //WATCHDOG
   // MCUSR &= ~(1 << WDRF); // Watchdog-Reset löschen
    //wdt_disable();
    //WDTCSR |= (1 << WDIE) | (1 << WDE) | (1 << WDP3) | (1 << WDP0); // WDT auf 20 Sekunden einstellen
    //wdt_enable(WDTO_8S);
    TCCR0B |= (1<<CS01);
    OCR0A = 255;
    TCCR0A |= (1<< WGM00);
    TIMSK0 |= (1<<OCIE0A);
  
    //GLOBAL
    sei();  

   DDRC = 0b0111111;
   DDRD = 0b11100000;
   DDRB = 0b00000110;
   PORTD = 0b00001100;
   PORTB = 0b00000001;

   while(1){
    
        wdt_reset();
        pwm_fkt(pwm, sleep_mode_on);


}

   
}



//isr core-functionality, isr wird 1x pro sekunde ausgelöst
ISR(TIMER2_OVF_vect){

    minute++;
    if(minute==60){
    minute= 0;
    stunde++;
    if(stunde==24){
    stunde = 0;
    }
    } 
    
}

//CTC mit Timer0 für PWM
ISR(TIMER0_COMPA_vect){

pwm++;
if(pwm>=200)pwm=0;

}
