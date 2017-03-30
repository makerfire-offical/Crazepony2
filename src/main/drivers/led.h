#ifndef _LED_H_
#define _LED_H_


#define LED_NUM 4
#define LA	    0x01
#define LB      0x02
#define LC      0x04
#define LD      0x08

#define READY 	0
#define CALI	1
#define BATL	2
#define LOST	3
#define LAND	4

#define LedA_on    GPIO_SetBits(GPIOB, GPIO_Pin_2)
#define LedA_off   GPIO_ResetBits(GPIOB, GPIO_Pin_2)

#define LedB_on    GPIO_SetBits(GPIOB, GPIO_Pin_3)
#define LedB_off   GPIO_ResetBits(GPIOB, GPIO_Pin_3)

#define LedC_on    GPIO_SetBits(GPIOB, GPIO_Pin_4)
#define LedC_off   GPIO_ResetBits(GPIOB, GPIO_Pin_4)

#define LedD_on    GPIO_SetBits(GPIOB, GPIO_Pin_5)
#define LedD_off   GPIO_ResetBits(GPIOB, GPIO_Pin_5)


typedef union {
	uint8_t byte;
	struct {
		uint8_t A:1;
		uint8_t B:1;
		uint8_t C:1;
		uint8_t D:1;
		uint8_t reserved:4;
	}bits;
}LEDBuf_t;

typedef struct Led_tt
{
uint8_t event;
uint8_t state;
uint16_t cnt;
}LED_t;

extern LED_t LEDCtrl;


void LED_init(void);
void LED_loop(void);

#endif


