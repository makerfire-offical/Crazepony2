/*				                                                                                                     
                                                 oo                                        
                                                 ooo                                      
                                                 ooo                                         
                                                oooo                                          
                                               ooooo                                          
                                         o    ooooo                                           
                                       oo    oooo                                             
                                      oo     oooo                                             
                                     oo      ooo                                              
                                    ooo   o  ooo                                              
                          oo       ooo    o  ooo       oo                                     
                           ooo     ooo    o   o      ooo                                      
                            oooo  ooooo    o       ooooo                                      
                            ooooooooooo     oo    oooooo                                      
                            oooooooooooo     oo  oooooo                                       
                            oooooooooooo     oo ooooooo                                       
                            ooooooooooooo     oooooooooo                                      
                           ooooooooooooooo     ooooooooo                                      
                          ooooooooooooooooo o oooooooooo                                     
                         ooooooooooooooooooooooooooooooooo                                    
                         oooooooooooooooooooooooooooooooooo                                   
                        ooooooooooooooooooooooooooooooooooo                                   
                        oooooooooooooooooooooooooooooooooooo                                  
                       ooooooooooooooooooooooooooooooooooooo                                  
                       ooooooooooo ooooooooooooo ooooooooooo                                  
                        ooooooooo  ooooooooooooo  oooooooooo                                  
                        oooooooo    ooooooooooo    oooooooo                                   
                         ooooooo      oooooooo      oooooo                                    
                          ooooo        ooooo        ooooo                                     
                           oooo                     oooo                                      
                          ooooo   oooo       oooo   ooooo                                     
                          oooooo   ooo       ooo   oooooo                                     
                              oo                   ooo                                        
                             oooo                 ooooo                                       
                            oooooo               oooooo                                       
                             oo ooo            ooo  oo                                       
                                 ooooooooooooooooo                                            
                                  ooooooooooooooo                                             
                                   o    ooo     o                                             
                                                                                                  
                                                                                                  
oooo        oooo            ooo                               oooo  ooo                      +---+  
ooooo      ooooo            ooo   ooo                        ooo                             | R |
oooooo    oooooo   ooooooo  ooo  oooo    oooooooo  ooo  oo ooooooo  ooo  ooo  oo   oooooooo  +---+ 
ooooooo  ooo ooo      ooooo ooooooo     ooo    ooo ooo oo  ooooooo  ooo  ooo oo   ooo    ooo  
ooo  oooooo  ooo   oooooooo ooooo      ooo    oooo ooooo     ooo    ooo  ooooo   ooo    oooo  
ooo   oooo   ooo ooo    ooo ooooooo    oooooooooo  oooo      ooo    ooo  oooo    oooooooooo        
ooo   oooo   ooo ooo    ooo ooo  oooo  ooo         ooo       ooo    ooo  ooo     ooo        
ooo   oooo   ooo  oooo ooo  ooo   ooo   oooooooo   ooo       ooo    ooo  ooo      oooooooo   

Author:	吴栋(cchaoss) 、李楷模(kaimo)
E-mail: 862281335@qq.com
编译链:arm-none-eabi-gcc-4.9.3
版  本:v1.0
*/

#include "stdio.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <platform.h>
#include "system.h"
#include "gpio.h"
#include "led.h"
#include "nrf2401.h"

LED_t LEDCtrl;
LEDBuf_t LEDBuf;//接口显存

/*
Led初始化函数
Led1-->PB2
Led2-->PB3
Led3-->PB4
Led4-->PB5
*/
void LED_init(void)
{

	gpio_config_t LEDs;//init led pins

	LEDs.pin = Pin_2 | Pin_3 | Pin_4 | Pin_5 ;
	LEDs.mode = Mode_Out_PP;
	LEDs.speed = Speed_2MHz;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	gpioInit(GPIOB,&LEDs);

	//循环闪烁4次
    for(char i=0;i<4;i++) {
		LedA_on;LedB_off;LedC_off;LedD_off;
		delay(100);
		LedA_off;LedB_on;LedC_off;LedD_off;
		delay(100);
		LedA_off;LedB_off;LedC_on;LedD_off;
		delay(100);
		LedA_off;LedB_off;LedC_off;LedD_on;
		delay(100);
    }
	//解锁成功，快速闪烁3次提示
    for(char i=0;i<3;i++) {
		LedA_on;LedB_on;LedC_on;LedD_on;
		delay(100);
		LedA_off;LedB_off;LedC_off;LedD_off;
		delay(100);
    }

}

//底层更新 
void LEDReflash(void)
{
 
	if(LEDBuf.bits.A)
		LedA_on;
	else
		LedA_off;
	
	if(LEDBuf.bits.B)
		LedB_on;
	else
		LedB_off;
	
	if(LEDBuf.bits.C)
		LedC_on;
	else
		LedC_off;
	
	if(LEDBuf.bits.D)
		LedD_on;
	else
		LedD_off;

}

//事件驱动层 10Hz
void LED_loop(void)
{
	static char a,b;
	LEDCtrl.event = READY;

	if(flag.batt_low == 1)	LEDCtrl.event = BATL;
	
	if(flag.batt_low == 2)  LEDCtrl.event = LAND;
	
	if(flag.single_loss)	LEDCtrl.event = LOST;	

	if(flag.calibration) 	{
		a++;
		if(a > 8) {a = 0;flag.calibration = false;}
		LEDCtrl.event = CALI;
	}

	switch(LEDCtrl.event) {
		case READY: LEDBuf.byte = 0X0F;
					break;

		case CALI :	if(b) {b = 0;LEDBuf.byte = LA|LB|LC|LD;}
					else  {b = 1;LEDBuf.byte = LA|LB;}
					break;

		case BATL : if(++LEDCtrl.cnt >= 3)	LEDCtrl.cnt = 0;
					if(LEDCtrl.cnt == 0)	LEDBuf.byte = LA|LB;
					else	LEDBuf.byte = 0;
					break;

		case LAND : if(++LEDCtrl.cnt >= 3)	LEDCtrl.cnt = 0;
					if(LEDCtrl.cnt == 0)	LEDBuf.byte = 0x0f;
					else	LEDBuf.byte = 0;
					break;

		case LOST : if(++LEDCtrl.cnt >= 4)	LEDCtrl.cnt = 0;
					LEDBuf.byte = 1<<LEDCtrl.cnt ;
					break;	
	}

	LEDReflash();
}



