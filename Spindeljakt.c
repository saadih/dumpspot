/*
 *
 *
 * init_app();
graphic_initialize();
graphic_clear_screen();

while (1) {
        draw_polygon(&pg1);
        delay_milli(500);
        graphic_clear_screen();
        delay_milli(500);
}
 *
 */

#define PORT_BASE ((volatile unsigned int*)0x40021000)

#define portModer ((volatile unsigned int*)PORT_BASE)
#define portOtyper ((volatile unsigned short*)PORT_BASE + 0x04)
#define portOspeedr ((volatile unsigned int*)PORT_BASE + 0x08)
#define portPupdr ((volatile unsigned int*)PORT_BASE + 0x0C)

#define portIdrLow ((volatile unsigned char*)PORT_BASE + 0x10)
#define portIdrHigh ((volatile unsigned char*)PORT_BASE + 0x11)

#define portOdrLow ((volatile unsigned char*)PORT_BASE + 0x14)
#define portOdrHigh ((volatile unsigned char*)PORT_BASE + 0x15)

#define GPIO_ODR ((volatile unsigned short*)0x40020C14)
#define STK_CTRL ((volatile unsigned int*)(0xE000E010))
#define STK_LOAD ((volatile unsigned int*)(0xE000E014))
#define STK_VAL ((volatile unsigned int*)(0xE000E018))

#define GPIO_ODR_HIGH ((volatile unsigned char*)0x40020C15)
#define GPIO_IDR ((volatile unsigned short*)0x40020C10)
#define GPIO_IDR_HIGH ((volatile unsigned char*)0x40020C11)
#define GPIO_ODR ((volatile unsigned short*)0x40020C14)

