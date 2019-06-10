/********************************************************************
	created:	2017/03/18
	created:	18:3:2017   15:02
	filename: 	Z:\pd\pd_modbus\src_bin\uart_oper.c
	file path:	Z:\pd\pd_modbus\src_bin
	file base:	uart_oper
	file ext:	c
	author:		
	purpose:	uart operation
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

#include "IO/Com/modbus/uart_oper.h"

int uart_set (int fd, int speed, int flow_ctrl, int databits, int stopbits, char parity)
{
	int i, speed_num;
	int speed_arr [] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300};
	int name_arr [] = {115200, 38400, 19200, 9600, 4800, 2400, 1200, 300};
    struct termios options;

	if (tcgetattr (fd, &options) != 0) {
		printf ("get serial port attributes error\n");
		return -1;
	}

	speed_num = sizeof (speed_arr) / sizeof (int);
	for (i = 0; i < speed_num; i++) {
		if (speed == name_arr [i]) {
			cfsetispeed (&options, speed_arr [i]);
			cfsetospeed (&options, speed_arr [i]);
			break;
		}
	}
	if (i >= speed_num) {
		cfsetispeed (&options, B9600);
		cfsetospeed (&options, B9600);
		printf ("Unsupport speed, using default speed\n");
	}

	//修改控制模式，保证程序不会占用串口
	options.c_cflag |= CLOCAL;
	//修改控制模式，使得能够从串口中读取输入数据
	options.c_cflag |= CREAD;

	//设置数据流控制
	switch (flow_ctrl) {	
	case 0://不使用流控制
		options.c_cflag &= ~CRTSCTS;
		break;
	case 1://使用硬件流控制
		options.c_cflag |= CRTSCTS;
		break;
	case 2://使用软件流控制
		options.c_cflag |= IXON | IXOFF | IXANY;
		break;
	default:
		options.c_cflag &= ~CRTSCTS;
		printf ("Unsupported data flow control mode, using default mode\n");
	}
	
	//设置数据位
	//屏蔽其他标志位
	options.c_cflag &= ~CSIZE;
	switch (databits) {
	case 5:
		options.c_cflag |= CS5;
		break;
	case 6:
		options.c_cflag |= CS6;
		break;
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		options.c_cflag |= CS8;
		printf ("Unsupported data size, using default value\n");
    }
	
	//设置校验位
	switch (parity) {
	case 'n':
	case 'N': //无奇偶校验位。
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		break;
	case 'o':
	case 'O'://设置为奇校验
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;
		break;
	case 'e':
	case 'E'://设置为偶校验
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;
		break;
	case 's':
	case 'S': //设置为空格
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		printf ("Unsupported parity, using default value\n");
    }
	
    // 设置停止位
	switch (stopbits) {
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		options.c_cflag &= ~CSTOPB;
		printf ("Unsupported stop bits, using default value\n");
	}

	//修改输出模式，原始数据输出
	options.c_oflag &= ~OPOST;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	//options.c_lflag &= ~(ISIG | ICANON);

	//防止串口把回车和换行当成同一个字符
//	options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    options.c_iflag &= ~(INLCR | ICRNL | IGNCR | BRKINT | INPCK | ISTRIP | IXON);
	options.c_oflag &= ~(ONLCR | OCRNL);
	
	//设置等待时间和最小接收字符
	options.c_cc [VTIME] = 1; /* 读取一个字符等待1*(1/10)s */
    options.c_cc [VMIN] = 1; /* 读取字符的最少个数为1 */

	//如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读
	tcflush (fd, TCIFLUSH);

	//激活配置 (将修改后的termios数据设置到串口中）
	if (tcsetattr (fd, TCSANOW, &options) != 0) {
		printf ("com set error!\n");
		return -1;
	}

	return 0;
}

int uart_open (char * port, int speed, int flow_ctrl, int databits, int stopbits, char parity)
{
	int fd, flags;

	fd = open (port, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0) {
		printf ("Can't open serial port %s\n", port);
		return -1;
	}

	flags = fcntl (fd, F_GETFL, 0);
#if 0
	//设置串口为非阻塞状态
	flags |= O_NONBLOCK;
#else
	//设置串口为阻塞状态
	flags &= ~O_NONBLOCK;
#endif
	if (fcntl (fd, F_SETFL, flags) == 0) {
		if (uart_set (fd, speed, flow_ctrl, databits, stopbits, parity) == 0) {

			return fd;
		}
	}

	uart_close (fd);

	return -1;
}

void uart_close (int fd)
{
	close (fd);
}

int uart_recv (int fd, unsigned char * recv_buf, int data_len)
{
#if 0
	int len, fs_sel;
	fd_set fs_read;
	struct timeval time;

	if (recv_buf == NULL) {
		return -1;
	}

	FD_ZERO (&fs_read);
	FD_SET (fd, &fs_read);

	time.tv_sec = 10;
	time.tv_usec = 0;

	fs_sel = select (fd + 1, &fs_read, NULL, NULL, &time);
	if (fs_sel > 0) {
		if (FD_ISSET (fd, &fs_read)) {
			len = read (fd, recv_buf, data_len);
			return len;
		}
	}

	return -1;
#else
	if (recv_buf == NULL) {
		return -1;
	}
	
	// 简单点，由VTIME和VMIN确定等待时间
	return read (fd, recv_buf, data_len);
#endif
}

int uart_send (int fd, unsigned char * send_buf, int data_len)
{
	int len = 0;

	len = write (fd, send_buf, data_len);
	if (len == data_len) {
		return len;
	}
	else {
		tcflush (fd, TCOFLUSH);
		return -1;
	}
}
