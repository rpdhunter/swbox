/********************************************************************
	created:	2015/11/09
	created:	9:11:2015   13:16
	filename: 	D:\cygwin64\home\ibm\prog\rtu\inc\io.h
	file path:	D:\cygwin64\home\ibm\prog\rtu\inc
	file base:	io
	file ext:	h
	author:		zhouxf
	purpose:	io operations defines
*********************************************************************/

#ifndef _IO_H_
#define _IO_H_

int yx_read (
	unsigned int begin_no,		/* 从0到YX_NUMBER-1 */
	unsigned int num,			/* 从1到YX_NUMBER */
	unsigned int yx_val []		/* 数据缓存数组 */
	);
int yk_oper (
	unsigned int yk_no,			/* 从0到YK_NUMBER-1 */
	unsigned int yk_val			/* 单点值，SP_OPEN or SP_CLOSE */
	);

#endif /* _IO_H_ */

