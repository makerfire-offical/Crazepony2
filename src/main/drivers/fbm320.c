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
编译环境:arm-none-eabi-gcc-4.9.3
版    本:v1.0
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <platform.h>
#include "stdio.h"
#include "bus_i2c.h"
#include "system.h"
#include "common/maths.h"
#include "build/debug.h"
#include "fc/fc_tasks.h"
#include "drivers/fbm320.h"

struct Sensor FB;


#define sample_count 3
static int32_t MedianFilter(int32_t data)
{
    static int32_t FilterSamples[sample_count];
    static uint8_t i = 0;
    static bool medianFilterReady = false;
    int j;
    j = (i + 1);
    if (j == sample_count) {
        j = 0;
        medianFilterReady = true;
    }
    FilterSamples[i] = data;
    i = j;
   
    if (medianFilterReady)
        return quickMedianFilter3(FilterSamples);
    else
        return data;
}

#define SAMPLE_COUNT_MAX 21
static uint32_t recalculateBarometerTotal(uint8_t baroSampleCount, uint32_t pressureTotal, int32_t newPressureReading)
{
    static int32_t barometerSamples[SAMPLE_COUNT_MAX];
    static uint8_t currentSampleIndex = 0;
    int nextSampleIndex;

    nextSampleIndex = (currentSampleIndex + 1);
    if (nextSampleIndex == baroSampleCount)
        nextSampleIndex = 0;
    barometerSamples[currentSampleIndex] = MedianFilter(newPressureReading);

	//保持有21个样本值的和
    pressureTotal += barometerSamples[currentSampleIndex];
    pressureTotal -= barometerSamples[nextSampleIndex];

    currentSampleIndex = nextSampleIndex;
    return pressureTotal;
}

//获取气压计初始值
bool fbm320_init(void)
{
	uint8_t whoami = 0x00;
	i2cRead(FMTISensorAdd_I2C,0x6b,1,&whoami);
	if(whoami == 0x42) {
		read_offset();
		start_temperature();				 
		delay(3);																					
		FB.UT = Read_data();
		start_pressure();					 
		delay(10);																				
		FB.UP = Read_data();															
		calculate_real_pressure(FB.UP, FB.UT);
		FB.Reff_P = FB.RP;
		FB.calibrate_finished = true;
		return true;
	}
	else {
		FB.calibrate_finished = false;
		return false;
	}
}


#define SCALE 0.4
static uint32_t baroPressureSum;

//解包
void taskFbm320(void)
{
	static float alt;
	static char state = 0;
	switch(state) {
		case 0:	FB.UP = Read_data();
				start_temperature();
				state = 1;
				break;
		case 1: FB.UT = Read_data();
				start_pressure();
				calculate_real_pressure(FB.UP, FB.UT);
				baroPressureSum = recalculateBarometerTotal(SAMPLE_COUNT_MAX, baroPressureSum, FB.RP);
				alt = Rel_Altitude(baroPressureSum/(SAMPLE_COUNT_MAX-1),FB.Reff_P) * 100;//unit:cm
				FB.Altitude = SCALE*alt + (1-SCALE)*FB.Altitude;
				//debug[3] = (int16_t)alt;
				state = 0;
				break;
		default:break;
	}
}

//开始温度转换
void start_temperature(void)
{
	i2cWrite(FMTISensorAdd_I2C,0xf4, 0x2e);
}

//开始气压转换
void start_pressure(void)
{
	i2cWrite(FMTISensorAdd_I2C,0xf4, 0xf4);
}

uint32_t Read_data(void)
{
	uint8_t buf[3];
	i2cRead(FMTISensorAdd_I2C,0xf6,1,&buf[0]);
	i2cRead(FMTISensorAdd_I2C,0xf7,1,&buf[1]);
	i2cRead(FMTISensorAdd_I2C,0xf7,1,&buf[2]);
	return ((uint32_t)buf[0] << 16) | ((uint16_t)buf[1] << 8) | buf[2];
}

//读取气压计出厂校准值
void read_offset(void)										
{
	uint8_t buf[2];
	uint16_t R[10]={0};
	for(uint8_t i = 0;i < 9;i++) {
		i2cRead(FMTISensorAdd_I2C,(0xaa+i*2),1,&buf[0]);
		i2cRead(FMTISensorAdd_I2C,(0xab+i*2),1,&buf[1]);
		R[i] = ((uint8_t) buf[0] << 8) | buf[1];
	}
	i2cRead(FMTISensorAdd_I2C,0xa4,1,&buf[0]);
	i2cRead(FMTISensorAdd_I2C,0xf1,1,&buf[1]);
	R[9] = ((uint8_t) buf[0] << 8) | buf[1];
	
	//Use R0~R9 calculate C0~C12 of FB-02
	FB.C0 = R[0] >> 4;
	FB.C1 = ((R[1] & 0xFF00) >> 5) | (R[2] & 7);
	FB.C2 = ((R[1] & 0xFF) << 1) | (R[4] & 1);
	FB.C3 = R[2] >> 3;
	FB.C4 = ((uint32_t)R[3] << 2) | (R[0] & 3);
	FB.C5 = R[4] >> 1;
	FB.C6 = R[5] >> 3;
	FB.C7 = ((uint32_t)R[6] << 3) | (R[5] & 7);
	FB.C8 = R[7] >> 3;
	FB.C9 = R[8] >> 2;
	FB.C10 = ((R[9] & 0xFF00) >> 6) | (R[8] & 3);
	FB.C11 = R[9] & 0xFF;
	FB.C12 = ((R[0] & 0x0C) << 1) | (R[7] & 7);
}

