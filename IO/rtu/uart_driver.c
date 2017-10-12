/*
 * uart_driver.c
 *
 *  Created on: 2016年8月25日
 *      Author: HE
 */

/*
#include "global_define.h"
*/

#include "uart_driver.h"
#include "../Modbus/uart_oper.h"
#include "../Modbus/gpio_oper.h"
#include <termios.h>
#include <unistd.h>

#define GPIO_RS485_RW			47
#define RS485_RD				0
#define RS485_WR				1


int UART_write (UART_Handle handle, const void * buffer, int size)
{
	int len;
	int sleep_cnt;

	if (size > 0) {
		//if (ndp->rs485_rw_pin >= 0 && ndp->mode == MODBUS_MODE_485) {
		//目前为485模式，不用判断
		gpio_set(GPIO_RS485_RW, RS485_WR);	/* set wr */
		len = uart_send(handle, buffer, size);
		sleep_cnt = (size * 9000 / 9600 + 2) * 1000;
		usleep(sleep_cnt);	/* sleep for a while, waiting for send completed */
		gpio_set(GPIO_RS485_RW, RS485_RD);	/* set rd */
#if 0
		if (ndp->rs485_rw_pin >= 0 && ndp->mode == MODBUS_MODE_485) {
			sleep_cnt = (ndp->send_len * 9000 / ndp->baundrate + 2) * 1000;
			usleep(sleep_cnt);	/* sleep for a while, waiting for send completed */
			gpio_set(ndp->rs485_rw_pin, RS485_RD);	/* set rd */
		}
#endif
		if (len == size) {
			show_msg("sent msg", (char *)buffer, size);
			return 0;
		}
		else {
			return -1;
		}
}

	return 0;
}
#if 0
int UART_read (UART_Handle handle, void * buffer, int size)
{
	return 0;
}
#endif
