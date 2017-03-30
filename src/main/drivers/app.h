#ifndef _APP_H_
#define _APP_H_

#define APP_DATA_LENGTH 	5

enum APP_CMD{
	APP_ARM = 1<<0,
	APP_DIS = 1<<1,
	APP_CAL = 1<<7,
	APP_ALT = 1<<4,
};
extern uint8_t App_data[APP_DATA_LENGTH];
extern bool APP_DATA_FLAG,WIFI_DATA_OK;

extern void wifi_uart_init(void);
#endif
