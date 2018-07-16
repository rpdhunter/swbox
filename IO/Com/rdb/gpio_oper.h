/********************************************************************
	created:	2017/03/18
	created:	18:3:2017   14:59
	filename: 	Z:\pd\pd_modbus\src_bin\gpio_oper.h
	file path:	Z:\pd\pd_modbus\src_bin
	file base:	gpio_oper
	file ext:	h
	author:		
	purpose:	gpio operation
*********************************************************************/
#ifndef _GPIO_OPER_H_
#define _GPIO_OPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_DIR				"/sys/class/gpio"
#define GPIO_EXPORT				GPIO_DIR"/export"
#define GPIO_UNEXPORT			GPIO_DIR"/unexport"
#define GPIO_DIR_FMT			GPIO_DIR"/gpio%d/direction"
#define GPIO_VAL_FMT			GPIO_DIR"/gpio%d/value"
#define GPIO_EDGE_FMT			GPIO_DIR"/gpio%d/edge"

#define GPIO_IO_BASE			906
#define GPIO_IO_NUM				118
#define GPIO_MIO_NUM			54
#define GPIO_EMIO_NUM			64

int gpio_open (unsigned int gpio_pin /* 0 - 53, mio, 54 - 117, emio */, char * gpio_dir /* in, out */);
int gpio_close (unsigned int gpio_pin /* 0 - 53, mio, 54 - 117, emio */);
int gpio_set (unsigned int gpio_pin, unsigned int val);
int gpio_get (unsigned int gpio_pin, unsigned int * val);
int gpio_edge (unsigned int gpio_pin, char * edge /* none, rising, falling, both */);

#ifdef __cplusplus
}
#endif

#endif /* _GPIO_OPER_H_ */
