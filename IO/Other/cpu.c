/*
 * thread or process set running on special core
 */


#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define __USE_GNU
#include <sched.h>
#include <string.h>

#include "cpu.h"

/********************************************************************
*功能:设置某进程运行的CPU范围
*参数:pid,进程id号，0表示本进程
*参数:cpu_mask,使用的CPU范围，bit0-bit31分别表示cpu0-cpu31,
		采用CPU_0到CPU_9的宏写，可以几个CPU或起来，如CPU_2|CPU_3|CPU_6
*返回值：成功返回0，出错返回-1.
********************************************************************/
extern unsigned int set_process_cpu(unsigned int pid, unsigned int cpu_mask)
{
	cpu_set_t mask;
	int num, i;

	CPU_ZERO(&mask);

	num = sysconf(_SC_NPROCESSORS_CONF);
	for(i = 0; i < num; i++)
	{
		if(cpu_mask & (1 << i))
		{
			CPU_SET(i, &mask);
		}
	}

	if (sched_setaffinity((pid_t)0, sizeof(mask), &mask) != 0)
	{
		printf ("set process CPU failed!\n");
		return -1;
	}

	return 0;
}

/********************************************************************
*功能:设置某线程运行的CPU范围
*参数:tid,线程id号
*参数:cpu_mask,使用的CPU范围，bit0-bit31分别表示cpu0-cpu31,
        采用CPU_0到CPU_9的宏写，可以几个CPU或起来，如CPU_2|CPU_3|CPU_6
*返回值：成功返回0，出错返回-1.
********************************************************************/
extern unsigned int set_thread_cpu(pthread_t tid, unsigned int cpu_mask)
{
    cpu_set_t mask;
    int num, i;

    CPU_ZERO(&mask);

    num = sysconf(_SC_NPROCESSORS_CONF);
    for(i = 0; i < num; i++)
    {
        if(cpu_mask & (1 << i))
        {
            CPU_SET(i, &mask);
        }
    }

    if (pthread_setaffinity_np (tid, sizeof(mask), &mask) != 0)
    {
        printf ("set thread CPU failed!\n");
        return -1;
    }

    return 0;
}

/********************************************************************
*功能:显示当前进程运行的CPU,死循环函数，一直打印，不返回,只能测试用
*参数:task_name_p,进程或者线程名称
*返回值：不返回
********************************************************************/
extern int show_process_cpu(char * task_name_p)
{
	int num, i;
	cpu_set_t get;

	num = sysconf(_SC_NPROCESSORS_CONF);
	printf ("total CPU number =%d\n", num);

	while(1)
	{
		CPU_ZERO(&get);
		if (sched_getaffinity(0, sizeof(get), &get) == -1)
		{
			printf ("get process CPU failed!\n");
			return -1;
		}

		printf ("start print:\n");
		for (i = 0; i < num; i++)
		{
	        if (CPU_ISSET(i, &get))
	        {
				printf ("process %d (%s) running CPU: %d\n",getpid(), task_name_p, i);
	        }
		}
		
		sleep(1);
	}

	return 0;
}

/********************************************************************
*功能:显示当前进程运行的CPU,只打一次
*参数:task_name_p,进程或者线程名称
*返回值：不返回
********************************************************************/
extern int show_process_cpu_one_time(char *task_name_p)
{
	int num, i;
	cpu_set_t get;

	num = sysconf(_SC_NPROCESSORS_CONF);
	printf ("total CPU number =%d\n", num);

	CPU_ZERO(&get);
	if (sched_getaffinity(0, sizeof(get), &get) == -1)
	{
		printf ("get process CPU failed!\n");
		return -1;
	}

	printf ("start print:\n");
	for (i = 0; i < num; i++)
	{
        if (CPU_ISSET(i, &get))
        {
			printf ("process %d (%s) running CPU: %d\n",getpid(), task_name_p, i);
        }
	}

	return 0;
}
