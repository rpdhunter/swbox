/*
 * uart_driver.h
 *
 *  Created on: 2016年8月25日
 *      Author: HE
 */

#ifndef INC_UART_DRIVER_H_
#define INC_UART_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
#include "global_define.h"
*/
#include "../rdb/data_types.h"

typedef int UART_Handle;

#define RS485_0_R()
#define RS485_0_W()
#define RS485_1_R()
#define RS485_1_W()
#define RS485_2_R()
#define RS485_2_W()
#define UART_PORT				"/dev/ttyPS0"

//int uart_write (UART_Handle handle, const void * buffer, int size, int mode, int channel);
//int uart_read (UART_Handle handle, void * buffer, int size, int mode, int channel);
int UART_write (UART_Handle handle, const void * buffer, int size);
//int UART_read (UART_Handle handle, void * buffer, int size);

#ifdef __cplusplus
}
#endif

#endif /* INC_UART_DRIVER_H_ */
