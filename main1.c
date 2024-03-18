#define F_CPU 1000000UL

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stdbool.h>
#include <stdio.h>

//CLOCK
volatile uint8_t sekunde = 0;
volatile uint8_t minute = 0;
volatile uint8_t stunde = 0;
volatile uint8_t ausgleich = 0;
//LED
volatile uint8_t pwm = 0;
volatile uint8_t hourBitShiftDown;
volatile uint8_t hourBitShiftUp;
//SLEEP
volatile bool sleep_mode_on = false;
//BUTTON
volatile uint8_t prellS = 0;
volatile uint8_t prellM = 0;
volatile uint8_t prellH = 0;
volatile bool countingM = false;
volatile bool countingH = false;
//DATE/EEPROM
uint8_t monate[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
uint8_t monate_schalt[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
typedef struct {
    uint8_t tag;
    uint8_t monat;
    uint16_t jahr;
    bool isSchalt;
} Datum;
Datum datum = { 22, 3, 2024, true };
uint8_t counterstorage0 EEMEM = 0b0;
uint8_t counterstorage1 EEMEM = 0b10101010;
uint8_t counterstorage2 EEMEM = 0b00001111;
volatile uint16_t wait = 0;


//DIMMEN und ON/OFF der LED
void pwm_fkt(volatile uint8_t pwm, volatile bool sleep_mode_on){

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

//SLEEP-FUNKTION
void schlafen(volatile bool sleep_mode_on){

    if(sleep_mode_on == true){

    SMCR |= (1 << SE);
    PRR |= (1 << PRADC);

    }
    else{

    SMCR |= (0 << SE);
    PRR |= (0 << PRADC);

    }

}

//DATUM SPEICHERN EEPROM
void datum_safe(Datum datum, volatile uint8_t* stunde, volatile uint8_t* minute, volatile uint8_t* sekunde, volatile uint16_t wait,  uint8_t* counterstorage0, uint8_t* counterstorage1, uint8_t* counterstorage2){

 if(*stunde==0 && *minute==0 && *sekunde<5 && wait==0){

  if(datum.isSchalt){
    if(datum.tag==monate_schalt[datum.monat]){
      datum.tag = 1;
      if(datum.monat<=12){
        datum.monat++;
      }
      else{
        datum.monat = 1;
        datum.jahr++;
        if(datum.jahr % 4 == 0){
            datum.isSchalt = true;
        }
        else{
            datum.isSchalt = false;
        }
      }
    }  
    else{
    datum.tag++;
    }
  }
  else{
     if(datum.tag==monate[datum.monat]){
      datum.tag = 1;
      if(datum.monat<=12){
        datum.monat++;
      }
      else{
        datum.monat = 1;
        datum.jahr++;
        if(datum.jahr % 4 == 0){
            datum.isSchalt = true;
        }
        else{
            datum.isSchalt = false;
        }
      }
    }  
    else{
    datum.tag++;
    }
  }

  eeprom_read_byte (&counterstorage0, &counterstorage1, &counterstorage2);
  eeprom_write_byte(&counterstorage0, datum.tag);
  eeprom_update_byte(&counterstorage1, datum.monat)
  eeprom_update_byte(&counterstorage2, datum.jahr);
  wait = 1275;

 }
}


int main (void){
 
//INITIALISIERUNG

    //EXT CLOCK SOURCE
    ASSR |= (1 << AS2);

    //TIMER2 OVF CTC
    TIMSK2 |= (1 << TOIE2) ;  //enable overflow 
    TCCR2B |= (1 << CS20) | (1 << CS22);     //ps=128, Timer 2, (32,768 kHz = 32768/128*256 = 1 1/s == 1s)

    //WATCHDOG
    MCUSR &= ~(1 << WDRF); // Watchdog-Reset löschen
    wdt_disable();
    WDTCSR |= (1 << WDIE) | (1 << WDE) | (1 << WDP3) | (1 << WDP0); // WDT einstellen
    wdt_enable(WDTO_8S);

    //TIMER0 CTC for PWM
    TCCR0B |= (1 << CS01);
    OCR0A = 255;
    TCCR0A |= (1 << WGM00);
    TIMSK0 |= (1 << OCIE0A);

    //SET Interrupts
    EIMSK |= (1 << INT0) | (1 << INT1); // enable interupt0 and interrupt 1
    EICRA |= (1 << ISC11) | (1 << ISC01); // enable external interrupt 0/1 auf fallende flanke
    PCICR |= (1 << PCIE0); // pin change interrupt enable for hourcounter in pcint 0-7
    PCMSK0 |= (1 << PCINT0); //interrupt pcint0 enable

    //SLEEPMODE disabled on default, but set on power_safe
    SMCR |= (0 << SE) | (1 << SM0) | (1 << SM1);

    //EEPROM
    EEARH |= 1;
    EEARL |= 1;
    EECR |= (1 << EERE) | (1 << EEPE) | (1 << EEMPE);

   //POWER-REDUCTION
    PRR |= (1 << PRTWI) | (1 << PRUSART0) | (1 << PRTIM1); // Power Reduction Register turns of TWI,timer0/1,usart by initialisation

    //SET DDR and PULL-UP
    DDRC = 0b0111111;
    DDRD = 0b11100000;
    DDRB = 0b00000110;
    PORTD = 0b00001100;
    PORTB = 0b00000001;

    //GLOBAL
    sei();  


   while(1){

    wdt_reset();
    pwm_fkt(pwm, sleep_mode_on);
    schlafen(sleep_mode_on);
    datum_safe(datum, &stunde, &minute, &sekunde, wait, &counterstorage0, &counterstorage1, &counterstorage2); 

    }
}


//ISR CORE-CLOCK FUNCTIONALITY
ISR(TIMER2_OVF_vect){

    sekunde++;
    if(sekunde==60){
    sekunde=0;
    minute++;
        if(minute==60){
        minute= 0;
        stunde++;
            if(stunde==24){
            stunde = 0;
            }
        }   
    }  
    //AUSGLEICH : 1,04 sekunden pro sekundenpuls, abweichung von 0,04 1sek/0,04 = 25 entspricht alle 25 sek, sekunde--
    ausgleich++;
    if(ausgleich==24){
    ausgleich = 0;
    sekunde--;
    }

    //DEKREMENT WAITER FOR EEPROM WRITE
    if(wait>0){
    wait--;
    }
}

//CTC TIMER0 für PWM und zählvariablen-dekrementierung
ISR(TIMER0_COMPA_vect){

    pwm++;
    if(pwm>=200)pwm=0;

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

//SLEEP Taster interrupt
ISR(INT0_vect){

    if(prellS==0){

        if(sleep_mode_on==false){

        sleep_mode_on = true;
        prellS = 220;
        }

        else{

        sleep_mode_on = false;
        prellS = 220;
        }
    }


}

//HOUR-BUTTON Einstellen Interrupt
ISR(INT1_vect){
    TIMSK2 |= (0<<TOIE2) ;  //disabel overflow 

    if(prellH==0){

        if(countingH == true){
            stunde++;
            if(stunde>=24){
                stunde = 0;
            }
            prellH=220;

        }
        else{
            countingH = true;
            stunde = 0;
            prellH = 220;
        }
    }   
    TIMSK2 |= (1<<TOIE2) ;  //enable overflow 
 
}

//MINUTE-BUTTON-Einstellen Interrupt (pinchange interrupt)
ISR(PCINT0_vect){

    if(prellM==0){

        if(countingM == true){
                TIMSK2 |= (0<<TOIE2) ;  //disabel overflow 

            minute++;
            if(minute>=60){
                minute = 0;
            }
            prellM=220;
            TIMSK2 |= (1<<TOIE2) ;  //enable overflow 

        }
        else{
            countingM = true;
            minute = 0;
            prellM = 220;
        }
    }   


}

