#define SER_COM1 0x3F8                           /* Basisadresse COM1 */
#define SER_COM2 0x2F8                           /* Basisadresse COM2 */

#define SER_IRQ_COM1 4                         /* IRQ 4 = Vektor 0x0C */
#define SER_IRQ_COM2 3                         /* IRQ 3 = Vektor 0x0B */

#define SER_TXBUFFER       0x00                  /* Transmit Register */
#define SER_RXBUFFER       0x00                   /* Receive Register */
#define SER_DIVISOR_LSB    0x00              /* Baudraten Divisor LSB */
#define SER_DIVISOR_MSB    0x01              /* Baudraten Divisor MSB */
#define SER_IRQ_ENABLE     0x01          /* Interrupt-Enable-Register */
#define SER_IRQ_ID         0x02              /* Interrupt ID-Register */
#define SER_FIFO           0x02                      /* FIFO-Register */
#define SER_2FUNCTION      0x02        /* Alternate-Function-Register */
#define SER_LINE_CONTROL   0x03                  /* Leitungskontrolle */
#define SER_MODEM_CONTROL  0x04                     /* Modemkontrolle */
#define SER_LINE_STATUS    0x05                     /* Leitungsstatus */
#define SER_MODEM_STATUS   0x06                        /* Modemstatus */
#define SER_SCRATCH        0x07                   /* Scratch-Register */

              /* IRQ-Enable-Register-Bits (Interrupts an-/abschalten) */
#define SER_IER_RECEIVED 0x01                /* IRQ nach Datenempfang */
#define SER_IER_SENT     0x02             /* IRQ nach gesendetem Byte */
#define SER_IER_LINE     0x04   /* IRQ nach Leitungszustands-énderung */
#define SER_IER_MODEM    0x08      /* IRQ nach Modemzustands-énderung */

                    /* IRQ-ID - Bits (Durch was wurde IRQ ausgelîst?) */
#define SER_ID_PENDING     0x01        /* steht ein serieller IRQ an? */
#define SER_ID_MASK        0x06    /* ID wird in Bits 1 und 2 kodiert */
#define SER_ID_LINESTATUS  0x06 /* Leitungszustand (Error oder Break) */
#define SER_ID_RECEIVED    0x04                    /* Daten empfangen */
#define SER_ID_SENT        0x02                /* Byte wurde gesendet */
#define SER_ID_MODEMSTATUS 0x00    /* CTS, DSR, RI oder RLSD énderung */

            /* Bitbelegung im FIFO-Register (Falls mind. 16550A-UART) */
#define SER_FIFO_ENABLE        0x01
#define SER_FIFO_RESETRECEIVE  0x02
#define SER_FIFO_RESETTRANSMIT 0x04

      /* FIFO-Bits (Nach wievielen Bytes im FIFO wird IRQ ausgelîst ) */
#define SER_FIFO_TRIGGER0   0x00                            /* Normal */
#define SER_FIFO_TRIGGER4   0x40                           /* 4 Bytes */
#define SER_FIFO_TRIGGER8   0x80                           /* 8 Bytes */
#define SER_FIFO_TRIGGER14  0xC0                          /* 14 Bytes */

              /* Line-Control-Register - Bits (öbertragungsparameter) */
#define SER_LCR_WORDLEN    0x03   /* Anzahl der zu Åbertragenden Bits */
#define SER_LCR_5BITS      0x00
#define SER_LCR_6BITS      0x01
#define SER_LCR_7BITS      0x02
#define SER_LCR_8BITS      0x03
#define SER_LCR_2STOPBITS  0x04                /* 2 bzw. 1.5 Stopbits */
#define SER_LCR_1STOPBIT   0x00                          /* 1 Stopbit */

#define SER_LCR_NOPARITY   0x00         /* ParitÑtsprÅfung abschalten */
#define SER_LCR_ODDPARITY  0x08                   /* ungerade ParitÑt */
#define SER_LCR_EVENPARITY 0x18                     /* gerade ParitÑt */
#define SER_LCR_PARITYSET  0x28          /* ParitÑtsbit immer gesetzt */
#define SER_LCR_PARITYCLR  0x38         /* ParitÑtsbit immer gelîscht */
#define SER_LCR_PARITYMSK  0x38

#define SER_LCR_SENDBREAK  0x40    /* Sende Break solange Bit gesetzt */
#define SER_LCR_SETDIVISOR 0x80  /* fÅr Zugriff auf Baudraten-Divisor */

                  /* Modem-Control-Register - Bits (Signal Kontrolle) */
#define SER_MCR_DTR        0x01                  /* DTR-Signal setzen */
#define SER_MCR_RTS        0x02                  /* RTS-Signal setzen */
#define SER_MCR_UNUSED     0x04
#define SER_MCR_IRQENABLED 0x08     /* IRQ's an IRQ-Controller melden */
#define SER_MCR_LOOP       0x10                         /* Selbsttest */

                  /* Line-Status-Register - Bits (öbertragungsfehler) */
#define SER_LSR_DATARECEIVED 0x01 /* Datenwort (5 - 8 Bits) empfangen */
#define SER_LSR_OVERRUNERROR 0x02    /* vorheriges Datenwort verloren */
#define SER_LSR_PARITYERROR  0x04                   /* ParitÑtsfehler */
#define SER_LSR_FRAMINGERROR 0x08            /* Start-/Stopbit Fehler */
#define SER_LSR_BREAKDETECT  0x10                   /* Break entdeckt */
#define SER_LSR_ERRORMSK (SER_LSR_OVERRUNERROR|SER_LSR_PARITYERROR|\
                          SER_LSR_FRAMINGERROR|SER_LSR_BREAKDETECT)
#define SER_LSR_THREMPTY     0x20
#define SER_LSR_TSREMPTY     0x40

/* Modem Status Register - Bits (Welche Signale sind gesetzt)         */
/* Die Delta...-Bits geben an, ob sich der Zustand der entsprechenden */
/* Signale seit dem letzten Lesen des Modem Status Register verÑndert */
/* hat.                                                               */
#define SER_MSR_DCTS 0x01               /* Delta CTS (Zustand in CTS) */
#define SER_MSR_DDSR 0x02               /* Delta DSR (Zustand in DSR) */
#define SER_MSR_DRI  0x04                 /* Delta RI (Zustand in RI) */
#define SER_MSR_DCD  0x08                 /* Delta CD (Zustand in CD) */
#define SER_MSR_CTS  0x10                    /* Clear To Send gesetzt */
#define SER_MSR_DSR  0x20                   /* Data Set Ready gesetzt */
#define SER_MSR_RI   0x40                    /* Rin Indicator gesetzt */
#define SER_MSR_CD   0x80                   /* Carrier Detect gesetzt */

#define NOSER    0
#define INS8250  1                   /* National Semiconductor UART's */
#define NS16450  2
#define NS16550A 3
#define NS16C552 4

#define SER_MAXBAUD 115200L                      /* Maximale Baudrate */

#define SER_SUCCESS    0
#define SER_ERRSIGNALS 0x0300
#define SER_ERRTIMEOUT 0x0400
