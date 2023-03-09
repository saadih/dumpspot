/*
 * 	startup.c
 *
 */

// KEYPAD = PORT D HIGH
// GRAPHICS DISPLAY = PORT E

__attribute__((naked)) __attribute__((section(".start_section"))) void startup(void)
{
    __asm__ volatile(" LDR R0,=0x2001C000\n"); /* set stack */
    __asm__ volatile(" MOV SP,R0\n");
    __asm__ volatile(" BL main\n");   /* call main */
    __asm__ volatile(".L1: B .L1\n"); /* never return */
}

#define PORT_D 0x40020C00
#define GPIO_D_MODER ((volatile unsigned int*)(PORT_D))
#define GPIO_D_OTYPER ((volatile unsigned short*)(PORT_D + 0x4))
#define GPIO_D_OSPEEDR ((volatile unsigned int*)(PORT_D + 0x8))
#define GPIO_D_PUPDR ((volatile unsigned int*)(PORT_D + 0xC))
#define GPIO_D_IDR_LOW ((volatile unsigned char*)(PORT_D + 0x10))
#define GPIO_D_IDR_HIGH ((volatile unsigned char*)(PORT_D + 0x11))
#define GPIO_D_ODR_LOW ((volatile unsigned char*)(PORT_D + 0x14))
#define GPIO_D_ODR_HIGH ((volatile unsigned char*)(PORT_D + 0x15))

#define PORT_E 0x40021000
#define GPIO_E_MODER ((volatile unsigned int*)(PORT_E))
#define GPIO_E_OTYPER ((volatile unsigned short*)(PORT_E + 0x4))
#define GPIO_E_OSPEEDR ((volatile unsigned int*)(PORT_E + 0x8))
#define GPIO_E_PUPDR ((volatile unsigned int*)(PORT_E + 0xC))
#define GPIO_E_IDR_LOW ((volatile unsigned char*)(PORT_E + 0x10))
#define GPIO_E_IDR_HIGH ((volatile unsigned char*)(PORT_E + 0x11))
#define GPIO_E_ODR_LOW ((volatile unsigned char*)(PORT_E + 0x14))
#define GPIO_E_ODR_HIGH ((volatile unsigned char*)(PORT_E + 0x15))

#define STK_CTRL ((volatile unsigned int*)(0xE000E010))
#define STK_LOAD ((volatile unsigned int*)(0xE000E014))
#define STK_VAL ((volatile unsigned int*)(0xE000E018))

#define SYSCFG_EXTICR1 ((int*)0x40013808)

#define EXTI_IMR ((unsigned int*)0x40013C00)
#define EXTI_FTSR ((unsigned int*)0x40013C0C)
#define EXTI_RTSR ((unsigned int*)0x40013C08)
#define EXTI_PR ((unsigned int*)0x40013C14)

#define EXTI3_IRQVEC ((void (**)(void))0x2001C064)
#define EXTI2_IRQVEC ((void (**)(void))0x2001C060)
#define EXTI1_IRQVEC ((void (**)(void))0x2001C05C)
#define EXTI0_IRQVEC ((void (**)(void))0x2001C058)

#define NVIC_ISER0 ((int*)0xE000E100)

#define NVIC_EXTI3_IRQ_BPOS (1 << 9)
#define NVIC_EXTI2_IRQ_BPOS (1 << 8)
#define NVIC_EXTI1_IRQ_BPOS (1 << 7)
#define NVIC_EXTI0_IRQ_BPOS (1 << 6)

#define EXTI3_IRQ_BPOS (1 << 3)
#define EXTI2_IRQ_BPOS (1 << 2)
#define EXTI1_IRQ_BPOS (1 << 1)
#define EXTI0_IRQ_BPOS (1 << 0)


