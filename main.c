/*
 *Прошивка для работы с rda5807 это как tea только круче
 * v0.9
 * наконец поправлена перестройка и индикация частоты
 * Добавлен RDS!!!
 * v1.0
 * 3 марта 2019
 */

/**********************************************************************
 * Секция include и defines
**********************************************************************/

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/spi.h>
#include "mini-printf.h"
#include "nokia5110frame.h"
#include "rda5807fp.h"




/**********************************************************************
 * Секция глобальных переменных
 *********************************************************************/
 

	
/**********************************************************************
 * Секция настройки переферии
**********************************************************************/

static void gpio_setup(){
	//PB0 - Подсветка lcd nokia5110
	//PA10 - Просто отладочный светодиод
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT,
					GPIO_PUPD_NONE, GPIO10);
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP,
                    GPIO_OSPEED_2MHZ, GPIO10);
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT,
					GPIO_PUPD_NONE, GPIO0);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_PP,
                    GPIO_OSPEED_2MHZ, GPIO0);
    gpio_clear(GPIOA,GPIO10);
    gpio_clear(GPIOB,GPIO0);
}

static void button_setup(void){
		/* Enable GPIOB clock. */
	rcc_periph_clock_enable(RCC_GPIOA);
	/*
	 * три кнопки с подтяжкой к питанию.
	 * PA0,PA1,PA2
	 * 
	 */
	 
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT,
					GPIO_PUPD_PULLUP, GPIO0|GPIO1|GPIO2);
	//gpio_set(GPIOA, GPIO4|GPIO5|GPIO6);
	}
	
void spi_setup(void){
	//Enable SPI1 Periph and gpio clocks
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_SPI1);
	/* Configure GPIOs:
	 * SCK PA5
	 * MOSI PA7
	 * CS PA6
	 * RST PA3
	 * DC PA4
	 */
	 //CS,RST,DC
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT,
					GPIO_PUPD_NONE, GPIO6|GPIO3|GPIO4);
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP,
                    GPIO_OSPEED_2MHZ, GPIO6|GPIO3|GPIO4);
    gpio_set(GPIOB,GPIO0|GPIO1|GPIO4);
	//MOSI,SCK
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7|GPIO5);
	gpio_set_af(GPIOA,GPIO_AF0,GPIO7|GPIO5);
	//Настройка SPI1
	spi_disable(SPI1);
	spi_reset(SPI1);
	spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_128,
					SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
					SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_MSBFIRST);
	//spi_set_crcl_8bit(SPI1); Закоментировато тк один хрен не работает
	spi_enable_software_slave_management(SPI1);
	spi_set_nss_high(SPI1);
	spi_enable(SPI1);
	}	
	
static void i2c_setup(void){
	/* Enable clocks for I2C2 and AFIO. */
	rcc_periph_clock_enable(RCC_I2C1);
	/* Set alternate functions for the SCL and SDA pins of I2C2.
	 * SDA PB7
	 * SCL PB6
	 *  */
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO7|GPIO6);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_OD,
                    GPIO_OSPEED_2MHZ, GPIO7|GPIO6);
    gpio_set_af(GPIOB,GPIO_AF1,GPIO7|GPIO6);
	
	/* Disable the I2C before changing any configuration. */
	i2c_peripheral_disable(I2C1);
	i2c_set_speed(I2C1,i2c_speed_fm_400k,48);
	i2c_peripheral_enable(I2C1);
}	




/**********************************************************************
 * Секция основных функций
 **********************************************************************/
 
static void info_send_lcd(unsigned char mode){
	/*
	 * mode
	 * 0 - set f
	 * 1 - set volume
	 * 2 -force mono
	 */
	char bufer[50];
	unsigned char s=0,vol, stereo=0, signal;
	uint16_t f,f_read;
	bufer_clear();
	rda_readall(rda_read);
	//Rds читается по два символа за раз, поэтому чем чаще его 
	//дёргаешь тем лучше.
	rda_getrds();
	switch(mode){
		case 0:
			lcdstr_at("TUNE", 0,0);
			break;
		case 1:
			lcdstr_at("VOL", 0,0);
			break;
		case 2:
			lcdstr_at("MONO",0,0);
			break;
				}
	if(rda_write[0]&(1<<13)) lcdstr_at("force mono",0,4);
	if(rda_read[0]&(1<<10)) lcdstr_at("STEREO",0,5);
	signal=rda_read[1]>>9;
	vol=rda_volume_get();
	f=rda_getf();
	f_read=rda_read_f();
	rf_level_disp(signal,40,0);// показываем уровень сигнала
	vollevel_disp(vol, 60,0);  // показываем уровень громкости
	if(f<1000) s=2; else s=0;
	snprintf(bufer,50,"%d.%d", f/10,f%10);
	lcdstrx2_at(bufer,s,2);
	lcdstr_at("MHz",10,2);
	lcdstr_at("FM",10,3);
	lcdstr_at(rds,2,4);
	bufer_send();
	}

