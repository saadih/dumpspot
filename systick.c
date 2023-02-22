/*
 * startup.c
 *
 */
 
 #define GPIO_ODR ((volatile unsigned short *) 0x40020C14)
 #define STK_CTRL ((volatile unsigned int *)(0xE000E010))  
 #define STK_LOAD ((volatile unsigned int *)(0xE000E014))  
 #define STK_VAL  ((volatile unsigned int *)(0xE000E018))
 
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n"); /* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n"); /* call main */
__asm__ volatile(".L1: B .L1\n"); /* never return */
}

void delay_250ns( void )
{
/* SystemCoreClock = 168000000 */
*STK_CTRL = 0;
*STK_LOAD = ((168/4) -1);
*STK_VAL = 0;
*STK_CTRL = 5;
while( (*STK_CTRL & 0x10000 )== 0 );
*STK_CTRL = 0;
}

void init_app() {
* ((volatile unsigned int *) 0x40020C00) = 0x00005555;
}

void delay_mikro(int m) {
for (int i = 0 ; i < m*2 ; i++) {
delay_250ns();
}
}
void main(void)
{
init_app();
while(1) {
*GPIO_ODR = 0;
*GPIO_ODR = 0;
delay_mikro(5000);
*GPIO_ODR = 0xFF;
*GPIO_ODR = 0xFF;
delay_mikro(5000);
}

}
