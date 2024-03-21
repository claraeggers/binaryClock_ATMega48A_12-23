#define F_CPU 1000000UL

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stdbool.h>
#include <stdio.h>
#include <avr/power.h>

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
volatile uint8_t prellE = 0;
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
uint8_t counterstorage0 EEMEM = 0b10101010;
uint8_t counterstorage1 EEMEM = 0b10100000;
volatile uint8_t aktueller_tag = 22;
volatile uint8_t aktueller_monat = 3;
volatile uint8_t pause = 0;


void eeprom_write_byte(uint8_t *address, uint8_t value) {
    eeprom_update_byte(address, value);
}

//DIMMEN und ON/OFF der LED
void pwm_fkt(volatile uint8_t pwm, volatile bool sleep_mode_on){

    if(sleep_mode_on == false){

        if(pwm%5){

         PORTD = 0b00001101;
         PORTB = 0b00000001;  
         PORTC = 0b00000000;
        }
        else{

         PORTC = (minute & 0b00111111);         // display minute and hour uint8_t in led mit bitmaske  
         hourBitShiftDown = ( stunde << 5); //Logik von 000xxxxx StundenByte für x = 0 oder 1 sollen die unteren 3 BITs auf PORTD5-7 angezeigt werden, shift um 5
         hourBitShiftUp = ( (stunde >> 2) & 0b00000110); //Logig von 000xxxxx Stundenbyte fpr x = 0 oder 1 sollen bit 3 und bit 4 alleine aif PB1 und PB" stehen, dafür linksshift um 2 und bitmaske fürpb0
         PORTD = (hourBitShiftDown & 0b11101101); //& mit Bitmaske, damit Pull-Up auf PD2&3 auf high bleibt
         PORTB = (hourBitShiftUp & 0b00000111); //& mit Bitmaske, damit Pull-Up auf PB0 auf high bleibt
         
        }
    }
    else{

     PORTD = 0b00001101;
     PORTB = 0b00000001;  
     PORTC = 0b00000000;
    }
}

//SLEEP-FUNKTION
void schlafen(volatile bool sleep_mode_on){

    if(sleep_mode_on){

        set_sleep_mode(SLEEP_MODE_PWR_SAVE);
        sleep_enable();
        sleep_cpu();
        sleep_disable();
        power_adc_disable();
        power_timer0_disable();
    }

    else{

        sleep_disable();
        power_adc_enable();
        power_timer0_enable();
    }

}

//DATUM SPEICHERN EEPROM
void datum_safe(Datum datum, volatile uint8_t* stunde, volatile uint8_t* minute, volatile uint8_t* sekunde, uint8_t* counterstorage0, uint8_t* counterstorage1){

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
    wdt_enable(WDTO_2S);

    //TIMER0 CTC for PWM
    TCCR0B |= (1 << CS01);   //Prescaler 8 (8000000/(256*8) = 3096,25 Hz
    OCR0A = 20;              //Ausgabevergleichsregister für Timer 0 (pwm)
    TCCR0A |= (1 << WGM00);  //CTC mglw überflüssig
    TIMSK0 |= (1 << OCIE0A); //compare-Match-Interrupt Timer0

    //SET Interrupts
    EIMSK |= (1 << INT0) | (1 << INT1); // enable interupt0 and interrupt 1
    EICRA |= (1 << ISC11) | (1 << ISC01); // enable external interrupt 0/1 auf fallende flanke
    PCICR |= (1 << PCIE0) | (1 << PCIE2); // pin change interrupt enable for hourcounter in pcint 0-7
    PCMSK0 |= (1 << PCINT0); //interrupt pcint0 enable
    PCMSK2 |= (1 << PCINT16); //interrupt pcint20

    //SLEEPMODE disabled on default, but set on power_safe
    SMCR |= (0 << SE) | (1 << SM0) | (1 << SM1);

    //EEPROM
    EEARH |= 1;
    EEARL |= 1;
    EECR |= (1 << EERE) | (1 << EEPE) | (1 << EEMPE);

   //POWER-REDUCTION
    PRR |= (1 << PRTWI) | (1 << PRTIM1); // Power Reduction Register turns of TWI,timer0/1,usart by initialisation

    //SET DDR and PULL-UP
    DDRC = 0b0111111;
    DDRD = 0b11100000;
    DDRB = 0b00000110;
    PORTD = 0b00001101;
    PORTB = 0b00000001;

    //GLOBAL
    sei();  


   while(1){

    while(pause==0){
    pwm_fkt(pwm, sleep_mode_on);
    }
    schlafen(sleep_mode_on);
    datum_safe(datum, &stunde, &minute, &sekunde, &counterstorage0, &counterstorage1); 
    eeprom_update_byte(&counterstorage0, datum.tag);
    eeprom_write_byte(&counterstorage1, datum.monat);
    }
}


//ISR CORE-CLOCK FUNCTIONALITY
ISR(TIMER2_OVF_vect){

    //sekunde++;
   // wdt_reset();
   // if(sekunde==60){
    sekunde=0;
    minute++;
    wdt_reset();
        if(minute==60){
        minute= 0;
        stunde++;
            if(stunde==24){
            stunde = 0;
            }
        }   
   // }  
    //AUSGLEICH : 1,04 sekunden pro sekundenpuls, abweichung von 0,04 1sek/0,04 = 25 entspricht alle 25 sek, sekunde--
    ausgleich++;
    if(ausgleich==24){
    ausgleich = 0;
    sekunde--;
    }

    //ENTPRELLEN SLEEP TASTER
    while(prellS > 0) {
        prellS--; // Dekrementiere Prellvariable
    }
    while(pause > 0) {
        pause--;
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
        while(prellE > 0) {
        prellE--; // Dekrementiere Prellvariable
    }
}

//SLEEP Taster interrupt
ISR(INT0_vect){

    if(prellS==0){

        if(sleep_mode_on==false){

        sleep_mode_on = true;
        prellS = 1;
        }

        else{

        sleep_mode_on = false;
        prellS = 1;
        }
    }
}

//HOUR-BUTTON Einstellen Interrupt
ISR(INT1_vect){
    TIMSK2 |= (0<<TOIE2) ;  //disabel overflow 

    if(prellH==0){

        if(countingH){
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

ISR(PCINT2_vect) {
    // Daten aus dem EEPROM lesen
    aktueller_tag = eeprom_read_byte(&counterstorage0);  // Adresse für Tag
    aktueller_monat = eeprom_read_byte(&counterstorage1); // Adresse für Monat
    
    pause = 1;

    // Aktualisieren der Zeit mit den EEPROM-Daten
    PORTC |= ((0b0111111) & aktueller_tag); 
    hourBitShiftDown = ( aktueller_monat << 5); //Logik von 000xxxxx StundenByte für x = 0 oder 1 sollen die unteren 3 BITs auf PORTD5-7 angezeigt werden, shift um 5
    hourBitShiftUp = ( (aktueller_monat >> 2) & 0b00000110); //Logig von 000xxxxx Stundenbyte fpr x = 0 oder 1 sollen bit 3 und bit 4 alleine aif PB1 und PB" stehen, dafür linksshift um 2 und bitmaske fürpb0
    PORTD = (hourBitShiftDown & 0b11101101); //& mit Bitmaske, damit Pull-Up auf PD2&3 auf high bleibt
    PORTB = (hourBitShiftUp & 0b00000111); //& mit Bitmaske, damit Pull-Up auf PB0 auf high bleibt

    
    // Zurücksetzen des Prell-Timers
    prellE = 220;
    // Aktivierung der Zählung für die Wartezeit von 3 Sekunden
    

}
