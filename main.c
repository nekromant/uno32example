#include <plib.h>

/* We're running at 80Mhz */
#define F_CPU 80000000ul
// Define timing constants
#define SHORT_DELAY (500*8)
#define LONG_DELAY  (4000*8)


void _mon_putc (char c)
{
	while (U1STAbits.UTXBF);
	U1TXREG = c;
}

int _mon_getc (int canblock)
{
	if (canblock==0)
	{
		if (U1STAbits.URXDA)
			return (int)U1RXREG;
	}
	else
	{
		while (!U1STAbits.URXDA);
		return (int)U1RXREG;
	}
	return -1; 
}


void uart1_init(unsigned int speed)
{
    // Set Baud Rate register
    U1BRG = F_CPU/(speed*16)-1;

    // Set UART1 Mode register
    U1MODEbits.STSEL    = 0; // One Stop bit
    U1MODEbits.PDSEL0   = 0;
    U1MODEbits.PDSEL1   = 0; // 8-bit data, no parity
    U1MODEbits.BRGH     = 0; // BRG generates 16 clocks per bit period (16x baud clock, Standard mode)
    U1MODEbits.RXINV    = 0; // UxRX Idle state is '1'
    U1MODEbits.ABAUD    = 0; // Baud rate measurement disabled or completed
    U1MODEbits.LPBACK   = 0; // Loopback mode is disabled
    U1MODEbits.WAKE     = 0; // No wake-up enabled
    U1MODEbits.UEN0     = 0;
    U1MODEbits.UEN1     = 0; // UxTX and UxRX pins are enabled and used; UxCTS and UxRTS/BCLKx pins controlled by PORT latches
    U1MODEbits.RTSMD    = 1; // UxRTS pin in Simplex mode
    U1MODEbits.IREN     = 0; // IrDA encoder and decoder disabled
    U1MODEbits.USIDL    = 0; // Continue module operation in Idle mode
    U1MODEbits.UARTEN   = 1; // UARTx is enabled; all UARTx pins are controlled by UARTx as defined by UEN<1:0>


    // UART1 Status and Control register
    U1STAbits.ADDEN     = 0; // Address Detect mode disabled
    U1STAbits.URXISEL0  = 0;
    U1STAbits.URXISEL1  = 0; // Interrupt is set when any character is received and transferred from the RSR to the receive buffer. Receive buffer has one or more characters.
    U1STAbits.UTXEN     = 1; // Transmit enabled, UxTX pin controlled by UARTx
    U1STAbits.UTXBRK    = 0; // Sync Break transmission disabled or completed
    U1STAbits.UTXINV    = 0; // UxTX Idle '1'
    U1STAbits.UTXISEL0  = 0;
    U1STAbits.UTXISEL1  = 1; // Interrupt when a character is transferred to the Transmit Shift Register (TSR) and as a result, the transmit buffer becomes empty
      
    // Clear UART1 Receive and Transmit interrupt flags
    IFS0bits.U1RXIF = 0; 
    IFS0bits.U1TXIF = 0; 

    /* No interrupts. Yet */
    // Enable UART1 Receive interrupt
    //IEC0bits.U1RXIE = 1;
}


int main()
{
	mJTAGPortEnable(DEBUG_JTAGPORT_OFF);
	mPORTFClearBits(BIT_0);

	// Make all lower 8-bits of PORTA as output
	mPORTFSetPinsDigitalOut( BIT_0 );
	TRISE=0x0;
	PORTE = 0x0f;

	// Start timer1, Fpb/256, max period
	OpenTimer1(T1_ON | T1_PS_1_256 | T1_SOURCE_INT, 0xFFFF);

	// The main loop
	
	PORTSetPinsDigitalIn(IOPORT_D, BIT_5);  
	mCNOpen(CN_ON, CN14_ENABLE, 0);
	
	// Read the port to clear any mismatch on change notice pins
	int dummy = PORTD;
	
	// Clear change notice interrupt flag
	ConfigIntCN(CHANGE_INT_ON | CHANGE_INT_PRI_2);

	INTEnableSystemMultiVectoredInt();

	uart1_init(115200);
	setbuf(stdin, NULL); //no input buffer (for scanf)
	setbuf(stdout, NULL); //no output buffer (for printf) 
	
	printf ("Hello World!\r\n");
	
	
	while( 1)
	{
		putchar(getchar());
		WriteTimer1(0);
		while ( TMR1 < LONG_DELAY);
		PORTE+=1;
	} 
}

void __ISR(_CHANGE_NOTICE_VECTOR, ipl6) ChangeNotice_Handler(void)
{
    // Step #1 - always clear the mismatch condition first
	int dummy = PORTReadBits(IOPORT_D, BIT_5);;
    
	// Step #2 - then clear the interrupt flag
	mCNClearIntFlag();
	
	PORTF ^= BIT_0;
}
