void entprellen() {
    if (prellM; > 0) {
        prellM--; // Dekrmentiere Prellvariable
    }
    if (prellH > 0) {
        prellH--; // Dekrementiere Prellvariable
    }
    if (prellS > 0) {
        prellS--; // Dekrementiere Prellvariable
    }
}

void tage(){
        if (stunde>=24){
            tag++;
            stunde=0;
            void monatjahr();
            //eeprom write stunde,  read tag, aktuellermonat, jahr == nicht ändern, ansonsten write tag, aktuellermonat, jahr, isSchalt
        }
}

void monatjahr(){
    
        //Funktion für Monate und Jahre
        if (jahr % 4 == 0) {
            isSchalt = 1;
            if (tag > monate_schalt[aktuellermonat]) {
            tag = 1;
            aktuellermonat++;
            if (aktuellermonat > sizeof(monate_schalt)) {
                aktuellermonat = 1;
                jahr++;
            }
        }
        } else {
            if (tag >= monate[aktuellermonat]) {
            isSchalt = 0;
            tag = 0;
            aktuellermonat++;
            if (aktuellermonat >= sizeof(monate)) {
                aktuellermonat = 0;
                jahr++;
            }
            }
        }
}

void displayTime(){

    if(sleepMode == 0){

        hourBitShiftDown = ( stunde << 5); //Logik von 000xxxxx StundenByte für x = 0 oder 1 sollen die unteren 3 BITs auf PORTD5-7 angezeigt werden, shift um 5
        hourBitShiftUp = ( (stunde >> 2) && 0b00000110); //Logig von 000xxxxx Stundenbyte fpr x = 0 oder 1 sollen bit 3 und bit 4 alleine aif PB1 und PB" stehen, dafür linksshift um 2 und bitmaske fürpb0
        PORTD |= (hourBitShiftDown && 0b11101100); //& mit Bitmaske, damit Pull-Up auf PD2&3 auf high bleibt
        PORTB |= (hourBitShiftUp && 0b00000111); //& mit Bitmaske, damit Pull-Up auf PB0 auf high bleibt
        PORTC |= (minute && 0b00111111);         // display minute and hour uint8_t in led mit bitmaske 

    }
    else{
      ledAus();

    }
}

void ledAus(){

        PORTD |= 0b00001100;
        PORTB |= && 0b00000001;
        PORTC |= 0b00000000;

}