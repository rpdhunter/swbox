/********************************************************************
	created:	2017/03/18
	created:	18:3:2017   14:58
	filename: 	Z:\pd\pd_modbus\src_bin\gpio_oper.c
	file path:	Z:\pd\pd_modbus\src_bin
	file base:	gpio_oper
	file ext:	c
	author:		
	purpose:	gpio operation
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

#include "gpio_oper.h"

int gpio_open (unsigned int gpio_pin /* 0 - 53, mio, 54 - 117, emio */, char * gpio_dir /* in, out, inout */)
{
	int fd, ret;
	char value [10], dir_file [100];

	if (gpio_pin >= GPIO_IO_NUM) {
		return -1;
	}

	/* convert gpio pin number */
	gpio_pin += GPIO_IO_BASE;

	/* export gpio */
	fd = open (GPIO_EXPORT, O_WRONLY);
	if (fd < 0) {
		return -1;
	}
	sprintf (value, "%d", gpio_pin);
	ret = write (fd, value, strlen (value));
	close (fd);
	if (ret < 0) {
		return -1;
	}

	/* set direction */
	sprintf (dir_file, GPIO_DIR_FMT, gpio_pin);
	fd = open (dir_file, O_WRONLY);
	if (fd < 0) {
		return -1;
	}
	ret = write (fd, gpio_dir, strlen (gpio_dir));
	close (fd);
	if (ret < 0) {
		return -1;
	}

	return 0;
}

int gpio_close (unsigned int gpio_pin /* 0 - 53, mio, 54 - 117, emio */)
{
	int fd, ret;
	char value [10];

	if (gpio_pin >= GPIO_IO_NUM) {
		return -1;
	}

	/* convert gpio pin number */
	gpio_pin += GPIO_IO_BASE;

	/* unexport gpio */
	fd = open (GPIO_UNEXPORT, O_WRONLY);
	if (fd < 0) {
		return -1;
	}
	sprintf (value, "%d", gpio_pin);
	ret = write (fd, value, strlen (value));
	close (fd);
	if (ret < 0) {
		return -1;
	}

	return 0;
}

int gpio_set (unsigned int gpio_pin, unsigned int val)
{
	int fd, ret;
	char val_file [100];

	if ((gpio_pin >= GPIO_IO_NUM) ||
		(val > 1)) {
		return -1;
	}

	/* convert gpio pin number */
	gpio_pin += GPIO_IO_BASE;

	sprintf (val_file, GPIO_VAL_FMT, gpio_pin);

	fd = open (val_file, O_WRONLY);
	if (fd < 0) {
		return -1;
	}
	if (val) {
		ret = write (fd, "1", 1);
	}
	else {
		ret = write (fd, "0", 1);
	}
	close (fd);
	if (ret < 0) {
		return -1;
	}

	return 0;
}

int gpio_get (unsigned int gpio_pin, unsigned int * val)
{
	int fd, ret;
	char val_file [100];
	char value [10];

	if ((gpio_pin >= GPIO_IO_NUM) ||
		(val == NULL)) {
		return -1;
	}

	/* convert gpio pin number */
	gpio_pin += GPIO_IO_BASE;

	sprintf (val_file, GPIO_VAL_FMT, gpio_pin);

	fd = open (val_file, O_RDONLY);
	if (fd < 0) {
		return -1;
	}
	ret = read (fd, value, sizeof (value));
	if (ret >= 0) {
		* val = atoi (value);
	}
	close (fd);
	if (ret < 0) {
		return -1;
	}

	return 0;
}

int gpio_edge (unsigned int gpio_pin, char * edge /* none, rising, falling, both */)
{
	int fd, ret;
	char edge_file [100];

	if (gpio_pin >= GPIO_IO_NUM) {
		return -1;
	}

	if ((strcmp (edge, "none") != 0) &&
		(strcmp (edge, "rising") != 0) &&
		(strcmp (edge, "falling") != 0) &&
		(strcmp (edge, "both") != 0)) {
		return -1;
	}

	/* convert gpio pin number */
	gpio_pin += GPIO_IO_BASE;

	sprintf (edge_file, GPIO_EDGE_FMT, gpio_pin);  

	fd = open (edge_file, O_WRONLY);
	if (fd < 0) {
		return -1;
	}
	ret = write (fd, edge, strlen (edge));
	close (fd);
	if (ret < 0) {
		return -1;
	}

	return 0;
}

