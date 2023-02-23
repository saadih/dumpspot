/*
 * 	startup.c
 *
 */
 #include <math.h>

 //delay del port D
 #define GPIO_ODR ((volatile unsigned short *) 0x40020C14)
 #define STK_CTRL ((volatile unsigned int *)(0xE000E010))  
 #define STK_LOAD ((volatile unsigned int *)(0xE000E014))  
 #define STK_VAL  ((volatile unsigned int *)(0xE000E018))
 
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

void delay_mikro(int m) {
for (int i = 0 ; i < m*2 ; i++) {
delay_250ns();
}
}


 
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

__attribute__((naked)) void graphic_initialize(void)
{
    __asm volatile(" .HWORD 0xDFF0\n");
    __asm volatile(" BX LR\n");
}

__attribute__((naked))
 void graphic_clear_screen(void)
{
    __asm volatile(" .HWORD 0xDFF1\n");
    __asm volatile(" BX LR\n");
}

__attribute__((naked))
 void graphic_pixel_set(int x, int y)
{
    __asm volatile(" .HWORD 0xDFF2\n");
    __asm volatile(" BX LR\n");
}

__attribute__((naked))
 void graphic_pixel_clear(int x, int y)
{
    __asm volatile(" .HWORD 0xDFF3\n");
    __asm volatile(" BX LR\n");
}
void swap(int* a, int* b) 
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

typedef struct {
  int x,y;
} POINT, *PPOINT;

typedef struct {
  POINT p0,p1; 
} LINE, *PLINE;

typedef struct polygonpoint{
	char x,y;
	struct polygonpoint *next;
} POLYPOINT,*PPOLYPOINT;


void draw_line(PLINE L)
{
	
	int x0 = L->p0.x;
	int y0 = L->p0.y;
	int x1 = L->p1.x;
	int y1 = L->p1.y;
	// int x1 = L[2];int y0 = L[1];int y1 = L[3];
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx, sy;
    if (x0 < x1) {
        sx = 1;
    } else {
        sx = -1;
    }
    if (y0 < y1) {
        sy = 1;
    } else {
        sy = -1;
    }
    int err = dx - dy;
    int x = x0;
    int y = y0;
    while (1) {
        graphic_pixel_set(x, y);
        if (x == x1 && y == y1) {
            break;
        }
        int e2 = 2 * err;
        if (e2 > -dy) {
            err = err - dy;
            x = x + sx;
        }
        if (e2 < dx) {
            err = err + dx;
            y = y + sy;
        }
    }
}
void draw_polygon(PPOLYPOINT p) {
	POINT p0;
	p0.x = p->x;
	p0.y = p->y;
	POINT p1;
	PPOLYPOINT ptr = p->next;
	while (ptr != 0) {
		p1.x = ptr->x;
		p1.y = ptr->y;
		LINE line = {p0,p1};
		draw_line(&line);
		p0.x = p1.x;
		p0.y = p1.y;
		ptr = ptr->next;
	}
}

void main(void)
{
	graphic_initialize();
	graphic_clear_screen();
	POLYPOINT pg8 = {20,20,0};
	POLYPOINT pg7 = {20,55,&pg8};
	POLYPOINT pg6 = {70,60,&pg7};
	POLYPOINT pg5 = {80,35,&pg6};
	POLYPOINT pg4 = {100,25,&pg5};
	POLYPOINT pg3 = {90,10,&pg4};
	POLYPOINT pg2 = {40,10,&pg3};
	POLYPOINT pg1 = {20,20,&pg2};
 
	
	while(1) {
		draw_polygon(&pg1);
		delay_mikro(2000);
		graphic_clear_screen();
	}

}

