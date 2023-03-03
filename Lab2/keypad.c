/*
 * 	startup.c
 *
 */
 
 #define GPIO_ODR_HIGH ((volatile unsigned char *) 0x40020C15)
 #define GPIO_IDR ((volatile unsigned short *) 0x40020C10)
 #define GPIO_IDR_HIGH ((volatile unsigned char *) 0x40020C11)
 #define GPIO_ODR ((volatile unsigned short *) 0x40020C14)

__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}
// Keypad is set at high
// Segment is set at Low
void app_init(void) {
	// port D medium speed
	* ( (volatile unsigned int *) 0x40020C08) = 0x55555555;
	// GPIOD_MODER
	* ( (unsigned long *) 0x40020C00) = 0x55005555;
	// GPIOD_OTYPER
	* ( (unsigned short *) 0x40020C04) = 0x00000000; 
	// GPIOD_PUPDR 
	// * ( (unsigned int *) 0x40020C0C) = 0x00660000;
}

void kbdActivate( unsigned int row )
{ /* Aktivera angiven rad hos tangentbordet, eller
* deaktivera samtliga */

if (row == (unsigned int)1) {
	*GPIO_ODR_HIGH = 0x10;
}

if (row == (unsigned int)2) {
	*GPIO_ODR_HIGH = 0x20;
}
if (row == (unsigned int)3) {
	*GPIO_ODR_HIGH = 0x40;
}
if (row == (unsigned int)4) {
	*GPIO_ODR_HIGH = 0x80;
}
if (row == (unsigned int)0) {
	*GPIO_ODR_HIGH = 0x00;
}

}


/*
 * 
switch( row )
{ 
case 1: *GPIO_ODR = 0x01000000; break;
case 2: *GPIO_ODR = 0x02000000; break;
case 3: *GPIO_ODR = 0x10000000; break;
case 4: *GPIO_ODR = 0x20000000; break;
case 0: *GPIO_ODR = 0x00000000; break;
}
 */

int kbdGetCol ( void )
{ /* Om någon tangent (i aktiverad rad)
* är nedtryckt, returnera dess kolumnnummer,
* annars, returnera 0 */
unsigned char c;
c = *GPIO_IDR_HIGH;
if ( c & 0x8 ) return 4;
if ( c & 0x4 ) return 3;
if ( c & 0x2 ) return 2;
if ( c & 0x1 ) return 1;
return 0;
}

unsigned char keyb(void) {
	unsigned char key[]={1,2,3,0xA,4,5,6,0xB,7,8,9,0xC,0xE,0,0xF,0xD};
	int row,col;
	for (row = 1 ; row <= 4; row++) {
		kbdActivate(row);
		 if ( ( col = kbdGetCol() ) ) {
			kbdActivate(0);
			return key [4*(row-1)+(col-1)];
		}
	}
	kbdActivate(0);
	return 0xFF;
}

void out7seg(unsigned char c) {
	unsigned int translation[] = {0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,0b01011101,0b01111101,0b00000111,0b11111111,0b01101111,0b01110111,0b11111100,0b00111001,0b01011110,0b10000000,0b01100011};
	*GPIO_ODR = translation[c];
 }

void main(void)
{
	unsigned char c;
	
	app_init();
	while(1) {
	c = keyb();	
	out7seg(c);
	}
	
}

