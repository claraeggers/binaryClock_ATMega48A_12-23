volatile uint8_t tag;
uint8_t monate[] = { 31,28,31,30,31,30, 31, 31, 30, 31, 30, 31 };
uint8_t monate_schalt[] = { 31,29,31,30,31,30, 31, 31, 30, 31, 30, 31 };
uint8_t jahr = 2024;
uint8_t aktuellermonat = 3;

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
            //eeprom write stunde,  read tag, aktuellermonat, jahr == nicht ändern, ansonsten write tag, aktuellermonat, jahr
        }
}

void monatjahr(){
        //Funktion für Monate und Jahre
        if (jahr % 4 == 0) {
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
        // display minute and hour uint8_t in led 


    }
    else{
     
    }

}