//计算实际压强以及温度
void calculate_real_pressure(int32_t UP, int32_t UT)										
{
	int32_t DT, DT2, X01, X02, X03, X11, X12, X13, X21, X22, X23, X24, X25, X26, X31, X32, CF, PP1, PP2, PP3, PP4;
	
	DT	=	((UT - 8388608) >> 4) + (FB.C0 << 4);
	X01	=	(FB.C1 + 4459) * DT >> 1;
	X02	=	((((FB.C2 - 256) * DT) >> 14) * DT) >> 4;
	X03	=	(((((FB.C3 * DT) >> 18) * DT) >> 18) * DT);
	FB.RT =	((2500 << 15) - X01 - X02 - X03) >> 15;
				
	DT2	=	(X01 + X02 + X03) >> 12;	
	X11	=	((FB.C5 - 4443) * DT2);
	X12	=	(((FB.C6 * DT2) >> 16) * DT2) >> 2;
	X13	=	((X11 + X12) >> 10) + ((FB.C4 + 120586) << 4);
				
	X21	=	((FB.C8 + 7180) * DT2) >> 10;
	X22	=	(((FB.C9 * DT2) >> 17) * DT2) >> 12;
	X23 =	(X22 >= X21) ? (X22 - X21) : (X21 - X22);

	X24	=	(X23 >> 11) * (FB.C7 + 166426);
	X25	=	((X23 & 0x7FF) * (FB.C7 + 166426)) >> 11;
	X26 =	(X21 >= X22) ? (((0 - X24 - X25) >> 11) + FB.C7 + 166426) : (((X24 + X25) >> 11) + FB.C7 + 166426);

	PP1	=	((UP - 8388608) - X13) >> 3;
	PP2	=	(X26 >> 11) * PP1;
	PP3	=	((X26 & 0x7FF) * PP1) >> 11;
	PP4	=	(PP2 + PP3) >> 10;
				
	CF	=	(2097152 + FB.C12 * DT2) >> 3;
	X31	=	(((CF * FB.C10) >> 17) * PP4) >> 2;
	X32	=	(((((CF * FB.C11) >> 15) * PP4) >> 18) * PP4);
	FB.RP =	((X31 + X32) >> 15) + PP4 + 99880;
}

//计算相对高度 	单位:m
float Rel_Altitude(int32_t Press, int32_t Ref_P)										
{
	return 44330 * (1 - pow(((float)Press / (float)Ref_P), (1/5.255)));
}

//计算绝对高度   单位:mm
int32_t Abs_Altitude(int32_t Press)																	
{
	int8_t P0;			
	int16_t hs1, dP0;			
	int32_t h0, hs0, HP1, HP2;			
	if(Press >= 103000)
	{	
		P0	=	103;
		h0	=	-138507;
		hs0	=	-21007;
		hs1	=	311;
	}	
	else if(Press >= 98000)
	{	
		P0	=	98;
		h0	=	280531;
		hs0	=	-21869;
		hs1	=	338;
	}	
	else if(Press >= 93000)
	{	
		P0	=	93;
		h0	=	717253;
		hs0	=	-22813;
		hs1	=	370;
	}			
	else if(Press >= 88000)
	{	
		P0	=	88;
		h0	=	1173421;
		hs0	=	-23854;
		hs1	=	407;
	}	
	else if(Press >= 83000)
	{	
		P0	=	83;
		h0	=	1651084;
		hs0	=	-25007;
		hs1	=	450;
	}	
	else if(Press >= 78000)
	{	
		P0	=	78;
		h0	=	2152645;
		hs0	=	-26292;
		hs1	=	501;
	}	
	else if(Press >= 73000)
	{	
		P0	=	73;
		h0	=	2680954;
		hs0	=	-27735;
		hs1	=	560;
	}	
	else if(Press >= 68000)
	{	
		P0	=	68;
		h0	=	3239426;
		hs0	=	-29366;
		hs1	=	632;
	}	
	else if(Press >= 63000)
	{	
		P0	=	63;
		h0	=	3832204;
		hs0	=	-31229;
		hs1	=	719;
	}	
	else if(Press >= 58000)
	{	
		P0	=	58;
		h0	=	4464387;
		hs0	=	-33377;
		hs1	=	826;
	}	
	else if(Press >= 53000)
	{	
		P0	=	53;
		h0	=	5142359;
		hs0	=	-35885;
		hs1	=	960;
	}		
	else if(Press >= 48000)
	{	
		P0	=	48;
		h0	=	5874268;
		hs0	=	-38855;
		hs1	=	1131;
	}	
	else if(Press >= 43000)
	{	
		P0	=	43;
		h0	=	6670762;
		hs0	=	-42434;
		hs1	=	1354;
	}	
	else if(Press >= 38000)
	{	
		P0	=	38;
		h0	=	7546157;
		hs0	=	-46841;
		hs1	=	1654;
	}	
	else if(Press >= 33000)
	{	
		P0	=	33;
		h0	=	8520395;
		hs0	=	-52412;
		hs1	=	2072;
	}	
	else
	{	
		P0	=	28;
		h0	=	9622536;
		hs0	=	-59704;
		hs1	=	2682;
	}
	dP0	=	Press - P0 * 1000;
	HP1	=	(hs0 * dP0) >> 2;
	HP2	=	(((hs1 * dP0) >> 10)* dP0) >> 4;		
	return	((h0 << 6) + HP1 + HP2) >> 6;
}


