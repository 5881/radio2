/* Библиотека работы с чипом rda5807fp это как tea5767 тока круче
 * v0.99
 * наконец поправил рандомное чтение. 3 марта 2019
 */
#include <string.h>
#define RDAAR 0x11 //адрес для чтения записи произвольных регистров
#define RDAAS 0x10 //адрес для последовательного чтения с 02h
					// или записи с 0Ah
#define I2C I2C1
					
					
/******************************************************************
 * Секция обявления переменных
 ******************************************************************/
uint16_t rda_read[6],
		rda_write[6]={0xC005,0x20d8,0x0100,0x84DA,0x4000,0x0000},
		rda_default[6]={0xC005,0x20d8,0x0100,0x84D1,0x4000,0x0000};
char rds[9]={0,0,0,0,0,0,0,0,0};

/******************************************************************
 * Секция функций
 ******************************************************************/
static void rda_writeall(uint16_t *);
static void rda_writerand(unsigned char, uint16_t);
static void rda_readall(uint16_t *);
static void rda_getrds(void);
unsigned char rda_volume_get(void);
static void rda_volume(unsigned char);
uint16_t rda_readrand(unsigned char);
uint16_t rda_getf_w(void);
uint16_t rda_getf(void);
uint16_t rda_read_f(void);
static void rda_mono(void);
static void rda_rds(void);
static void rda_setf(uint16_t);
static void forvard(void);
static void revers(void);
static void vol_inc(void);
static void vol_dec(void);



static void rda_writeall(uint16_t *data){
	unsigned char temp[12],x=0;
	for(unsigned char i=0; i<6;i++){
		temp[x++]=data[i]>>8;
		temp[x++]=data[i]&0xff;
		}
	i2c_transfer7(I2C,RDAAS,temp,12,0,0);
	}

static void rda_writerand(unsigned char addr, uint16_t data){
	unsigned char temp[3];
	temp[0]=addr;
	temp[1]=data>>8;
	temp[2]=data&0xff;
	i2c_transfer7(I2C,RDAAR,temp,3,0,0);
	}
	
static void rda_readall(uint16_t *data){
	unsigned char temp[12],x=0;
	i2c_transfer7(I2C,RDAAS,0,0,temp,12);
	for(unsigned char i=0; i<6;i++){
		data[i]=temp[x++]<<8;
		data[i]|=temp[x++];
		};
	}
static void rda_getrds(void){
	//Для начала напишу анилиз RDS как самое вкусное
	unsigned char rdssync=0,rds_gr_type=0,noerror=0,index=0;
	char ch1,ch2;
	//if rdssynchro = 1 rds decoder syncrhonized
	if(rda_read[0]&1<<12) rdssync=1;
	//Если станция не держит RDS то затираем строку
	if(!rdssync)strcpy(rds,"        ");
	//проверяем отсутствие ошибок в данных
	//ошибки можно исправлять но пока этого делать не будем
	if((rda_read[1]&0b11)<2) noerror=1;
	//определяем тип пакета rda_read[3] - blockB
	rds_gr_type=0x0A|((rda_read[3]&0xF000)>>8)|((rda_read[3]&0x0800)>>11);
	
	if(noerror && rdssync && rds_gr_type==0xA){
			index=rda_read[3]&0b11; //множитель 2 потому что пишем по 2
			index<<=1;				//байта
			ch1=rda_read[5]>>8;
			ch2=rda_read[5]&0xff;
			if(ch1>0x1f && ch1<0x80) rds[index++]=ch1;
			if(ch2>0x1f && ch2<0x80) rds[index]=ch2;
		}
		rds[8]=0;//Обязательно занулять строку на всяк случай.
	}

//void rda_rds(){
//	rda_write
//}

uint16_t rda_readrand(unsigned char addr){
	unsigned char temp[2];
	uint16_t data=0;
	//unsigned char REG=0xA;
	i2c_transfer7(I2C,RDAAR,&addr,1,temp,2);
	data=temp[0]<<8;
	data|=temp[1]&0xff;
	return data; 
	}

unsigned char rda_volume_get(void){
	return (unsigned char)(rda_write[3]&0x0f);
	}

static void rda_volume(unsigned char vol){
	if(vol>15) vol=15;
	rda_write[3]&=~(0b1111);
	rda_write[3]|=vol;
	rda_writerand(5,rda_write[3]);
	}

uint16_t rda_getf_w(void){//Читает частоту из массива для записи в чипом
	uint16_t data;		  //поэтому w
	data=(rda_write[1]>>6)&0x3FF;
	return data+760;
	}
	
uint16_t rda_getf(void){ //Читает частоту из прочитанного массива
	uint16_t data;		 //из чипа
	data=rda_read[0]&0x3FF;
	return data+760;
	}
	
uint16_t rda_read_f(void){//функция непосредственно считывает частоту
	uint16_t data;		  //из rda5807 регистр AH[9:0]
	data=rda_readrand(0x0A)&0x3ff;//0x3ff -маска частоты
	return data+760;
	}	
	
static void rda_mono(void){
	rda_write[0]^=1<<13;// 1 - force mono
	rda_writerand(1,rda_write[0]);	
	}
	
static void rda_rds(void){//Включить RDS
	rda_write[0]|=1<<3;// 1 - rds en;
	rda_writerand(2,rda_write[0]);	
	}

//Старая версия функции
//static void rda_setf(uint16_t f){
//	unsigned char band;
//	uint16_t chanel;
//	chanel=f-760;
//	band=0b10;
//	rda_write[1]=(chanel<<6)|(band<<2)|(1<<4);
//	/* 03H
//	 * [15-6] - chanal
//	 * bit 4  - tune
//	 * [3-2]  - band
//	 * [1-0]  - step 0b00 - 100кгц
//	 */
//	rda_writerand(3,rda_write[1]);
//	//rda_write[1]&=~(1<<4);
//	}


static void rda_setf(uint16_t f){
	uint16_t chanel;
	chanel=f-760;
	chanel=(chanel<<6)|0b11000;
	/* 03H
	 * [15-6] - chanal
	 * bit 4  - tune
	 * [3-2]  - band  0b11 - 76-108Mhz
	 * [1-0]  - step 0b00 - 100кгц
	 */
	rda_writerand(3,chanel);
	}


static void forvard(void){ //Функция перестройки частоты +100кгц
	uint16_t temp;
	temp=rda_getf();
	temp++;
	rda_setf(temp);
	}
	
static void revers(void){ //Функция перестройки частоты -100кгц
	uint16_t temp;
	temp=rda_getf();
	temp--;
	rda_setf(temp);
	}

static void vol_inc(void){
	unsigned char temp;
	temp=rda_volume_get();
	temp++;
	rda_volume(temp);
}

static void vol_dec(void){
	unsigned char temp;
	temp=rda_volume_get();
	if(temp)temp--;
	rda_volume(temp);
}