void rf_level_disp(unsigned char signal,
							unsigned char x,unsigned char y){
	unsigned char temp;
	temp=signal/8+6; //6 -длинна символа антены в массиве rflevel_const
	clear_at_pos(14,x,y);
	s_at_pos(rflevel_const,temp,x,y);
	}

void vollevel_disp(unsigned char vol,
							unsigned char x,unsigned char y){
	unsigned char temp=5;//5 -длинна символа антены в массиве vollevel_const
	if(vol) temp+=2;
	if(vol>3)  temp+=2;
	if(vol>10) temp+=2;
	if(vol==15) temp+=2;
	clear_at_pos(11,x,y);
	s_at_pos(vollevel_const,temp,x,y);
	}


void int16_to_binstr(char *str, uint16_t data){
	for(unsigned char i=0;i<16;i++){
		if(data&(1<<(15-i))) str[i]='1'; else str[i]='0';
		}
		str[16]=0;
	}
void int8_to_binstr(char *str, uint8_t data){
	for(unsigned char i=0;i<8;i++){
		if(data&(1<<(7-i))) str[i]='1'; else str[i]='0';
		}
		str[8]=0;
	}

rda_info(unsigned char reg){
	char buf[50],hi[9],lo[9];
	rda_readall(rda_read);
	int8_to_binstr(hi,rda_read[reg]>>8);
	int8_to_binstr(lo,rda_read[reg]&0xff);
	switch(reg){
		case 0:
			snprintf(buf,50,"AH h:%s\nAH l:%s",hi,lo);
			break;
		case 1:
			snprintf(buf,50,"BH h:%s\nBH l:%s",hi,lo);
			break;
		case 2:
			snprintf(buf,50,"CH h:%s\nCH l:%s",hi,lo);
			break;
		case 3:
			snprintf(buf,50,"DH h:%s\nDH l:%s",hi,lo);
			break;
		case 4:
			snprintf(buf,50,"EH h:%s\nEH l:%s",hi,lo);
			break;
		case 5:
			snprintf(buf,50,"FH h:%s\nFH l:%s",hi,lo);
			break;
		}
		
	lcdstr_at(buf,0,4);
	bufer_send();
}


void rda_info_rand(){
	uint16_t temp;
	char buf[50],hi[9],lo[9];
	temp=rda_readrand(0x0a);
	int8_to_binstr(hi,temp>>8);
	int8_to_binstr(lo,temp&0xff);
	snprintf(buf,50,"AH h:%s\nAH l:%s",hi,lo);
	lcdstr_at(buf,0,4);
	bufer_send();
	}


int main(void){
	rcc_clock_setup_in_hsi_out_48mhz();
	button_setup();
	gpio_setup();
	spi_setup();
	i2c_setup();
	lcdinit();
	lcdclear();
	//init command
	gpio_set(GPIOA,GPIO2|GPIO1);
	bufer_clear();
	lcdstr_at("radio v. 1.0",0,0);
	lcdstrx2_at("SHAMAN",0,2);
	lcdstr_at("3 mar. 2019",0,5);
	bufer_send();
	unsigned char mode=0;
	
	
	rda_writeall(rda_default);
	rda_volume(0);
	for(int i=0;i<0xffffff;i++)__asm__("nop");
	info_send_lcd(mode);
	rda_rds();
	while (1){
		if(!gpio_get(GPIOA, GPIO2)){
			int i=0x3ffff;
			while(!gpio_get(GPIOA, GPIO2) && i--);
			switch(mode){
				case 0:
					forvard();
					break;
				case 1:
					vol_inc();
					break;
				case 2:
					rda_mono();
					break;
				}
			info_send_lcd(mode);
			gpio_toggle(GPIOA,GPIO10);
		}
		
		if(!gpio_get(GPIOA,GPIO1)){
			int i=0x3ffff;
			while(!gpio_get(GPIOA, GPIO1) && i--);
			switch(mode){
				case 0:
					revers();
					break;
				case 1:
					vol_dec();
					break;
				case 2:
					rda_mono();
					break;
				}
			gpio_toggle(GPIOA,GPIO10);
			info_send_lcd(mode);
		}
		
		if(!gpio_get(GPIOA,GPIO0)){
			while(!gpio_get(GPIOA, GPIO0));
			mode++;
			if(mode>1) mode=0;
			
			gpio_toggle(GPIOA,GPIO10);
			info_send_lcd(mode);
		}
		
		//for(int i=0;i<0xffff;i++)__asm__("nop");
		info_send_lcd(mode);
		//rda_info(5);
		//rda_info_rand();
				
		};
		
return 0;
}
