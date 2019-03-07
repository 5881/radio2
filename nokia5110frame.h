/*
 * Библиотека работы с монохромным дисплеем nokia5110
 * v 1.0
 * 3 feb 2019
 * v1.1
 * Добавлено рисование линий
 * 11 feb 2019
 * Alexander Belyy
 * v1.11
 * Добавлена запись строки байтов в буфер с указанной позиции
 * 3 марта 2019
 * /



/**********************************************************************
 * Здесь определяем пины дисплея и SPI
 **********************************************************************/
#define SPI SPI1
#define SPI_PORT GPIOA
#define MOSI GPIO7
#define CS GPIO6
#define SCK GPIO5
#define RST GPIO3
#define DC GPIO4
/**********************************************************************
 * Здесь определяем константы
 **********************************************************************/
#define DATA 1
#define CMD 0
#define X 84
#define Y 48

struct LCD {
	unsigned char framebufer[X*Y/8];
	uint16_t pos;
} lcd;

static const char rflevel_const[14]={1,2,4,0xff,4,2,0x81,
	0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff};
static const char vollevel_const[13]={0x3c,0x3c,0x42,0x81,
	0xff,0x0,0x18,0x42,0x3c,0x81,0x7e,0x0,0x7e};

static const char ASCII[][5] =
{
 {0x00, 0x00, 0x00, 0x00, 0x00} // 20  
,{0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
,{0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
,{0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
,{0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
,{0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
,{0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
,{0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
,{0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
,{0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
,{0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
,{0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
,{0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
,{0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
,{0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
,{0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
,{0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
,{0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
,{0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
,{0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
,{0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
,{0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
,{0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
,{0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
,{0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
,{0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
,{0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
,{0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
,{0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
,{0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
,{0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
,{0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
,{0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
,{0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
,{0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
,{0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
,{0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
,{0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
,{0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
,{0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
,{0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
,{0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
,{0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
,{0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
,{0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
,{0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
,{0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
,{0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
,{0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
,{0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
,{0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
,{0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
,{0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
,{0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
,{0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
,{0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
,{0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
,{0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
,{0x02, 0x04, 0x08, 0x10, 0x20} // 5c ¥
,{0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
,{0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
,{0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
,{0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
,{0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
,{0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
,{0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
,{0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
,{0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
,{0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
,{0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
,{0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
,{0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
,{0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j 
,{0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
,{0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
,{0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
,{0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
,{0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
,{0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
,{0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
,{0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
,{0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
,{0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
,{0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
,{0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
,{0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
,{0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
,{0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
,{0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
,{0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
,{0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
,{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
,{0x10, 0x08, 0x08, 0x10, 0x08} // 7e ←
,{0x78, 0x46, 0x41, 0x46, 0x78} // 7f →
};

/*********************************************************************
 * Секция основных функций
 *********************************************************************/


void drawLine(unsigned char,unsigned char,unsigned char, unsigned char);
void draw_circle(unsigned char, unsigned char,unsigned char);
void draw_rectangle(unsigned char,unsigned char,
										unsigned char,unsigned char);
void draw_pixel(unsigned char x, unsigned char y);
void setpos_xy(unsigned char x,unsigned char y);
void s_at_pos(unsigned char *, unsigned char,
										unsigned char,unsigned char);
void clear_at_pos(unsigned char,unsigned char, unsigned char);
void bufer_clear(void);
void bufer_char(char);
void bufer_char_x2(char);
void lcdstr_at(char *,unsigned char,unsigned char);
void lcdstrx2_at(char *,unsigned char, unsigned char);
void lcdinit(void);
void lcdsend(char, char);
void lcdclear(void);
void bufer_send(void);
void img_send( unsigned char *);



/**********************************************************************
 * рисование линий с помощью  Алгоритм Брезенхэма
 * https://ru.wikibooks.org/
 **********************************************************************/

void drawLine(unsigned char x1, unsigned char y1,
							unsigned char x2, unsigned char y2){
    char deltax,deltay,signx,signy;
    deltax = x2 - x1;
    if(deltax<0) deltax*=-1;
    deltay = y2 - y1;
    if(deltay<0) deltay*=-1;
    signx = x1 < x2 ? 1 : -1;
    signy = y1 < y2 ? 1 : -1;
    //
    short int error = deltax - deltay, error2;
    //
    draw_pixel(x2, y2);
		while(x1 != x2 || y1 != y2){
			draw_pixel(x1, y1);
			error2 = error * 2;
			//
			if(error2 > -deltay){
				error -= deltay;
				x1 += signx;
				}
			if(error2 < deltax){
				error += deltax;
				y1 += signy;
				}
		}

	}


void draw_circle(unsigned char x0, unsigned char y0,
											unsigned char radius) {
	short int x = 0;
	short int y = radius;
	short int delta = 1 - 2 * radius;
	short int error = 0;
	while(y >= 0) {
		draw_pixel(x0 + x, y0 + y);
		draw_pixel(x0 + x, y0 - y);
		draw_pixel(x0 - x, y0 + y);
		draw_pixel(x0 - x, y0 - y);
		error = 2 * (delta + y) - 1;
		if(delta < 0 && error <= 0) {
			++x;
			delta += 2 * x + 1;
			continue;
			}
		error = 2 * (delta - x) - 1;
		if(delta > 0 && error > 0) {
			--y;
			delta += 1 - 2 * y;
			continue;
			}
		++x;
		delta += 2 * (x - y);
		--y;
		}
	}
	
	
void draw_rectangle(unsigned char x, unsigned char y,unsigned char dx,
										unsigned char dy){
	drawLine(x,y,x+dx,y);										
	drawLine(x+dx,y,x+dx,y+dy);
	drawLine(x,y,x,y+dy);
	drawLine(x,y+dy,x+dx,y+dy);					
	}



void draw_pixel(unsigned char x, unsigned char y){
	uint16_t byte;
	unsigned char bit;
	byte=x+y/8*84;
	bit=y%8;
	lcd.framebufer[byte]|=(1<<bit);
	}

void setpos_xy(unsigned char x,unsigned char y){
	lcd.pos=x*6+y*84;
	}
	
void s_at_pos(unsigned char *str, unsigned char len,
								unsigned char xpix, unsigned char y){
	lcd.pos=xpix+y*84;								
	if(lcd.pos>497) lcd.pos=0;
	while(len--) lcd.framebufer[lcd.pos++]=*str++;
	} 
	
void clear_at_pos(unsigned char len,
								unsigned char xpix, unsigned char y){
	lcd.pos=xpix+y*84;								
	if(lcd.pos>497) lcd.pos=0;
	while(len--) lcd.framebufer[lcd.pos++]=0;
	} 

void bufer_clear(void){
	for(uint16_t i=0;i<504;i++) lcd.framebufer[i]=0;
	}

void bufer_char(char c){
	if(lcd.pos>497) lcd.pos=0;
	//lcd.framebufer[lcd.pos++]=0;
	unsigned char temp=c-0x20;
	for (int i=0; i<5; i++) lcd.framebufer[lcd.pos++]=ASCII[temp][i];
	lcd.framebufer[lcd.pos++]=0;
	if(lcd.pos>503) lcd.pos=0;
	}
void bufer_char_x2(char c){
	//масшбабированный в 2 раза шрифт.
	unsigned char temp=c-0x20,a,b;
	uint16_t ascii;
	for(unsigned char i=0;i<5;i++){
		for(unsigned j=0;j<8;j++){
			ascii<<=2;
			if(ASCII[temp][i]&(128>>j)) ascii|=3;//128=1<<7, 3=0b11;
			}
		for(unsigned char k=0;k<2;k++){
			lcd.framebufer[lcd.pos++]=ascii&0xff;
			if(lcd.pos>503-84) lcd.pos=0;
			lcd.framebufer[lcd.pos+83]=ascii>>8;
			}
	} 
	lcd.framebufer[lcd.pos++]=0;
	lcd.framebufer[lcd.pos+83]=0;
}
	
void lcdstr_at(char *c,unsigned char x,unsigned char y){
	setpos_xy(x,y);
	while(*c){
		if(*c=='\n'){y++;c++;setpos_xy(0,y);}
		bufer_char(*c++);
		}
	}

void lcdstrx2_at(char *c,unsigned char x, unsigned char y){
	setpos_xy(x,y);
	while(*c){
		if(*c=='\n'){y+=2;c++;setpos_xy(0,y);}
		bufer_char_x2(*c++);
		}
	}

	
void lcdinit(void){
	gpio_clear(SPI_PORT,CS);
	gpio_clear(SPI_PORT, RST);
	for (unsigned char i=0;i<70;i++)
	gpio_set(SPI_PORT,RST);
	lcdsend(CMD, 0x21 );  // LCD Extended Commands. 
	lcdsend(CMD, 0xb9 );  // Set LCD Vop (Contrast). Здесь константа в оригинале была B1 (c)flanker 
	lcdsend(CMD, 0x04 );  // Set Temp coefficent. //0x04
	lcdsend(CMD, 0x14 );  // LCD bias mode 1:48. //0x13
	lcdsend(CMD, 0x20 );  // LCD Basic Commands
	lcdsend(CMD, 0x0C );  // LCD in normal mode.
	lcdsend(CMD, 0x40 );  //x=0
	lcdsend(CMD, 0x80 );  //y=0
	}

void lcdsend(char mode, char data){
	gpio_clear(SPI_PORT,CS);
	if (mode)gpio_set(SPI_PORT, DC); else gpio_clear(SPI_PORT,DC);
	spi_send8(SPI, data);
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(SPI_PORT,CS);
	}
void lcdclear(void){
	gpio_clear(SPI_PORT,CS);
	gpio_set(SPI_PORT, DC);
	for (int i=0; i<X*Y/8; i++) spi_send8(SPI, 0);
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(SPI_PORT,CS);
	}

void bufer_send(void){
	gpio_clear(SPI_PORT, CS);
	gpio_set(SPI_PORT, DC);
	for(uint16_t i=0;i<504;i++) spi_send8(SPI, lcd.framebufer[i]);
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(SPI_PORT,CS);
	}	
void img_send( unsigned char *img){
	gpio_clear(SPI_PORT, CS);
	gpio_set(SPI_PORT, DC);
	for(uint16_t i=0;i<504;i++) spi_send8(SPI, *img++);
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(SPI_PORT, CS);
	}