// Nilos section
#define PORT_BASE 		( (volatile unsigned int * ) 	0x40021000 )

 #define portModer 		( (volatile unsigned int * ) 	PORT_BASE  )

 #define portOtyper		( (volatile unsigned short * ) 	PORT_BASE + 0x04)

 #define portOspeedr	( (volatile unsigned int * ) 	PORT_BASE + 0x08)

 #define portPupdr		( (volatile unsigned int * ) 	PORT_BASE + 0x0C)

 #define portIdrLow		( (volatile unsigned char * ) 	PORT_BASE + 0x10)

 #define portIdrHigh	( (volatile unsigned char * ) 	PORT_BASE + 0x11)

 #define portOdrLow		( (volatile unsigned char * ) 	PORT_BASE + 0x14)

 #define portOdrHigh	( (volatile unsigned char * ) 	PORT_BASE + 0x15) 

	// Masker för kontrollbitar 

 #define B_E 0x40 /* Enable-signal */

 #define B_SELECT 4 /* Välj ASCII-display */

 #define B_RW 2 /* 0=Write, 1=Read */

 #define B_RS 1 /* 0=Control, 1=Data */

 

 /*TIMER6*/

 

 #define TIM6_CR1 ((volatile unsigned short *) 0x40001000)

 #define TIM6_CNT ((volatile unsigned short *) 0x40001024)

 #define TIM6_ARR ((volatile unsigned short *) 0x4000102C)

 #define UDIS (1<<1)

 #define CEN (1<<0)

 // C Port

 #define GPIO_ODR_LOW ((volatile unsigned char *) (0x40020814))


#define MAX_POINTS 45

// Masker för kontrollbitar
#define B_E 0x40   /* Enable-signal */
#define B_SELECT 4 /* Välj ASCII-display */
#define B_RW 2     /* 0=Write, 1=Read */
#define B_RS 1     /* 0=Control, 1=Data */

// Delay functions
void delay_250ns(void)
{
    /* SystemCoreClock = 168000000 */
    *STK_CTRL = 0;
    *STK_LOAD = ((168 / 4) - 1);
    *STK_VAL = 0;
    *STK_CTRL = 5;
    while((*STK_CTRL & 0x10000) == 0)
	;
    *STK_CTRL = 0;
}

void delay_micro(int u)
{
    /* SystemCoreClock = 168000000 */
    for(int i = 0; i < u * 4; i++) {
	delay_250ns();
    }
}

// Graphics
__attribute__((naked)) void graphic_initialize(void)
{
    __asm volatile(" .HWORD 0xDFF0\n");
    __asm volatile(" BX LR\n");
}

__attribute__((naked)) void graphic_clear_screen(void)
{
    __asm volatile(" .HWORD 0xDFF1\n");
    __asm volatile(" BX LR\n");
}

__attribute__((naked)) void graphic_pixel_set(int x, int y)
{
    __asm volatile(" .HWORD 0xDFF2\n");
    __asm volatile(" BX LR\n");
}

__attribute__((naked)) void graphic_pixel_clear(int x, int y)
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
    int x, y;
} POINT, *PPOINT;

typedef struct {
    POINT p0, p1;
} LINE, *PLINE;

typedef struct {
    int numpoints;
    int sizex;
    int sizey;
    POINT px[MAX_POINTS];
} GEOMETRY, *PGEOMETRY;

typedef struct tObj {
    PGEOMETRY geo;
    int dirx, diry;
    int posx, posy;
    void (*draw)(struct tObj*);
    void (*clear)(struct tObj*);
    void (*move)(struct tObj*);
    void (*set_speed)(struct tObj*, int, int);
} OBJECT, *POBJECT;

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
    if(x0 < x1) {
	sx = 1;
    } else {
	sx = -1;
    }
    if(y0 < y1) {
	sy = 1;
    } else {
	sy = -1;
    }
    int err = dx - dy;
    int x = x0;
    int y = y0;
    while(1) {
	graphic_pixel_set(x, y);
	if(x == x1 && y == y1) {
	    break;
	}
	int e2 = 2 * err;
	if(e2 > -dy) {
	    err = err - dy;
	    x = x + sx;
	}
	if(e2 < dx) {
	    err = err + dx;
	    y = y + sy;
	}
    }
}

