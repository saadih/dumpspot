/*
 * 	startup.c
 *
 */
 
 #define PORT_BASE 		( (volatile unsigned int * ) 	0x40021000 )
 
 #define portModer 		( (volatile unsigned int * ) 	PORT_BASE  )
 #define portOtyper		( (volatile unsigned short * ) 	PORT_BASE + 0x04)
 #define portOspeedr	( (volatile unsigned int * ) 	PORT_BASE + 0x08)
 #define portPupdr		( (volatile unsigned int * ) 	PORT_BASE + 0x0C)
 
 #define portIdrLow		( (volatile unsigned char * ) 	PORT_BASE + 0x10)
 #define portIdrHigh	( (volatile unsigned char * ) 	PORT_BASE + 0x11)
 
 #define portOdrLow		( (volatile unsigned char * ) 	PORT_BASE + 0x14)
 #define portOdrHigh	( (volatile unsigned char * ) 	PORT_BASE + 0x15)
 
 #define GPIO_ODR ((volatile unsigned short *) 0x40020C14)
 #define STK_CTRL ((volatile unsigned int *)(0xE000E010))  
 #define STK_LOAD ((volatile unsigned int *)(0xE000E014))  
 #define STK_VAL  ((volatile unsigned int *)(0xE000E018))
 
	// Masker för kontrollbitar 
 #define B_E 0x40 /* Enable-signal */
 #define B_SELECT 4 /* Välj ASCII-display */
 #define B_RW 2 /* 0=Write, 1=Read */
 #define B_RS 1 /* 0=Control, 1=Data */
 
 
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

void init_app(void) {
	* PORT_BASE = 0x55555555;
}


void delay_250ns( void )
{
/* SystemCoreClock = 168000000 */
	*STK_CTRL = 0;
	*STK_LOAD = ((168/4) -1);
	*STK_VAL = 0;
	*STK_CTRL = 5;
	while( (*STK_CTRL & 0x10000 ) == 0 );
	*STK_CTRL = 0;
}

void delay_micro(int u) {
	/* SystemCoreClock = 168000000 */
	for (int i = 0 ; i < u*4 ; i++ ) {
		delay_250ns();
	}
}

// addressera ascii display och ettställ de bitar som är 1 i x
void ascii_ctrl_bit_set(unsigned char x) {
	char c;
	c = *portOdrLow;
	*portOdrLow = B_SELECT | x | c;
}

// addressera ascii display och nollställ de bitar som är 1 i x
void ascii_ctrl_bit_clear(unsigned char x) {
	char c;
	c = *portOdrLow;
	c = c & ~x;
	*portOdrLow = B_SELECT | c;
}

void ascii_write_controller( char c )
{
	ascii_ctrl_bit_set( B_E );
	*portOdrHigh = c; 
	ascii_ctrl_bit_clear( B_E );
	delay_250ns();
}

unsigned char ascii_read_controller( void )
{
	char c;
	ascii_ctrl_bit_set( B_E );
	delay_250ns();
	delay_250ns();
	c = *portIdrHigh;
	ascii_ctrl_bit_clear( B_E );
	return c;
}

void ascii_write_cmd(unsigned char command) {
	ascii_ctrl_bit_clear(B_RS);
	ascii_ctrl_bit_clear(B_RW);
	ascii_write_controller(command);
}

void ascii_write_data(unsigned char data) {
	ascii_ctrl_bit_set(B_RS);
	ascii_ctrl_bit_clear(B_RW);
	ascii_write_controller(data);
}

unsigned char ascii_read_status( void )
{
	char c;
	*portModer = 0x00005555;
	ascii_ctrl_bit_set( B_RW );
	ascii_ctrl_bit_clear( B_RS );
	c = ascii_read_controller();
	*portModer = 0x55555555;
	
	return c;
}

unsigned char ascii_read_data(void) {
	
	char c;
	*portModer = 0x00005555;
	ascii_ctrl_bit_set( B_RW );
	ascii_ctrl_bit_set( B_RS );
	c = ascii_read_controller();
	*portModer = 0x55555555;
	
	return c;
}

int waitForClear(void) {
	while ((ascii_read_status() & 0x80)==0x80) {}
		delay_micro(8);
	return 0;
}

// ------------------------

void ascii_init(void) {
	waitForClear();
	ascii_write_cmd(0b00111000);
	delay_micro(40);
	
	waitForClear();
	ascii_write_cmd(0b00001111);
	delay_micro(40);
	
	waitForClear();
	ascii_write_cmd(1);
	delay_micro(2000);
	
	waitForClear();
	ascii_write_cmd(0b00000110);
	delay_micro(40);
}	


void ascii_gotoxy(int x, int y) {
	unsigned char address = x - 1;
	if (y == 2) {
		address = address + 0x40;
	}
	ascii_write_cmd(0x80 | address);
}

void ascii_write_char(unsigned char s) {
	waitForClear();
	delay_micro(8);
	ascii_write_data(s);
}


void main(void)
{
	unsigned char *s;
	unsigned char test1[] = "Alfanumeriskt";
	unsigned char test2[] = "Display - test";
	
	init_app();
	ascii_init();
	ascii_gotoxy(1,1);
	s = test1;
	while (*s) {
		ascii_write_char(*s++);
	}
	ascii_gotoxy(1,2);
	s = test2;
	while (*s) {
		ascii_write_char(*s++);
	}
}

