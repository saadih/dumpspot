/*
 * 	startup.c
 *
 */
 
 #define SIMULATOR = false;
 
 #ifdef SIMULATOR
 #define DELAY_COUNT 100
 #else 
 #define DELAY_COUNT 100000
 #endif
 
#define GPIO_ODR_LOW ((volatile unsigned int *) 0x40020C14)
#define STK_CTRL ((volatile unsigned int *)(0xE000E010))  
#define STK_LOAD ((volatile unsigned int *)(0xE000E014))  
#define STK_VAL  ((volatile unsigned int *)(0xE000E018))


__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}
static volatile int systick_flag;
static volatile int delay_count;
void systick_irq_handler(void);

void app_init(void) {
	// PORT D init
	*((volatile unsigned long *) 0x40020C00) = 0x00005555;
	
	// USBDM
	*((unsigned long *) 0x40023830) = 0x18;
	*((volatile unsigned long *) 0x40023844) |= 0x4000;
	*((unsigned long *) 0xE000ED08) = 0x2001C000;
	
	
	*((volatile void (**)(void)) 0x2001C03C) = systick_irq_handler;
	systick_flag = 0;

}
void delay_1mikro(void) {
	systick_flag = 0;
	*STK_CTRL = 0;
	*STK_LOAD = (167);
	*STK_VAL = 0;
	*STK_CTRL = 7;
}


void systick_irq_handler(void) {
	*STK_CTRL = 0;
	delay_count--;
	if (delay_count > 0) {
		delay_1mikro();
	} else {
		systick_flag = 1;
	}
}
void delay(unsigned int count)
{
	delay_count = count;
	delay_1mikro();
}



void main(void)
{	
	app_init();
	*GPIO_ODR_LOW = 0;
	delay(DELAY_COUNT* 100);
	*GPIO_ODR_LOW = 0xFF;
	while(1) {
		if (systick_flag) break;
	} 
	*GPIO_ODR_LOW = 0;
}

