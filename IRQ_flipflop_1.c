/*
 * 	startup.c
 *
 */
__attribute__((naked)) __attribute__((section (".start_section")) )

#define PORT_D 0x40020C00
#define GPIO_D_MODER ((volatile unsigned int *)(PORT_D))
#define GPIO_D_ODR_LOW ((volatile unsigned char *)(PORT_D + 0x14))

#define PORT_E 0x40021000
#define GPIO_E_MODER ((volatile unsigned int *)(PORT_E))
#define GPIO_E_OTYPER ((volatile unsigned short *)(PORT_E + 0x4))
#define GPIO_E_SPEEDR ((volatile unsigned int *)(PORT_E + 0x8))
#define GPIO_E_PUPDR ((volatile unsigned int *)(PORT_E + 0xC))
#define GPIO_E_IDR_LOW ((volatile unsigned char *)(PORT_E + 0x10))
#define GPIO_E_IDR_HIGH ((volatile unsigned char *)(PORT_E + 0x11))
#define GPIO_E_ODR_LOW ((volatile unsigned char *)(PORT_E + 0x14))
#define GPIO_E_ODR_HIGH ((volatile unsigned char *)(PORT_E + 0x15))

#define EXTI_IMR ((unsigned int *) 0x40013C00)
#define EXTI_FTSR ((unsigned int *) 0x40013C0C)
#define EXTI_RTSR ((unsigned int *) 0x40013C08)
#define EXTI_PR ((unsigned int *)  0x40013C14)

#define EXTI3_IRQVEC 0x2001C064
#define EXTI2_IRQVEC 0x2001C060
#define EXTI1_IRQVEC 0x2001C05C
#define EXTI0_IRQVEC 0x2001C058

#define NVIC_ISER0 ((int*)0xE000E100)

#define SYSCFG_EXTICR1 ((int*) 0x40013808)


void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}
void irq_handler(void);


void irq_handler(void) {
	int bit;
	
	*GPIO_D_MODER = 0x00005555;
		*EXTI_PR |= (1 << 3);
	if (*GPIO_E_IDR_LOW == 0x08 + (1 << 0) ) {
		*GPIO_E_MODER = 0x0005555;
		bit = 4;
		*GPIO_E_ODR_LOW |= (1 << bit);
		*GPIO_E_ODR_LOW &= ~(1 << bit);
		*GPIO_D_ODR_LOW += 1;
		*GPIO_E_MODER = 0x0000000;
	}
	
	if (*GPIO_E_IDR_LOW == 0x08 + (1 << 1) ) {
		*GPIO_E_MODER = 0x0005555;
		bit = 5;
		*GPIO_E_ODR_LOW |= (1 << bit);
		*GPIO_E_ODR_LOW &= ~(1 << bit);
		*GPIO_D_ODR_LOW = 0;
		*GPIO_E_MODER = 0x0000000;
	}
	
	if (*GPIO_E_IDR_LOW == 0x08 + (1 << 2) ) {
		*GPIO_E_MODER = 0x0005555;
		bit = 6;
		*GPIO_E_ODR_LOW |= (1 << bit);
		*GPIO_E_ODR_LOW &= ~(1 << bit);
		if (*GPIO_D_ODR_LOW == 0 ) {
			 *GPIO_D_ODR_LOW = 0xFF;
		} else
		{
			*GPIO_D_ODR_LOW = 0;
		}
		*GPIO_E_MODER = 0x0000000;
	}
	
}


// 0x4000 = 0b 0100 0000 0000 0000
void appInit(void) {
	// Nollställ SYSCFG_EXTICR1
	// Vi vill antagligen ha RST som utgångar.. update : gör de istället i handlern
	//* GPIO_E_MODER = 0b00000000000000000010101000000000; 
	
	* ((unsigned int *) SYSCFG_EXTICR1) &= 0x0FFF;
	// PE3 -> EXTI3 
	* ((unsigned int *) SYSCFG_EXTICR1) |= 0x4000;
	
	// Aktivera avbrottslinan
	*EXTI_IMR |= (1 << 3);
	
	// Vi vill antagligen aktivera vid båda flank
	*EXTI_RTSR |= (1 << 3);
	*EXTI_FTSR |= (1 << 3);
	
	// Hitta address vektorn o sätt handler funktionen för den
	*((volatile void (**)(void) ) 0x2001C000 + 0x64) = irq_handler;
	
	//NVIC grejen
	*((volatile unsigned int *) 0xE000E100) |= (1 << 9);
}



void main(void)
{
	appInit();
	
	while(1) {
	
	}
}

