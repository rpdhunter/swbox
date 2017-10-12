/********************************************************************
	created:	2013/12/27
	created:	27:12:2013   10:55
	filename: 	Z:\prog\nr-switch-prog\src\comm_lib\src\thead.c
	file path:	Z:\prog\nr-switch-prog\src\comm_lib\src
	file base:	thead
	file ext:	c
	author:		zhouxf
	purpose:	thread related operations
*********************************************************************/

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sched.h>

#include "thread.h"

#define PTHREAD_STACK_MIN 8192

typedef struct thread_creat_info_s {
	void		(* f)(void *);
	void		*arg;
	sem_t		sem;
} thread_creat_info_t;

static void *
thread_boot (void * ti_void)
{
	thread_creat_info_t	* ti = ti_void;
	void	(* f)(void *);
	void	* arg;

	/*block_signal_for_proc ();*/

    /* Ensure that we give up all resources upon exit */
    pthread_detach (pthread_self());

    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);

    f = ti->f;
    arg = ti->arg;

    /* Notify parent to continue */
    sem_post (&(ti->sem));

    /* Call thread function */
    (*f)(arg);

    /* Thread function did not call sal_thread_exit() */
    thread_exit (0);

    /* Will never get here */
    return NULL;
}

/*
 * Support schedule policy and priority setting
 * prio 0 : SCHED_OTHER
 *      1~49  : SCHED_RR
 *      50~99 : SCHED_FIFO
 */
pthread_t thread_create (char * name, int ss, int prio, void (f)(void *), void *arg)
{
	pthread_attr_t attribs;
	pthread_t	   id = -1;
	thread_creat_info_t * tinfo;
	struct sched_param param;
	int 		   ret, sched_policy;

	if (pthread_attr_init (&attribs)) {
		return -1;
	}
	/*
	函数传入值：attr:线程属性。
	函数返回值：成功： 0
	失败： -1
	成功时返回0，if为假，不做return
	*/
	ss += PTHREAD_STACK_MIN;
	if (ret=pthread_attr_setstacksize(&attribs, ss)) {
		printf("pthread_attr_setstacksize failed %d\n", ret);
		printf("stacksize %d kB\n", ss/1024);
		return -1;
	}

	/* set schedule policy first */
	if (prio == 0) {
		sched_policy = SCHED_OTHER;
	}
	else if (prio >= 1 && prio <= 49) {
		sched_policy = SCHED_RR;
	}
	else if (prio >= 50 && prio <= 99) {
		sched_policy = SCHED_FIFO;
	}
	else {
		/* set to default priority and other schedule */
		prio = 0;
		sched_policy = SCHED_OTHER;
	}
	if ((ret = pthread_attr_setschedpolicy (&attribs, sched_policy)) != 0) {
		printf ("pthread_attr_setschedpolicy failed %d\n", ret);
		return -1;
	}

	/* set priority */
	pthread_attr_getschedparam (&attribs, &param);
	param.sched_priority = prio;
	if ((ret = pthread_attr_setschedparam (&attribs, &param)) != 0) {
		printf ("pthread_attr_setschedparam failed %d\n", ret);
		return -1;
	}

	tinfo = (thread_creat_info_t *)malloc (sizeof (thread_creat_info_t));
	if (tinfo == NULL) {
		return -1;
	}

	if (sem_init (&(tinfo->sem), 0, 0) < 0) {
		free (tinfo);
		return -1;
	}

	tinfo->f = f;
	tinfo->arg = arg;

    if (pthread_create (&id, &attribs, thread_boot, (void *)tinfo) != 0) {
		goto create_done;
    }

    sem_wait (&(tinfo->sem));

create_done:
    sem_destroy (&(tinfo->sem));
	free (tinfo);

    return id;
}

int thread_destroy (pthread_t tid)
{
    if (pthread_cancel (tid)) {
		return -1;
	}

    return 0;
}

void thread_exit (int rc)
{
    pthread_exit ((void *)(rc));
}

void printids (char * proc_thread_name)
{
	pid_t pid;
	pthread_t tid;

	pid = getpid ();
	tid = pthread_self ();
	printf ("Process(thread) %s pid %u tid %u\n", proc_thread_name, (unsigned int)pid, (unsigned int)tid);
}
