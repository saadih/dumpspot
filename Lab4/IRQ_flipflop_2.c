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

#define SYSCFG_EXTICR1 ((int*) 0x40013808)

#define EXTI_IMR ((unsigned int *) 0x40013C00)
#define EXTI_FTSR ((unsigned int *) 0x40013C0C)
#define EXTI_RTSR ((unsigned int *) 0x40013C08)
#define EXTI_PR ((unsigned int *)  0x40013C14)

#define EXTI3_IRQVEC ((void (**)(void) ) 0x2001C064)
#define EXTI2_IRQVEC ((void (**)(void) ) 0x2001C060)
#define EXTI1_IRQVEC ((void (**)(void) ) 0x2001C05C)
#define EXTI0_IRQVEC ((void (**)(void) ) 0x2001C058)

#define NVIC_ISER0 ((int*)0xE000E100)

#define NVIC_EXTI3_IRQ_BPOS (1<<9)
#define NVIC_EXTI2_IRQ_BPOS (1<<8)
#define NVIC_EXTI1_IRQ_BPOS (1<<7)
#define NVIC_EXTI0_IRQ_BPOS (1<<6)

#define EXTI3_IRQ_BPOS (1<<3)
#define EXTI2_IRQ_BPOS (1<<2)
#define EXTI1_IRQ_BPOS (1<<1)
#define EXTI0_IRQ_BPOS (1<<0)

void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

void irq0_handler(void) {
	*EXTI_PR |= EXTI0_IRQ_BPOS;
	*GPIO_D_ODR_LOW += 1;
	
	*GPIO_E_MODER = 0x0005555;
	int bit = 4;
	*GPIO_E_ODR_LOW |= (1 << bit);
	*GPIO_E_ODR_LOW &= ~(1 << bit);
	*GPIO_E_MODER = 0x0000000;
	
}

void irq1_handler(void) {
	*EXTI_PR |= EXTI1_IRQ_BPOS;
	*GPIO_D_ODR_LOW = 0;
	
	*GPIO_E_MODER = 0x0005555;
	int bit = 5;
	*GPIO_E_ODR_LOW |= (1 << bit);
	*GPIO_E_ODR_LOW &= ~(1 << bit);
	*GPIO_E_MODER = 0x0000000;
	
}

void irq2_handler(void) {
	*EXTI_PR |= EXTI2_IRQ_BPOS;
	if (*GPIO_D_ODR_LOW == 0 ) {
		*GPIO_D_ODR_LOW = 0xFF;
	} else {
		*GPIO_D_ODR_LOW = 0;
	}
	
	*GPIO_E_MODER = 0x0005555;
	int bit = 6;
	*GPIO_E_ODR_LOW |= (1 << bit);
	*GPIO_E_ODR_LOW &= ~(1 << bit);
	*GPIO_E_MODER = 0x0000000;
	
}

int initapp(void) {
	// Aktivera D MODER
		*GPIO_D_MODER = 0x00005555;
	// USBDM
	 *((unsigned long *) 0x40023830) = 0x18;
	 *((volatile unsigned long *) 0x40023844) |= 0x4000;
	 *((unsigned long *) 0xE000ED08) = 0x2001C000;	
	
	// PE0 -> EXTI0 
	* ((unsigned int *) SYSCFG_EXTICR1) &= 0xFFF0;
	* ((unsigned int *) SYSCFG_EXTICR1) |= 0x0004;
	// PE1 -> EXTI1 
	* ((unsigned int *) SYSCFG_EXTICR1) &= 0xFF0F;
	* ((unsigned int *) SYSCFG_EXTICR1) |= 0x0040;
	// PE2 -> EXTI2
	* ((unsigned int *) SYSCFG_EXTICR1) &= 0xF0FF;
	* ((unsigned int *) SYSCFG_EXTICR1) |= 0x0400;
	
	// Aktivera alla brottlinor
	*EXTI_IMR |= EXTI0_IRQ_BPOS;
	*EXTI_IMR |= EXTI1_IRQ_BPOS;
	*EXTI_IMR |= EXTI2_IRQ_BPOS;
	
	// Endast positiv flank?
	*EXTI_RTSR |= EXTI0_IRQ_BPOS;
	*EXTI_RTSR |= EXTI1_IRQ_BPOS;
	*EXTI_RTSR |= EXTI2_IRQ_BPOS;
	
	// Sätt handlers
	* EXTI2_IRQVEC = &irq2_handler;
	* EXTI1_IRQVEC = &irq1_handler;
	* EXTI0_IRQVEC = &irq0_handler;
	
	// Aktivera linorna för NVIC
	*NVIC_ISER0 |= NVIC_EXTI2_IRQ_BPOS;
	*NVIC_ISER0 |= NVIC_EXTI1_IRQ_BPOS;
	*NVIC_ISER0 |= NVIC_EXTI0_IRQ_BPOS;

}

void main(void)
{
	initapp();
	while(1) {
		
	}
}

