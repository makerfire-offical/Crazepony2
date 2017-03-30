#ifndef _NRF2401_H_
#define _NRF2401_H_


//*********************************************NRF24L01*******************
#define TX_ADR_WIDTH    5   	// 5 uints TX address width
#define RX_ADR_WIDTH    5   	// 5 uints RX address width

#define RX_PLOAD_WIDTH  32  	// 32 uints TX payload
#define TX_PLOAD_WIDTH  32  	// 32 uints TX payload
//***************************************NRF24L01寄存器指令****************
#define NRF_READ_REG    0x00  	// 读寄存器指令
#define NRF_WRITE_REG   0x20 	// 写寄存器指令
#define RD_RX_PLOAD     0x61  	// 读取接收数据指令
#define WR_TX_PLOAD     0xA0  	// 写待发数据指令
#define FLUSH_TX        0xE1 	// 冲洗发送 FIFO指令
#define FLUSH_RX        0xE2  	// 冲洗接收 FIFO指令
#define REUSE_TX_PL     0xE3  	// 定义重复装载数据指令
#define NOP             0xFF  	// 保留
//*************************************SPI(nRF24L01)寄存器地址**************
#define CONFIG          0x00  // 配置收发状态，CRC校验模式以及收发状态响应方式
#define EN_AA           0x01  // 自动应答功能设置
#define EN_RXADDR       0x02  // 可用信道设置
#define SETUP_AW        0x03  // 收发地址宽度设置
#define SETUP_RETR      0x04  // 自动重发功能设置
#define RF_CH           0x05  // 工作频率设置
#define RF_SETUP        0x06  // 发射速率、功耗功能设置
#define NRFRegSTATUS    0x07  // 状态寄存器
#define OBSERVE_TX      0x08  // 发送监测功能
#define CD              0x09  // 地址检测           
#define RX_ADDR_P0      0x0A  // 频道0接收数据地址
#define RX_ADDR_P1      0x0B  // 频道1接收数据地址
#define RX_ADDR_P2      0x0C  // 频道2接收数据地址
#define RX_ADDR_P3      0x0D  // 频道3接收数据地址
#define RX_ADDR_P4      0x0E  // 频道4接收数据地址
#define RX_ADDR_P5      0x0F  // 频道5接收数据地址
#define TX_ADDR         0x10  // 发送地址寄存器
#define RX_PW_P0        0x11  // 接收频道0接收数据长度
#define RX_PW_P1        0x12  // 接收频道1接收数据长度
#define RX_PW_P2        0x13  // 接收频道2接收数据长度
#define RX_PW_P3        0x14  // 接收频道3接收数据长度
#define RX_PW_P4        0x15  // 接收频道4接收数据长度
#define RX_PW_P5        0x16  // 接收频道5接收数据长度
#define FIFO_STATUS     0x17  // FIFO栈入栈出状态寄存器设置

#define RX_DR			6	  //中断标志
#define TX_DS			5
#define MAX_RT	  		4
#define MAX_TX  		0x10  //达到最大发送次数中断
#define TX_OK   		0x20  //TX发送完成中断
#define RX_OK   		0x40  //接收到数据中断
//************************************************************************

#define ROL	0
#define PIT	1
#define YA	2
#define THR	3


//msp cmd
enum MSP_CMD{
	ARM	= 1<<0,
	FREEHEAD = 1<<1,
	ALTHOLD = 1<<2,
	CALIBRATION = 1<<3,
	NEWADDRESS = 1<<4,
};

//nrf2401 data
typedef struct 
{
	uint8_t checkCode[4];
	uint8_t length;
	uint16_t mspCmd;
	uint16_t motor[4];
}dataPackage;
extern dataPackage mspData;

typedef struct 
{
	uint16_t batt;
	int16_t roll;
	int16_t pitch;
	int16_t yaw;
	float height;
	uint8_t batt_low;
	bool alt;
	bool single_loss;
	bool take_off;
	bool land;
	bool calibration;
}golbal_flag;
extern golbal_flag flag;


#define SPI_CE_H()   GPIO_SetBits(GPIOB, GPIO_Pin_1)//CE
#define SPI_CE_L()   GPIO_ResetBits(GPIOB, GPIO_Pin_1)

#define SPI_CSN_H()  GPIO_SetBits(GPIOB, GPIO_Pin_12)//NSS/CSN
#define SPI_CSN_L()  GPIO_ResetBits(GPIOB, GPIO_Pin_12)

#define bound(val,max,min) ((val) > (max)? (max) : (val) < (min)? (min) : (val))

bool NRF24L01_init(void);
bool NRF24L01_Check(void); 
void nrf24l01HardwareInit(void);

void rx_data_process(int16_t *buf);
bool nrf_rx(void);
void SetRX_Mode(void);

#endif

