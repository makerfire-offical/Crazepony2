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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <platform.h>
#include "build/debug.h"
#include "fc/fc_tasks.h"
#include "config/parameter_group.h"
#include "drivers/dma.h"
#include "drivers/system.h"
#include "drivers/serial.h"
#include "drivers/serial_uart.h"

#include "io/serial.h"
#include "common/crc.h"
#include "rx/rx.h"
#include "app.h"


uint8_t App_data[APP_DATA_LENGTH];
bool APP_DATA_FLAG = false,WIFI_DATA_OK = false;

static void App_DataReceive(uint16_t data);

void wifi_uart_init(void)
{
	
	//openSerialPort(SERIAL_PORT_UART2,FUNCTION_TELEMETRY_MAVLINK, App_DataReceive, 19200, MODE_RX, SERIAL_NOT_INVERTED);//SERIAL_STOPBITS_1
	uartOpen(USART2,App_DataReceive,19200,MODE_RX,SERIAL_NOT_INVERTED);
}

//对串口数据进行解码
void App_DataReceive(uint16_t data)
{
	static uint8_t count, i, buffer[APP_DATA_LENGTH+1];

	switch(count)
	{
   		case 0: if(data == 0x66)	count = 1;
				break;

 		case 1: if(i < 5)	buffer[i++] = data;
					else {buffer[i] = data;i = 0;count = 2;}
				break;
		
		case 2:if(buffer[5] == ((buffer[0]^buffer[1]^buffer[2]^buffer[3]^buffer[4])&0xff)) {
					memcpy(App_data,buffer,APP_DATA_LENGTH);
					APP_DATA_FLAG = true;
				}
				count = 0;
				break;

		default:count = 0;i = 0;break;
	}

}
