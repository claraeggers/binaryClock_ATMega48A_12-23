
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


void init_PWM() {
    // PWM mit TC2 an Pin 17 (OC2A an PORTB,3):
    TCCR2A = (1 << COM2A1) | (1 << WGM21) | (1 << WGM20); // Nicht-invertierter Modus, Fast PWM
    TCCR2B = (1 << CS22) | (1 << CS21); // Prescaler /256, resultiert in einer PWM-Frequenz von 1 Hz
}

void set_LED_PWM(uint8_t brightness) {
    OCR2A = brightness; // für PWM
}