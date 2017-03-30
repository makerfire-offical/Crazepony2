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
#include <math.h>
#include <string.h>
#include <platform.h>
#include "bus_spi.h"
#include "bus_i2c.h"
#include "system.h"
#include "gpio.h"
#include "nrf2401.h"
#include "app.h"
#include "build/debug.h"


golbal_flag flag;
dataPackage mspData;

uint8_t  TXData[TX_PLOAD_WIDTH];//tx_data
uint8_t  TX_ADDRESS[RX_ADR_WIDTH]= {0x11,0xff,0xff,0xff,0xff};//tx_address

uint8_t  RXDATA[RX_PLOAD_WIDTH];//rx_data
uint8_t  RX_ADDRESS[RX_ADR_WIDTH]= {0x11,0xff,0xff,0xff,0xff};//rx_address


static inline void NRF_Write_Reg(uint8_t reg, uint8_t data)
{
    SPI_CSN_L();
    spiTransferByte(SPI2, reg + 0x20);
    spiTransferByte(SPI2, data);
    SPI_CSN_H();
}

static inline void NRF_Write_Buf(uint8_t reg, uint8_t *data, uint8_t length)
{
    SPI_CSN_L();
    spiTransferByte(SPI2, reg + 0x20);
    spiTransfer(SPI2, NULL, data, length);
    SPI_CSN_H();
}

static inline void NRF_Read_Buf(uint8_t reg, uint8_t *data, uint8_t length)
{
    SPI_CSN_L();
    spiTransferByte(SPI2, reg); // read transaction
    spiTransfer(SPI2, data, NULL, length);
    SPI_CSN_H();
}



//NRF24L01_Data_Receive
bool nrf_rx(void)
{
    uint8_t sta;
	static uint8_t count;
    NRF_Read_Buf(NRFRegSTATUS, &sta, 1);
    if(sta & (1<<RX_DR)) {
        NRF_Read_Buf(RD_RX_PLOAD,RXDATA,RX_PLOAD_WIDTH);
		memcpy(&mspData,RXDATA,sizeof(mspData));
		NRF_Write_Reg(NRFRegSTATUS, sta);//清除nrf的中断标志位
		count = 0;	
     }else count++;

	if(count > 60) {
		count = 60;
		return false;
	}
	else return true;
		
}


void rx_data_process(int16_t *buf)
{
	static bool roll_flag,arm_flag;

	if(WIFI_DATA_OK) {

		if(App_data[4] & APP_ARM)\
			mspData.mspCmd |= ARM;

		if(App_data[4] & APP_DIS)\
			mspData.mspCmd &= ~ARM;
		
		if(App_data[4] & APP_ALT)
			mspData.mspCmd |= ALTHOLD;
		else mspData.mspCmd &= ~ALTHOLD;
		
		if(App_data[4] & APP_CAL) 
			mspData.mspCmd |= CALIBRATION;
		else mspData.mspCmd &= ~CALIBRATION;
		
		mspData.motor[0] = (App_data[0]<<2) + 988;
		mspData.motor[1] = (App_data[1]<<2) + 988;
		mspData.motor[2] = (App_data[3]<<2) + 988;
		mspData.motor[3] = (App_data[2]<<2) + 988;
	}

	if(!strcmp("$M<",(char *)mspData.checkCode) || WIFI_DATA_OK) {
		//低电压不可以解锁，开机检测遥控为解锁状态需再次解锁
		
		if(mspData.mspCmd & ARM) {
			if(roll_flag && arm_flag)	mwArm();
				else  mwDisarm();
			//侧翻超过70度上锁
			if(fabs(flag.pitch) > 700 || fabs(flag.roll) > 700) {
				roll_flag = false;
				mwDisarm();
			}
		}		
		else {
			mwDisarm();
			roll_flag = true;
			if(flag.batt < 100) arm_flag = false;
				else arm_flag = true;
		}


		if(mspData.mspCmd & CALIBRATION) {
			accSetCalibrationCycles(400);
			flag.calibration = true;
		}
	
		for(uint8_t i = 0;i<4;i++)	buf[i] = bound(mspData.motor[i],2000,1000);
	}

}