// Masker för kontrollbitar
#define B_E 0x40   /* Enable-signal */
#define B_SELECT 4 /* Välj ASCII-display */
#define B_RW 2     /* 0=Write, 1=Read */
#define B_RS 1     /* 0=Control, 1=Data */
#define MAX_POINTS 30

    typedef struct {
    char x, y;
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

__attribute__((naked)) __attribute__((section(".start_section"))) void startup(void)
{
    __asm__ volatile(" LDR R0,=0x2001C000\n"); /* set stack */
    __asm__ volatile(" MOV SP,R0\n");
    __asm__ volatile(" BL main\n");   /* call main */
    __asm__ volatile(".L1: B .L1\n"); /* never return */
}

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
__attribute__((naked)) void graphic_pixel_clear(int x, int y)
{
    __asm volatile(" .HWORD 0xDFF3\n");
    __asm volatile(" BX LR\n");
}
__attribute__((naked)) void graphic_pixel_set(int x, int y)
{
    __asm volatile(" .HWORD 0xDFF2\n");
    __asm volatile(" BX LR\n");
}

void draw_line(PLINE p)
{
    int dx = p->p1.x - p->p0.x;
    int dy = p->p1.y - p->p0.y;
    int step_x = dx < 0 ? -1 : 1;
    int step_y = dy < 0 ? -1 : 1;
    dx = abs(dx);
    dy = abs(dy);

    int x = p->p0.x;
    int y = p->p0.y;

    if(dx >= dy) {
	int d = 2 * dy - dx;
	while(x != p->p1.x) {
	    graphic_pixel_set(x, y);
	    if(d > 0) {
		y += step_y;
		d -= 2 * dx;
	    }
	    d += 2 * dy;
	    x += step_x;
	}
    } else {
	int d = 2 * dx - dy;
	while(y != p->p1.y) {
	    graphic_pixel_set(x, y);
	    if(d > 0) {
		x += step_x;
		d -= 2 * dy;
	    }
	    d += 2 * dx;
	    y += step_y;
	}
    }
}

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

void delay_milli(int m)
{
    for(int i = 0; i < m * 1000; i++) {
	delay_250ns();
	delay_250ns();
	delay_250ns();
	delay_250ns();
    }
}

void init_app(void)
{
    // port D medium speed
    *((volatile unsigned int*)0x40020C08) = 0x55555555;
    // GPIOD_MODER
    *((unsigned long*)0x40020C00) = 0x55005555;
    // GPIOD_OTYPER
    *((unsigned short*)0x40020C04) = 0x00000000;
    // GPIOD_PUPDR
    // * ( (unsigned int *) 0x40020C0C) = 0x00660000;
}

void kbdActivate(unsigned int row)
{ /* Aktivera angiven rad hos tangentbordet, eller
   * deaktivera samtliga */

    if(row == (unsigned int)1) {
	*GPIO_ODR_HIGH = 0x10;
    }

    if(row == (unsigned int)2) {
	*GPIO_ODR_HIGH = 0x20;
    }
    if(row == (unsigned int)3) {
	*GPIO_ODR_HIGH = 0x40;
    }
    if(row == (unsigned int)4) {
	*GPIO_ODR_HIGH = 0x80;
    }
    if(row == (unsigned int)0) {
	*GPIO_ODR_HIGH = 0x00;
    }
}

int kbdGetCol(void)
{ /* Om någon tangent (i aktiverad rad)
   * är nedtryckt, returnera dess kolumnnummer,
   * annars, returnera 0 */
    unsigned char c;
    c = *GPIO_IDR_HIGH;
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


int objects_overlap(POBJECT o1, POBJECT o2)
{
    int o1_x = o1->posx;
    int o1_y = o1->posy;
    int o1_size_x = o1->geo->sizex;
    int o1_size_y = o1->geo->sizey;
    int o2_x = o2->posx;
    int o2_y = o2->posy;
    int o2_size_x = o2->geo->sizex;
    int o2_size_y = o2->geo->sizey;
    
    if (o1_x < o2_x + o2_size_x &&
        o1_x + o1_size_x > o2_x &&
        o1_y < o2_y + o2_size_y &&
        o1_y + o1_size_y > o2_y) {
        return 1; 
    }
    
    return 0;
}

int pixel_overlap(POBJECT o1, POBJECT o2) {
  int offset1x = o1->posx;
  int offset1y = o1->posy;
  int offset2x = o2->posx;
  int offset2y = o2->posy;
  for (int i = 0; i < o1->geo->numpoints; i++) {
    for (int j = 0; j < o2-> geo->numpoints; j++)
      if ((offset1x + o1->geo->px[i].x == offset2x + o2->geo->px[j].x) &&
        (offset1y + o1->geo->px[i].y == offset2y + o2->geo->px[j].y)) return 1;
  }
  return 0;
}
void draw_ballobject(POBJECT obj)
{
    GEOMETRY g = *((*obj).geo);
      for (int i = 0 ; i < g.numpoints; i++ ) {
	   int x = g.px[i].x;
	   int y = g.px[i].y;
		graphic_pixel_set(x + (*obj).posx, y + (*obj).posy);
   }
}
void clear_ballobject(POBJECT obj)
{
    GEOMETRY g = *((*obj).geo);
    for(int x = 0; x < g.sizex; x++) {
	for(int y = 0; y < g.sizey; y++) {
	    graphic_pixel_clear(x + (*obj).posx, y + (*obj).posy);
	}
    }
}

void move_ballobject(POBJECT o)
{

    clear_ballobject(o);
    int x = o->posx + o->dirx;
    int y = o->posy + o->diry;
    if(x < 1) {
	o->dirx *= -1;
	x = o->posx + o->dirx;
    }
	if(x + (o->geo)->sizex > 128) {
	o->dirx *= -1;
	x = o->posx + o->dirx;
    }
    if(y < 1) {
	o->diry *= -1;
	y = o->posy + o->diry;
    }
    if(y + (o->geo)->sizey > 64) {
	o->diry *= -1;
	y = o->posy + o->diry;
    }
    o->posx = x;
    o->posy = y;
    draw_ballobject(o);
}

void set_ballobject_speed(POBJECT o, int speedx, int speedy)
{
    o->dirx = speedx;
    o->diry = speedy;
}

GEOMETRY ball_geometry = { 12, 4, 4,
    {
        { 0, 1 },
        { 0, 2 },
        { 1, 0 },
        { 1, 1 },
        { 1, 2 },
        { 1, 3 },
        { 2, 0 },
        { 2, 1 },
        { 0, 1 },
        { 2, 3 },
        { 3, 1 },
        { 3, 2 },
    } };

static OBJECT ball = { &ball_geometry, 4, 1, 0, 0, draw_ballobject, clear_ballobject, move_ballobject,
    set_ballobject_speed };

void draw_spindelobject(POBJECT obj)
{
    GEOMETRY g = *((*obj).geo);
    for (int i = 0 ; i < g.numpoints; i++ ) {
	   int x = g.px[i].x;
	   int y = g.px[i].y;
		graphic_pixel_set(x + (*obj).posx, y + (*obj).posy);
   }
}
void clear_spindelobject(POBJECT obj)
{
    GEOMETRY g = *((*obj).geo);
    for(int x = 0; x < g.sizex; x++) {
	for(int y = 0; y < g.sizey; y++) {
	    graphic_pixel_clear(x + (*obj).posx, y + (*obj).posy);
	}
    }
}

void move_spindelobject(POBJECT o)
{

    clear_spindelobject(o);
    int x = o->posx + o->dirx;
    int y = o->posy + o->diry;
    if(x < 1) {
	o->dirx *= -1;
	x = o->posx + o->dirx;
    }
	if(x + (o->geo)->sizex > 128) {
	o->dirx *= -1;
	x = o->posx + o->dirx;
    }
    if(y < 1) {
	o->diry *= -1;
	y = o->posy + o->diry;
    }
    if(y + (o->geo)->sizey > 64) {
	o->diry *= -1;
	y = o->posy + o->diry;
    }
    o->posx = x;
    o->posy = y;
    draw_spindelobject(o);
}

void set_spindelobject_speed(POBJECT o, int speedx, int speedy)
{
    o->dirx = speedx;
    o->diry = speedy;
}

GEOMETRY spindel_geometry = { 22 , 6, 8,
    {
        {2,0},{3,0},{1,1},{4,1},{0,2},{1,2},{2,2},{3,2},{4,2},{5,2},{0,3},{2,3},{3,3},{5,3},{1,4},{4,4},{2,5},{3,5},{1,6},{4,6},{0,7},{5,7}
    } 
};

static OBJECT spider = { &spindel_geometry, 0, 0, 64, 32, draw_spindelobject, clear_spindelobject, move_spindelobject,
    set_spindelobject_speed };


void main(void)
{
  char c;
  POBJECT victim = &ball;
  POBJECT creature = &spider;
  init_app();
  graphic_initialize();
  graphic_clear_screen();
  victim->set_speed(victim,4,1);
  
  while(1) {
	  victim->move(victim);
	  creature->move(creature);
	  c = keyb();
	  switch(c) {
		  case 6: creature->set_speed(creature,2,0); break;
		  case 4: creature->set_speed(creature,-2,0); break;
		  case 5: creature->set_speed(creature,0,0); break;
		  case 2: creature->set_speed(creature,0,-2); break;
		  case 8: creature->set_speed(creature,0,2); break;
			default: 
				creature->set_speed(creature,0,0); 
					break;
		}
		
		if (objects_overlap(victim,creature)) {
			if (pixel_overlap(victim,creature)) {
			break;	
			}
		}
	
		delay_micro(50);
	}
}
