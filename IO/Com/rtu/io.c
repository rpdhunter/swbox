/********************************************************************
	created:	2015/11/09
	created:	9:11:2015   13:15
	filename: 	D:\cygwin64\home\ibm\prog\rtu\src_bin\io.c
	file path:	D:\cygwin64\home\ibm\prog\rtu\src_bin
	file base:	io
	file ext:	c
	author:		zhouxf
	purpose:	io operation
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "../rdb/rdb.h"

int yx_read (
	unsigned int begin_no,		/* 从0到YX_NUMBER-1 */
	unsigned int num,			/* 从1到YX_NUMBER */
	unsigned int yx_val []		/* 数据缓存数组 */
	)
{
	printf ("read yx begin_no %d, num %d\n", begin_no, num);
	return 0;
}

int yk_oper (
	unsigned int yk_no,			/* 从0到YK_NUMBER-1 */
	unsigned int yk_val			/* 单点值，SP_OPEN or SP_CLOSE */
	)
{
	printf ("oper yk yk_no %d, yk_val %d\n", yk_no, yk_val);
	yk_done (yk_no, 1);

	return 0;
}

