#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <helperfunctions.h>
#include <setup.h>
#include <interrupt_routines.h>
//#include <test.h>

void main(){

    void initialisieren();

    while(1){
    
    wdt_reset();
    void entprellen();
    void tage();
  
}
