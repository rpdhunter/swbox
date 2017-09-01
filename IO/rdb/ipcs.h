/********************************************************************
	created:	2014/02/26
	created:	26:2:2014   11:07
	filename: 	Z:\prog\nr-switch-prog\inc\sync.h
	file path:	Z:\prog\nr-switch-prog\inc
	file base:	sync
	file ext:	h
	author:		zhouxf
	purpose:	Inter-Process Communication operations
*********************************************************************/
#ifndef _IPCS_H_
#define _IPCS_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <time.h>

/********************************************************************
	purpose:	define semophore operations
*********************************************************************/

#define CREAT_KEY(A,B,C,D)	(((A) << 24) | ((B) << 16) | ((C) << 8) | (D))

/*进程互斥信号量*/
#define PROCESS_WAIT_FOREVER -1
#define PROCESS_SEM_TIMEOUT  10  /*秒*/
#define SYNC_CALL_TIMEOUT_MS 10000 /*ms*/

/*线程互斥信号量*/
#define PTHREAD_WAIT_FOREVER -1
#define PTHREAD_MUTEX_TIMEOUT  10  /*秒*/

/*
 * pthread
 */
int sem_timewait (sem_t * sem, const struct timespec * abs_timeout);

/********************************************************************
*功能:创建进程互斥的信号量。原来不存在，则创建，赋值；存在则直接使用，
      不重新赋值
*参数:key,信号量标识
*参数:value,信号量值.
*返回值：成功返回创建的semid值，出错返回-1.
********************************************************************/
extern int create_sem(key_t key, int value);

/********************************************************************
*功能:创建进程互斥的信号量，并指定semflg值。如果创建成功，用value值赋值
*参数:key,信号量标识
*参数:value,信号量值
*参数:semflg,信号量flag，如0666|IPC_CREAT|IPC_EXCL等
*返回值：成功返回创建的semid值，出错返回-1.
********************************************************************/
extern int create_sem_excl(key_t key, int value, int semflg);

/********************************************************************
*功能:设置进程互斥的信号量值
*参数:semid,信号量
*参数:value,信号量值
*返回值：成功返回0，出错返回-1.
********************************************************************/
extern int setvalue_sem(int semid, int value);

/********************************************************************
*功能:读取进程互斥的信号量值
*参数:semid,信号量
*返回值：成功返回读到的值，出错返回-1.
********************************************************************/
extern int getvalue_sem(int semid);

/********************************************************************
*功能:删除进程互斥的信号量
*参数:semid,信号量
*返回值：成功返回0，出错返回-1.
********************************************************************/
extern int destroy_sem(int semid);

/********************************************************************
*功能:获取进程互斥的信号量
*参数:semid,信号量id
*参数:timeout_s,超时时间,单位秒。用PRCESS_WAIT_FOREVER宏表示一直等待.
*返回值：成功返回0，出错返回-1.
********************************************************************/
extern int process_mutex_take(int semid, int timeout_s);

/********************************************************************
*功能:释放进程互斥的信号量
*参数:semid,信号量id
*返回值：成功返回0，出错返回-1.
********************************************************************/
extern int process_mutex_give(int semid);

/********************************************************************
*功能:创建线程互斥的信号量
*参数:mutex,信号量指针
*返回值：成功返回0，出错返回出错码.
********************************************************************/
extern int pthread_mutex_create(pthread_mutex_t *mutex);

/********************************************************************
*功能:删除线程互斥的信号量
*参数:mutex,信号量指针
*返回值：成功返回0，出错返回出错码.
********************************************************************/
extern int pthread_mutex_delete(pthread_mutex_t *mutex);

/********************************************************************
*功能:获取线程互斥的信号量
*参数:mutex,信号量指针
*参数:timeout_s,超时时间,单位秒。用PTHREAD_WAIT_FOREVER宏表示一直等待.
*返回值：成功返回0，出错返回出错码.
********************************************************************/
extern int pthread_mutex_take(pthread_mutex_t *mutex, int timeout_s);

/********************************************************************
*功能:释放线程互斥的信号量
*参数:mutex,信号量指针
*返回值：成功返回0，出错返回出错码.
********************************************************************/
extern int pthread_mutex_give(pthread_mutex_t *mutex);


/********************************************************************
	purpose:	msgq related operations
*********************************************************************/
#define MSGQ_TEST_EXIST		(IPC_EXCL)
#define MSGQ_GET_FLAG		(0666 | IPC_CREAT)
#define MSGQ_DEL_FLAG		(IPC_RMID)
#define MSGQ_SND_FLAG		(IPC_NOWAIT)
#define MSGQ_RCV_FLAG		(0)

#define MSG_CONTEX_SIZE		1024

typedef struct msgbuf_s {
	int  msg_type;
	char msg_contex [MSG_CONTEX_SIZE];
} msgbuf_t;

/*
 * return value:
 * -1: error
 *  0: success
 *  1: created
 */
int msgq_create (int key, int * msgq_id);
int create_msg_q(int *q_id);
int msgq_delete (int msgq_id);
int msgq_send (int msgq_id, char * msg, int len, int msg_type);
int msgq_recv (int msgq_id, char * msg, int len, int msg_type);
int msgrcv_timeout(int msqid, void *msgp, unsigned int msgsz, long msgtyp, int time_ms);


/********************************************************************
	purpose:	shared memory related operations
*********************************************************************/
void * create_shared_memory (key_t key, int size);

/********************************************************************
	purpose:	event mechanism 
*********************************************************************/

typedef struct events_s {
	sem_t			cnt_sem;
	sem_t			mutex_sem;
	unsigned int	event_mask;
} events_t;

int events_create (events_t * ev);
unsigned int events_timewait (events_t * ev, struct timespec * abs_timeout);
int events_post (events_t * ev, unsigned int ev_mask);

#endif /* _IPCS_H_ */

