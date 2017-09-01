/********************************************************************
	created:	2013/12/27
	created:	27:12:2013   10:55
	filename: 	Z:\prog\nr-switch-prog\inc\thread.h
	file path:	Z:\prog\nr-switch-prog\inc
	file base:	thread
	file ext:	h
	author:		zhouxf
	purpose:	thread related operations
*********************************************************************/

#ifndef _NR_THREAD_H_
#define _NR_THREAD_H_

#include <pthread.h>

pthread_t thread_create  (char * name, int ss, int prio, void (f)(void *), void *arg);
int       thread_destroy (pthread_t tid);
void      thread_exit    (int rc);
void	  printids       (char * proc_thread_name);

#endif /* _NR_THREAD_H_ */
