
  void entprellen() {
    if (prell > 0) {
        prell--; // Dekrementiere Prellvariable
    }
    }

void monate(){
    //Funktion für Monate und Jahre
        if (jahr % 4 == 0) {
            if (tag >= monate_schalt[aktuellermonat]) {
            tag = 0;
            aktuellermonat++;
            if (aktuellermonat >= sizeof(monate_schalt)) {
                aktuellermonat = 0;
                jahr++;
                // ISR für EEPROM-Lese-/Schreiboperationen
            }
        }
        } else {
            if (tag >= monate[aktuellermonat]) {
            tag = 0;
            aktuellermonat++;
            if (aktuellermonat >= sizeof(monate)) {
                aktuellermonat = 0;
                jahr++;
                // ISR für EEPROM-Lese-/Schreiboperationen
            }
            }
        }
}