// ASCII

// addressera ascii display och ettställ de bitar som är 1 i x
void ascii_ctrl_bit_set(unsigned char x)
{
    char c;
    c = *GPIO_E_ODR_LOW;
    *GPIO_E_ODR_LOW = B_SELECT | x | c;
}

// addressera ascii display och nollställ de bitar som är 1 i x
void ascii_ctrl_bit_clear(unsigned char x)
{
    char c;
    c = *GPIO_E_ODR_LOW;
    c = c & ~x;
    *GPIO_E_ODR_LOW = B_SELECT | c;
}

void ascii_write_controller(char c)
{
    ascii_ctrl_bit_set(B_E);
    *GPIO_E_ODR_HIGH = c;
    ascii_ctrl_bit_clear(B_E);
    delay_250ns();
}

unsigned char ascii_read_controller(void)
{
    char c;
    ascii_ctrl_bit_set(B_E);
    delay_250ns();
    delay_250ns();
    c = *GPIO_E_IDR_HIGH;
    ascii_ctrl_bit_clear(B_E);
    return c;
}

void ascii_write_cmd(unsigned char command)
{
    ascii_ctrl_bit_clear(B_RS);
    ascii_ctrl_bit_clear(B_RW);
    ascii_write_controller(command);
}

void ascii_write_data(unsigned char data)
{
    ascii_ctrl_bit_set(B_RS);
    ascii_ctrl_bit_clear(B_RW);
    ascii_write_controller(data);
}

unsigned char ascii_read_status(void)
{
    char c;
    *GPIO_E_MODER = 0x00005555;
    ascii_ctrl_bit_set(B_RW);
    ascii_ctrl_bit_clear(B_RS);
    c = ascii_read_controller();
    *GPIO_E_MODER = 0x55555555;

    return c;
}

unsigned char ascii_read_data(void)
{

    char c;
    *GPIO_E_MODER = 0x00005555;
    ascii_ctrl_bit_set(B_RW);
    ascii_ctrl_bit_set(B_RS);
    c = ascii_read_controller();
    *GPIO_E_MODER = 0x55555555;

    return c;
}

int waitForClear(void)
{
    while((ascii_read_status() & 0x80) == 0x80) {
    }
    delay_micro(8);
    return 0;
}

void write_string(char s[])
{
    while(*s) {
	ascii_write_char(*s++);
    }
}

// ------------------------

