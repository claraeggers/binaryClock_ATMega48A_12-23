   void initialisieren(){

    //EXT CLOCK SOURCE


    //TIMER2 OVF CTC
    TCCR2A = (1 << WGM21); 
    TCCR2B = (1 << CS22) | (1 << CS21);     //ps=256, Timer 2, (32,768 kHz = 32768/128*256 = 1 1/s == 1s)
    OCR2A = 124; // 125-1
    TIFR2 |= (1<<OCF2A) | (1<<TOV2); //flag register timer interrupt for ocie0b
    TIMSK |= (1<<OCIE2A); //enable compare interrupt
    TIMSK |= (1<<TOIE2); //enable overflow 

    //INTERRUPTS
    EIMSK |= (1<<INT0) | (1<<INT1); // enable interupt0 and interrupt 1
    EICRA |= (1<<ISC11) | (1<<ISC01); // enable external interrupt 0/1 auf fallende flanke
    PCICR |= (1<<PCIE0)// pin change interrupt enable for hourcounter in pcint 0-7

    //WATCHDOG
    MCUSR &= ~(1 << WDRF); // Watchdog-Reset löschen
    wdt_disable();
    wdt_enable(WDT0_20S);
    WDTCSR = (1 << WDIE) | (1 << WDE) | (1 << WDP3) | (1 << WDP0); // WDT auf 20 Sekunden einstellen
    WDTCR |= (1 << WDIE);     // Watchdog-Interrupt aktivieren

    //POWER-REDUCTION
    PRR |= (1<<TWI) | (1<<PRUSART0) | (1<<PRTIM0) | (1<<PRTIM1); // Power Reduction Register turns of TWI,timer0/1,usart by initialisation

    //SET-DDR and pull-up
    DDRC = 0b00111111; //PC0-5 als led output hour
    DDRB = 0b00010110; //PB4 as XTAL1 output, pb1 undßb2 als led output min
    DDRD = 0b11100000; //pd5-7 als led output hour
    sleepB = 1;
    hButton = 1;
    mButton = 1



    sei();

    }