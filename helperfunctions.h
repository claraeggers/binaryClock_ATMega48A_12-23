volatile uint8_t tag;
volatile uint8_t hourBitShiftDown = 0;
volatile uint8_t hourBitShiftUpper = 0;
uint8_t monate[] = { 31,28,31,30,31,30, 31, 31, 30, 31, 30, 31 };
uint8_t monate_schalt[] = { 31,29,31,30,31,30, 31, 31, 30, 31, 30, 31 };
uint8_t jahr = 2024;
uint8_t aktuellermonat = 3;
bool isSchalt = 1;

void entprellen() {
    if (prell > 0) {
        prell--; // Dekrementiere Prellvariable
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

        hourBitShiftDown = ( 0b00000111 && stunde); //Logik von 000xxxxx StundenByte für x = 0 oder 1 sollen die unteren 3 BITs auf PORTD angezeigt werden, dafür & mit bitmaske
        hourBitShiftUp = ( stunde >> 3); //Logig von 000xxxxx Stundenbyte fpr x = 0 oder 1 sollen bit 3 und bit 4 alleine stehen, dafür linksshift um 3
        PORTD = hourBitShiftDown;
        PORTB = hourBitShiftUp;
        PORTC = minute;         // display minute and hour uint8_t in led 

    }
    else{

    }
        //bei einem minutenwechsel wird die Zählvariable für Button-Einstellung der Uhrzeit genullt
        countingMin = 0;
        countingHour = 0;
}