void ascii_init(void)
{
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

void ascii_gotoxy(int x, int y)
{
    unsigned char address = x - 1;
    if(y == 2) {
	address = address + 0x40;
    }
    ascii_write_cmd(0x80 | address);
}

void ascii_write_char(unsigned char s)
{
    waitForClear();
    delay_micro(8);
    ascii_write_data(s);
}

// Keypad

void kbdActivate(unsigned int row)
{ /* Aktivera angiven rad hos tangentbordet, eller
   * deaktivera samtliga */
    if(row == (unsigned int)1) {
	*GPIO_D_ODR_HIGH = 0x10;
    }
    if(row == (unsigned int)2) {
	*GPIO_D_ODR_HIGH = 0x20;
    }
    if(row == (unsigned int)3) {
	*GPIO_D_ODR_HIGH = 0x40;
    }
    if(row == (unsigned int)4) {
	*GPIO_D_ODR_HIGH = 0x80;
    }
    if(row == (unsigned int)0) {
	*GPIO_D_ODR_HIGH = 0x00;
    }
}

int kbdGetCol(void)
{ /* Om någon tangent (i aktiverad rad)
   * är nedtryckt, returnera dess kolumnnummer,
   * annars, returnera 0 */
    unsigned char c;
    c = *GPIO_D_IDR_HIGH;
    if(c & 0x8)
	return 4;
    if(c & 0x4)
	return 3;
    if(c & 0x2)
	return 2;
    if(c & 0x1)
	return 1;
    return 0;
}

unsigned char keyb(void)
{
    unsigned char key[] = { 1, 2, 3, 0xA, 4, 5, 6, 0xB, 7, 8, 9, 0xC, 0xE, 0, 0xF, 0xD };
    int row, col;
    for(row = 1; row <= 4; row++) {
	kbdActivate(row);
	if((col = kbdGetCol())) {
	    kbdActivate(0);
	    return key[4 * (row - 1) + (col - 1)];
	}
    }
    kbdActivate(0);
    return 0xFF;
}

// initializers
void app_init()
{

    *GPIO_D_OSPEEDR = 0x55555555;
    *GPIO_D_MODER = 0x55005555;
    *GPIO_E_MODER = 0x55555555;
}

// Game code
void draw_map()
{
    LINE lines[] = {
	{ 128 / 3, 0, 128 / 3, 64 },
	{ 128 - 128 / 3, 0, 128 - 128 / 3, 64 },
	{ 0, 64 / 3, 128, 64 / 3 },
	{ 0, 64 - 64 / 3, 128, 64 - 64 / 3 },
    };
    for(int i = 0; i < sizeof(lines) / sizeof(LINE); i++) {
	draw_line(&lines[i]);
    }
}

static GEOMETRY mole_geometry = { 41, 12, 10,
    {
        { 0, 3 },
        { 0, 4 },
        { 0, 5 },
        { 0, 6 },
        { 0, 7 },
        { 1, 2 },
        { 1, 8 },
        { 2, 1 },
        { 2, 9 },
        { 3, 0 },
        { 3, 6 },
        { 3, 10 },
        { 4, 0 },
        { 4, 3 },
        { 4, 7 },
        { 4, 10 },
        { 5, 0 },
        { 5, 7 },
        { 5, 10 },
        { 6, 0 },
        { 6, 7 },
        { 6, 10 },
        { 7, 0 },
        { 7, 7 },
        { 7, 10 },
        { 8, 0 },
        { 8, 3 },
        { 8, 7 },
        { 8, 10 },
        { 9, 0 },
        { 9, 6 },
        { 9, 10 },
        { 10, 1 },
        { 10, 9 },
        { 11, 2 },
        { 11, 8 },
        { 12, 3 },
        { 12, 4 },
        { 12, 5 },
        { 12, 6 },
        { 12, 7 },
    } };

void draw_moleobject(POBJECT obj)
{
    GEOMETRY g = *((*obj).geo);
    for(int i = 0; i < g.numpoints; i++) {
	int x = g.px[i].x;
	int y = g.px[i].y;
	graphic_pixel_set(x + (*obj).posx, y + (*obj).posy);
    }
}
void clear_moleobject(POBJECT obj)
{
    GEOMETRY g = *((*obj).geo);
    for(int i = 0; i < g.numpoints; i++) {
	int x = g.px[i].x;
	int y = g.px[i].y;
	graphic_pixel_clear(x + (*obj).posx, y + (*obj).posy);
    }
}

void move_moleobject(POBJECT o)
{

    clear_moleobject(o);
    int x = o->posx + o->dirx;
    int y = o->posy + o->diry;

    o->posx = x;
    o->posy = y;
    draw_moleobject(o);
}

void set_moleobject_speed(POBJECT o, int speedx, int speedy)
{
    o->posx = speedx;
    o->posy = speedy;
}

static OBJECT moleobject = {
    &mole_geometry,
    0,
    0,
    0,
    0,
    draw_moleobject,
    clear_moleobject,
    move_moleobject,
    set_moleobject_speed,
};

static char Points = 0; 

void int_to_str(int num, char *str) {
    int i = 0;
    int is_negative = 0;

    // handle negative numbers
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    // extract each digit and store it in the string buffer
    do {
        str[i++] = num % 10 + '0';
        num /= 10;
    } while (num > 0);

    // add a negative sign if necessary
    if (is_negative) {
        str[i++] = '-';
    }

    // add a null terminator to the end of the string
    str[i] = '\0';

    // reverse the string to put the digits in the correct order
    int len = i;
    for (i = 0; i < len / 2; i++) {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }
}


// Randomizer functions

void timer6_init()

{

	*TIM6_CR1 &= ~CEN; //Stoppa räknarmodul

	*TIM6_ARR = 0xFFFF; //Sätt räknarregister till nio

	*TIM6_CR1 |= (CEN | UDIS);

	*TIM6_CR1 |= CEN; //starta räknarmodul

	}
	
	
	char randomDigit(char RandomNumber) {
		// max number can be 255
		char returnedValue = 1;
		
		if (RandomNumber > 26)
			returnedValue = 1;
		if (RandomNumber >= 51)
			returnedValue = 2;
		if (RandomNumber >= 77)
			returnedValue = 3;	
		if (RandomNumber >= 102)
			returnedValue = 4;	
		if (RandomNumber >= 128)
			returnedValue = 5;	
		if (RandomNumber >= 153)
			returnedValue = 6;	
		if (RandomNumber >= 179)
			returnedValue = 7;	
		if (RandomNumber >= 204)
			returnedValue = 8;	
		if (RandomNumber >= 230)
			returnedValue = 9;
		
		return returnedValue;
	}



static char mLocation;
void newRound(POBJECT o, int givePoint, int startOfRound)
{
	
	o->clear(o);
	waitForClear();
    ascii_write_cmd(1);
	delay_micro(500);
    if(givePoint) {
	Points++;
	write_string("You got the mole! +1");
    } else {
	if (!startOfRound)
	write_string("You missed the mole");
	}
	// using the speed function to set the pos
	mLocation = randomDigit((char)*TIM6_CNT);
	switch(mLocation) {
		case 1:
		o->set_speed(o,13,5);  
		break;
		case 2:
		o->set_speed(o,58,5);  
		break;
		case 3:
		o->set_speed(o,103,5);  
		break;
		case 4:
		o->set_speed(o,13,27);  		
		break;
		case 5:
		o->set_speed(o,59,27);  		
		break;
		case 6:
		o->set_speed(o,102,27); 
		break;
		case 7:
		o->set_speed(o,13,48);  		
		break;
		case 8:
		o->set_speed(o,58,48);  		
		break;
		case 9:
		o->set_speed(o,98,48);  		
		break;
	}
    delay_micro(1000);
	waitForClear();
    ascii_write_cmd(1);
	o->move(o);
}

void main(void)
{
	graphic_initialize();
    graphic_clear_screen();
    app_init();
    ascii_init();
    draw_map();
	timer6_init();
    POBJECT m = &moleobject;
    char c;
	newRound(m,0,1);	
	mLocation = randomDigit((char)*TIM6_CNT);
	int refreshCount = 100;
	int gameRound = 40;
	int difficulty = 10;
	
	int count = 0;
	int gameCount = 0;
	gameStart:
	draw_map();
    while(1) {
		c = keyb();
		if (mLocation == c){
			newRound(m,1,0);
			count = 0;
		}
		delay_micro(refreshCount);
		
		if (count > difficulty) {
			newRound(m,0,0);
			count = 0;
		}
		
		if (gameCount > gameRound) {
			graphic_clear_screen();
			waitForClear();
			ascii_write_cmd(1);
			delay_micro(2000);
			write_string("Game over; you got");
			ascii_gotoxy(1,2);
			char result[20];
			int_to_str(Points,result);
			write_string(result);
			delay_micro(3000);
			waitForClear();
			ascii_write_cmd(1);
			delay_micro(2000);
			write_string("To restart, press 0");
			while (1) {
				c = keyb();
				if (c==0) {
					count = 0;
					gameCount = 0;
					goto gameStart;
				}
			}
		}
		
		count++;
		gameCount++;
    }
}