//NFR24L01初始化
bool NRF24L01_init(void)
{
	uint8_t sta;

	nrf24l01HardwareInit();

	if(NRF24L01_Check()) {
		SetRX_Mode();//default:0x11

		for(uint8_t i = 0;i<5;i++) {
			NRF_Read_Buf(NRFRegSTATUS, &sta, 1);
			delay(10);
		}

		if(sta & (1<<RX_DR)) {
	        NRF_Read_Buf(RD_RX_PLOAD,RXDATA,RX_PLOAD_WIDTH);// read receive payload from RX_FIFO buffer 
			memcpy(&mspData,RXDATA,sizeof(mspData));
			NRF_Write_Reg(NRFRegSTATUS, sta);//清除nrf的中断标志位
			if(mspData.mspCmd & NEWADDRESS) {
				RX_ADDRESS[0] = mspData.motor[2];
				RX_ADDRESS[1] = mspData.motor[2] >> 8;
				RX_ADDRESS[2] = mspData.motor[3];
				RX_ADDRESS[3] = mspData.motor[3] >> 8;

				//save new_address to flash
				FLASH_Unlock();
				FLASH_ErasePage(0x0803E800);
				FLASH_ProgramWord(0x0803E800, mspData.motor[2]);
				FLASH_ProgramWord(0x0803E820, mspData.motor[3]);
				FLASH_Lock();
				SetRX_Mode();//use the new_address!
			}
		}
		//load the address form flash!
		else {	
			RX_ADDRESS[0] = *(uint16_t *)0x0803E800;
			RX_ADDRESS[1] = *(uint16_t *)0x0803E800 >> 8;
			RX_ADDRESS[2] = *(uint16_t *)0x0803E820;
			RX_ADDRESS[3] = *(uint16_t *)0x0803E820 >> 8;

			SetRX_Mode();//use the new_address!
		}
		return true;
	}else return false;

}


bool NRF24L01_Check(void) 
{ 
	uint8_t buf = 0x77; 
   	uint8_t buf1; 
	
	NRF_Write_Buf(TX_ADDR,&buf,1); 
	delay(2);
	NRF_Read_Buf(TX_ADDR,&buf1,1); 

	if(buf1 == 0x77)return true;
		else	return false;
} 

void SetRX_Mode(void)
{
	SPI_CE_L();
	NRF_Write_Reg(FLUSH_RX,0xff);//清除TX FIFO寄存器			 
  	NRF_Write_Buf(RX_ADDR_P0,(uint8_t*)RX_ADDRESS,RX_ADR_WIDTH);//写RX节点地址
   	NRF_Write_Reg(EN_AA,0x01);       //使能通道0的自动应答    
  	NRF_Write_Reg(EN_RXADDR,0x01);   //使能通道0的接收地址  	 
  	NRF_Write_Reg(RF_CH,40);	 	//设置RF通信频率		  
  	NRF_Write_Reg(RX_PW_P0,RX_PLOAD_WIDTH);//选择通道0的有效数据宽度 	    
  	NRF_Write_Reg(RF_SETUP,0x0f);   //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	NRF_Write_Reg(CONFIG, 0x0f);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 
	SPI_CE_H();

} 


void nrf24l01HardwareInit(void)
{
	gpio_config_t IRQPIN;//nrf24l01 pins

	IRQPIN.pin = Pin_0;
	IRQPIN.mode = Mode_IN_FLOATING;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	gpioInit(GPIOB,&IRQPIN);


	gpio_config_t CE;//nrf24l01 pins

	CE.pin = Pin_1;
	CE.mode = Mode_Out_PP;
	CE.speed = Speed_10MHz;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	gpioInit(GPIOB,&CE);
}


